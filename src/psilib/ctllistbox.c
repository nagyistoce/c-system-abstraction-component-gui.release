

#include <stdhdrs.h>
#include <string.h>
#include <sharemem.h>
#include <keybrd.h>

#define LISTBOX_SOURCE


#include "controlstruc.h"
#include <psi.h>

PSI_LISTBOX_NAMESPACE

//typedef struct listitem_tag *PLISTITEM;

struct listitem_tag
{
	TEXTCHAR *text;
	PSI_CONTROL within_list;
	PTRSZVAL data;
	// top == -1 if not show, or not shown yet
	// else top == pixel offset of the top of the item
	S_32 top; // top of the item in the listbox...
	         // makes for quick rendering of custom items
	         // also can use this to push current down
	         // when inserting sorted items...
	_32 height; // height of the line...
	int nLevel; // level of the tree item...
	Image icon;
	struct {
		_32 bSelected :1;
		_32 bFocused  :1;
		_32 bOpen : 1; // if open, show any items after this +1 level...
	} flags;
	PMENU pPopup;
	void (CPROC*MenuProc)(PTRSZVAL,struct listitem_tag*,_32);
	PTRSZVAL psvContextMenu;
	PLISTITEM next, prior;
};
typedef struct listitem_tag LISTITEM;

typedef struct listcolumn_tag LISTCOL, *PLISTCOL;
struct listcolumn_tag
{
	struct {
      // this column label is meant ot be vertical.
		BIT_FIELD bVertical : 1;
	} flags;
	CTEXTSTR header;
	int position; // previously stop (tab stop)

};

typedef struct listbox_tag
{
   //_32 attr;
	Image  ListSurface;
	PLISTITEM items // first item
				, last // last item
				, current // current (focus cursor on this)
				, firstshown // first item shown
				, lastshown  // last item shown
            , mouseon // item the mouse first clicked on (for popup menu support)
            , first_selected; // first item selected for shift-click multi-select
	PLISTITEM _pli; // last state - used for dispatch item (which if deleted, needs to not resend)
	struct {
		_32 bSingle : 1; // alternative multiple selections can be made
		_32 bDestroying : 1;
		_32 bNoUpdate : 1;
		_32 bTree : 1;
		_32 bInitial : 1;
		_32 bSortNormal : 1; // sort least to most (top is least, bottom is most )
		_32 bVertical_Column_abels : 1;
		_32 bSizable_Columns : 1;
		_32 bSortable_Columns : 1;
		_32 bLazyMulti : 1; // just click on item toggles state.
	} flags;
	PSI_CONTROL pcScroll;
	int nLastLevel;
	int TimeLastClick;
	int x, y, b; // old mouse info;

	int nTabstops;
	int nTabstop[30];

	SelectionChanged SelChangeHandler;
	PTRSZVAL psvSelChange;
	DoubleClicker DoubleClickHandler;
	PTRSZVAL psvDoubleClick;
	ListItemOpened ListItemOpenHandler;
	PTRSZVAL psvOpenClose;
} LISTBOX, *PLISTBOX;

//---------------------------------------------------------------------------

void CPROC SetListBoxTabStops( PSI_CONTROL pc, int nStops, int *pStops )
{
	int n;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( nStops > 30 )
	{
      lprintf( WIDE("Only setting first 30 stops") );
		nStops = 30;
	}
	for( n = 0; ( plb->nTabstop[n] = pStops[n]),(n < nStops); n++ );
   plb->nTabstops = nStops;
}

//---------------------------------------------------------------------------
void DeleteListItem( PSI_CONTROL pc, PLISTITEM hli )
{
	PLISTITEM pli = (PLISTITEM)hli;
	PLISTITEM cur;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		cur = plb->items;
		while( cur )
		{
			if( cur == pli )
			break;
			cur = cur->next;
		}
		if( cur )
		{
			PLISTITEM pliNew;
			if( pli->prior )
				pli->prior->next = pli->next;
			else
				plb->items = cur->next;

			if( pli->next )
			{
				pli->next->prior = pli->prior;
				pliNew = pli->next;
			}
			else
			{
				plb->last = pli->prior;
				pliNew = pli->prior;
			}

			if( plb->firstshown == pli )
			{
				if( pli->next )
					plb->firstshown = pli->next;
				else if( pli->prior )
					plb->firstshown = pli->prior;
				else
					plb->firstshown = NULL;
			}
			if( plb->lastshown == pli )
				plb->lastshown = NULL;
			if( plb->current == pli )
			{
			// set current selection
				plb->current = pliNew;
			}
			Release( pli->text );
			Release( pli );
		}
		if( !plb->flags.bDestroying )
			SmudgeCommon(pc);
	}
}

//---------------------------------------------------------------------------

PSI_PROC( void, ResetList )( PSI_CONTROL pc )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
      plb->flags.bDestroying = 1; // fake it...
      while( plb->items )
         DeleteListItem( pc, (PLISTITEM)plb->items );
      plb->flags.bDestroying = 0; // okay we're not really ...
		SmudgeCommon( pc );
   }
}

//---------------------------------------------------------------------------

static void CPROC DestroyListBox( PSI_CONTROL pc )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
		plb->flags.bDestroying = TRUE;
		UnmakeImageFile( plb->ListSurface );
		while( plb->items )
			DeleteListItem( pc, (PLISTITEM)plb->items );
	}
}

//---------------------------------------------------------------------------

void ClearSelectedItems( PSI_CONTROL pc )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	PLISTITEM pli;
	plb->first_selected = NULL; // no selection
	pli = plb->items;
	while( pli )
	{
		pli->flags.bSelected = FALSE;
		pli = pli->next;
	}
}

//---------------------------------------------------------------------------

static void AdjustItemsIntoBox( PSI_CONTROL pc )
{
	// current item must be shown in the list...
	_32 w, h;
	int x, y, maxchars;
	PLISTITEM pli;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	GetStringSize( WIDE(" "), &w, &h );
	maxchars = ( pc->surface_rect.width - 8 ) / w;
	y = 2;
	x = 4;
	// if no current, then the cursor line need not be shown
	// in fact first shown is first item...
	if( plb->current )
	{
		plb->firstshown = plb->current;
		pli = plb->current;
		while( pli )
		{
			y += h;
			pli = pli->next;
		}
		// while there's stuff above current to show, and
		// current will still fit integrally on the listbox
		// back up firstshown....
		while( plb->firstshown->prior && 
		       ( y < (pc->surface_rect.height - (h-1))) )
		{
			y += h;
			plb->firstshown = plb->firstshown->prior;
		}
	}
}

#define BRANCH_WIDTH  20

//---------------------------------------------------------------------------

static int RenderRelationLines( PSI_CONTROL pc, PLISTITEM pli, int drawthis )
{
   //ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	PLISTITEM pliNextUpLevel;
   int x, y, ymin, ymax;
	pliNextUpLevel = pli->next;
	x = pli->nLevel * BRANCH_WIDTH + (BRANCH_WIDTH/2);
	y = pli->top + ( pli->height / 2 );
	ymin = pli->top;
	ymax = pli->top + pli->height - 1;
   if( pli->nLevel )
	{
		PLISTITEM pliParent = pli->prior;
		while( pliParent && pliParent->nLevel >= pli->nLevel )
		{
         pliParent = pliParent->prior;
		}
		if( pliParent && !pliParent->flags.bOpen )
		{
			// parent is not open, therefore do not draw lines...
         // also - x offset is irrelavent return..
			return 0;
		}

	}
	if( drawthis )
	{
		while( pliNextUpLevel && pliNextUpLevel->nLevel > pli->nLevel )
			pliNextUpLevel = pliNextUpLevel->next;
		if( pliNextUpLevel && ( pliNextUpLevel->nLevel == pli->nLevel ) )
		{
			do_hline( pc->Surface, y, x, x + (BRANCH_WIDTH/2)-1, basecolor(pc)[SHADE] );
			do_vline( pc->Surface, x, ymin, ymax, basecolor(pc)[SHADE] );
		}
		else
		{
			do_hline( pc->Surface, y, x, x + (BRANCH_WIDTH/2)-1, basecolor(pc)[SHADE] );
			do_vline( pc->Surface, x, ymin, y, basecolor(pc)[SHADE] );
		}
	}
	pliNextUpLevel = pli->next;
	while( pliNextUpLevel )
	{
		while( pliNextUpLevel && pliNextUpLevel->nLevel >= pli->nLevel )
			pliNextUpLevel = pliNextUpLevel->next;
		if( pliNextUpLevel )
		{
			do_vline( pc->Surface
					  , pliNextUpLevel->nLevel * BRANCH_WIDTH + (BRANCH_WIDTH/2)
					  , ymin, ymax
					  , basecolor(pc)[SHADE] );
		}
      pli = pliNextUpLevel;
	}
   return x + BRANCH_WIDTH/2;
}

//---------------------------------------------------------------------------

static int RenderItemKnob( PSI_CONTROL pc, PLISTITEM pli )
{
	PLISTITEM pliNextUpLevel;
	//ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	int x, y; // x, y of center...
	if( !pli->next || ( pli->next->nLevel <= pli->nLevel ) )
	{
		// this is not an openable item, therefore
      // just draw some lines...
		return RenderRelationLines( pc, pli, TRUE );
	}
	else
	{
      // render lines to the left of here but not including here...
		RenderRelationLines( pc, pli, FALSE );
	}
	pliNextUpLevel = pli->next;
	while( pliNextUpLevel && ( pliNextUpLevel->nLevel > pli->nLevel ) )
		pliNextUpLevel = pliNextUpLevel->next;
	x = pli->nLevel * BRANCH_WIDTH + (BRANCH_WIDTH/2);
	y = pli->top + ( pli->height / 2 );

   // this draws the box with a + in it...
	do_hline( pc->Surface, y - 5, x-5, x+5, basecolor(pc)[SHADE] );
	do_hline( pc->Surface, y + 5, x-5, x+5, basecolor(pc)[SHADE] );
	do_vline( pc->Surface, x - 5, y-5, y+5, basecolor(pc)[SHADE] );
	do_vline( pc->Surface, x + 5, y-5, y+5, basecolor(pc)[SHADE] );
	if( !pli->flags.bOpen )
	{
		do_hline( pc->Surface, y, x-3, x+3, basecolor(pc)[SHADOW] );
		do_vline( pc->Surface, x, y-3, y+3, basecolor(pc)[SHADOW] );
	}
	else
	{
		do_hline( pc->Surface, y, x-3, x+3, basecolor(pc)[SHADOW] );
	}

   // draw line leading in (top) and out (right)
   do_vline( pc->Surface, x, y - 5, pli->top, basecolor(pc)[SHADE] );
	do_hline( pc->Surface, y, x + 5, x + (BRANCH_WIDTH/2)-1, basecolor(pc)[SHADE] );

   // optionally draw line leading down (bottom)
	if( pliNextUpLevel && ( pliNextUpLevel->nLevel == pli->nLevel ) )
	{
      // next item is on this level, extend branch line down.
      do_vline( pc->Surface, y, y + 5, pli->top + pli->height - 1, basecolor(pc)[SHADE] );
	}
   return x + BRANCH_WIDTH/2;
}

//---------------------------------------------------------------------------
static void UpdateScrollForList//Ex
( PSI_CONTROL pc
 //DBG_PASS
)
//#define UpdateScrollForList(pc) UpdateScrollForListEx( pc DBG_SRC )
{
   ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	int current = -1, onview = -1, count = 0;
	PLISTITEM pli = plb->items;
	if( plb->firstshown && plb->lastshown )
	{
		while( pli )
		{
			if( pli == plb->firstshown )
				current = count;
			if( current >= 0 && onview < 0 )
			{
				if( pli == plb->lastshown )
				{
					onview = count - current + 1;
				}
			}
			count++;
			if( !pli->flags.bOpen )
			{
				PLISTITEM skip = pli->next;
				while( skip && ( skip->nLevel > pli->nLevel ) )
					skip = skip->next;
				pli = skip;
			}
			else
				pli = pli->next;
		}
	}
   //_xlprintf(LOG_ALWAYS DBG_RELAY)( WIDE("Set scroll params %d %d %d %d"), 0, current, onview, count );
   SetScrollParams( plb->pcScroll, 0, current, onview, count );
}

//---------------------------------------------------------------------------

static int CPROC RenderListBox( PSI_CONTROL pc )
{
   int bFirstDraw;
	_32 w, h;
	int x, y, maxchars;
	PLISTITEM pli;
   ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   Font font;
	Image pSurface = plb->ListSurface;
	if( plb->flags.bInitial )
	{
      bFirstDraw = TRUE;
		plb->flags.bInitial = FALSE;
      plb->flags.bNoUpdate = FALSE;
	}
	else
		bFirstDraw = FALSE;
   BlatColorAlpha( pc->Surface, 0, 0, pc->surface_rect.width, pc->surface_rect.height, basecolor(pc)[EDIT_BACKGROUND] );
	//ClearImageTo( pSurface, basecolor(pc)[EDIT_BACKGROUND] );
   font = GetFrameFont( pc );
	GetStringSizeFont( WIDE("X"), &w, &h, font );
	maxchars = ( pc->surface_rect.width - 8 ) / w;
	y = 2;
	x = 4;
	w = pc->surface_rect.width;
	pli = plb->items;
	if( !plb->firstshown )
      plb->firstshown = plb->items;
	while( pli != plb->firstshown )
	{
		pli->top = -1;
		pli = pli->next;
	}
	//pli = plb->firstshown;
	while( pli && y < pc->surface_rect.height )
	{
		TEXTCHAR *start = pli->text;
		TEXTCHAR *end;
      int tab = 0;
		pli->top = y;
      pli->height = h;
		if( plb->flags.bTree )
		{
			x = RenderItemKnob( pc, pli );
		}
		if( pli->flags.bSelected )
		{
			BlatColor( pSurface, x-2, y, w-4, h, basecolor(pc)[SELECT_BACK] );
		}
		while( start )
		{
         _32 width = 0;
         int bRight = 0;
			S_32 column = plb->nTabstop[tab];
			if( plb->nTabstop[tab] < 0 )
			{
            bRight = 1;
				column = plb->nTabstop[tab+1];
				if( column < 0 )
               column = -column;
			}
			ScaleCoords( pc, &column, NULL );
			end = strchr( start, '\t' );
			if( !end )
				end = start + strlen( start );

			if( bRight )
			{
				width = GetStringSizeFontEx( start, end-start, NULL, NULL, font );
				column -= width;
			}
			if( pli->flags.bSelected )
			{
				PutStringFontEx( pSurface, x + column, y, basecolor(pc)[SELECT_TEXT], 0, start, end-start, font );
			}
			else
			{
				PutStringFontEx( pSurface, x + column, y, basecolor(pc)[EDIT_TEXT], 0, start, end-start, font );
			}
			tab++;
			if( tab >= plb->nTabstops )
            tab = plb->nTabstops-1;
         if( end[0] )
				start = end+1;
			else
            start = NULL;
		}
		if( pc->flags.bFocused &&
		    plb->current == pli )
			do_line( pSurface, x + 1, y + h-2, w - 6, y + h - 2, basecolor(pc)[SHADE] );
		y += h;
      //xlprintf(LOG_ALWAYS)( "y is %ld and height is %ld", y , pc->surface_rect.height );
		if( y < pc->surface_rect.height ) // probably is only partially shown...
		{
			plb->lastshown = pli;
		}
      if( plb->flags.bTree && pli->flags.bOpen )
			pli = pli->next;
		else
		{
			PLISTITEM next = pli->next;
			while( next && next->nLevel > pli->nLevel )
			{
				next->top = -1;
				next = next->next;
			}
         pli = next;
		}
	}

	while( pli )
	{
		pli->top = -1;
		pli = pli->next;
	}
	if( bFirstDraw )
	{
		UpdateScrollForList( pc );
	}
   return 1;
}

//---------------------------------------------------------------------------

static int GetItemIndex( PLISTBOX plb, PLISTITEM pli )
{
	PLISTITEM cur;
	int cnt = 0;
	cur = plb->items;
	while( cur )
	{
		if( pli == cur )
			break;
		cnt++;
		cur = cur->next;
	}
	if( cur )
		return cnt;
	return -1;
}

//---------------------------------------------------------------------------

void MoveListItemEx( PSI_CONTROL pc, PLISTITEM pli, int level_direction, int direction )
{
	if( pli )
	{
      int grouped_item_count = 0;
      PLISTITEM pliLast;
		ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
		INDEX pliIndex = GetItemIndex( plb, pli );
		{
         // find start and end of group (may be a tree'd control and have levels on items)
			PLISTITEM pliGroupStart, pliGroupEnd;
			if( ( pli == plb->items ) && ( direction < 0 ) )
			{
            // item is already at the top of the list
				return;
			}

			pliGroupStart = pliGroupEnd = pli;

			// keep the prior end, which becomes the last item of the group.
         // (may be same as pli)
			grouped_item_count++;
			pliLast = pliGroupEnd;
			pliGroupEnd = pliGroupEnd->next;
			while( pliGroupEnd && pliGroupEnd->nLevel > pliGroupStart->nLevel )
			{
				grouped_item_count++;
            pliLast = pliGroupEnd;
				pliGroupEnd = pliGroupEnd->next;
			}
			if( !pliGroupEnd && direction > 0 )
			{
            // item is already at the end, cannot move any further down.
				return;
			}
			if( plb->firstshown == pli )
				plb->firstshown = pli->prior;
			if( plb->lastshown == pli )
            plb->lastshown = pliLast->next;
			if( pli->prior )
				pli->prior->next = pliGroupEnd;
			else
            plb->items = pliLast->next;
			if( pliGroupEnd )
				pliGroupEnd->prior = pli->prior;
			if( !pliLast->next ) // nothing left - update end of list.
            plb->last = pli->prior;
			pli->prior = NULL;
         pliLast->next = NULL;
		}
		if( ( direction < 0 ) && (-direction >= pliIndex ) )
		{
			if( plb->items )
				plb->items->prior = pliLast;
			else
				plb->last = pliLast;
			pliLast->next = plb->items;
			if( plb->items == plb->firstshown )
            plb->firstshown = pli;
         plb->items = pli;
		}
      else
		{
			PLISTITEM pliInsertAfter = GetNthItem( pc, pliIndex
															  + direction-1 );
			if( !pliInsertAfter )
			{
				// insert at end... overflow end...
				if( plb->last )
				{
					plb->last->next = pli;
					pli->prior = plb->last;
				}
				else
				{
               // list was empty - put group back in.
					plb->items = pli;
               plb->last = pliLast;
				}
			}
			else
			{
				PLISTITEM pliInsertBefore = pliInsertAfter;
            pliInsertBefore = pliInsertBefore->next;
				while( pliInsertBefore && (pliInsertBefore->nLevel > pliInsertAfter->nLevel) )
				{
               pliInsertBefore = pliInsertBefore->next;
				}
				if( pliInsertBefore )
				{
					pliInsertBefore->prior->next = pli;
               pli->prior = pliInsertBefore->prior;
               pliLast->next = pliInsertBefore;
					pliInsertBefore->prior = pliLast;
				}
				else
				{
               // insert at end of list after group of insertafter.
					if( plb->last )
					{
						pli->prior = plb->last;
                  plb->last->next = pli;
						plb->last = pliLast;
					}
					else
					{
						// okay list became empty again - here we should
						// have already run across conditions which should have
						// handled this.
						plb->last = pliLast;
                  plb->items = pli;
					}
				}
			}
		}
	}
   SmudgeCommon( pc );
}

//---------------------------------------------------------------------------

void MoveListItem( PSI_CONTROL pc, PLISTITEM pli, int direction )
{
   MoveListItemEx( pc, pli, 0, direction );
}

//---------------------------------------------------------------------------

// GetNthTreeItem searches under the item specified
// for the Nth item at level specified... ignoring child levels.
PLISTITEM GetNthTreeItem( PSI_CONTROL pc, PLISTITEM pli, int level, int idx )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	PLISTITEM _pli;
   lprintf( WIDE("This function is unfinished in implementation.") );
	_pli = pli = plb->items;
	while( idx && pli )
	{
		idx--;
		_pli = pli;
		pli = pli->next;
		//if( !_pli->flags.bOpen )
		{
			while( pli && ( pli->nLevel > _pli->nLevel ) )
			{
				pli = pli->next;
			}
		}
	}
	//if( !pli )
	//	return (PLISTITEM)_pli;
	return (PLISTITEM)pli;
}

//---------------------------------------------------------------------------

PSI_PROC( PLISTITEM, GetNthItem )( PSI_CONTROL pc, int idx )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	PLISTITEM pli, _pli;
	_pli = pli = plb->items;
	while( idx && pli )
	{
		idx--;
		_pli = pli;
		pli = pli->next;
		if( !_pli->flags.bOpen )
		{
			while( pli && ( pli->nLevel > _pli->nLevel ) )
			{
				pli = pli->next;
			}
		}
	}
	//if( !pli )
	//	return (PLISTITEM)_pli;
	return (PLISTITEM)pli;
}

//---------------------------------------------------------------------------

int GetItemCount( PLISTBOX plb )
{
	PLISTITEM pli, next;
	int cnt = 0;
	pli = plb->items;
	while( pli )
	{
		cnt++;
		next = pli->next;
		if( !pli->flags.bOpen )
		{
			while( next && next->nLevel > pli->nLevel )
				next = next->next;
		}
		pli = next;
	}
	return cnt;
}

//---------------------------------------------------------------------------

static void CPROC ScrollBarUpdate( PTRSZVAL psvList, int type, int current )
{
   PSI_CONTROL pc = (PSI_CONTROL)psvList;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( pc->nType == LISTBOX_CONTROL )
	{
		plb->firstshown = (PLISTITEM)GetNthItem( pc, current );
		SmudgeCommon( pc );
	}	
}


static void DispatchSelectionChanged( PLISTBOX plb, PSI_CONTROL pc, PLISTITEM pli )
{
	//PLISTITEM _pli = plb->current;
	//do
	{
		if( plb->SelChangeHandler )
			plb->SelChangeHandler( plb->psvSelChange, pc, pli );
	}
	//while( pli && plb->current != _pli );
}

//---------------------------------------------------------------------------

static void SelectRange( PLISTBOX plb, PLISTITEM pli_start, PLISTITEM pli_end )
{
	PLISTITEM pli;
	LOGICAL bMark = FALSE;
	pli = plb->items;
	while( pli )
	{
		if( pli == pli_start || pli == pli_end )
		{
			pli->flags.bSelected = TRUE; // the first, and the last are both selected anyhow..
			bMark = !bMark;
		}
      else
			pli->flags.bSelected = bMark;
		pli = pli->next;
	}
}

//---------------------------------------------------------------------------

static int CPROC MouseListBox( PSI_CONTROL pc, S_32 x, S_32 y, _32 b )
{
  	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		if( b & MK_SCROLL_DOWN )
		{
			MoveScrollBar( plb->pcScroll, UPD_1DOWN );
		}
		if( b & MK_SCROLL_UP )
		{
			MoveScrollBar( plb->pcScroll, UPD_1UP );
		}
 		if( b & MK_LBUTTON )
		{
			if( !(plb->b & MK_LBUTTON ) )
			{
				PLISTITEM pli;
				pli = plb->firstshown;
				while( pli )
				{
					if( pli->top >= 0 &&
						 y >= pli->top &&
						 y < ( pli->top + pli->height ) )
                  break;
					pli = pli->next;
				}
				if( pli )
				{
               LOGICAL bWasSelected = pli->flags.bSelected;
					if( plb->flags.bTree
						&& pli->next
						&& ( pli->next->nLevel > pli->nLevel )
						&& ( x < ( ( pli->nLevel + 1) * BRANCH_WIDTH ) ) )
					{
						pli->flags.bOpen = !pli->flags.bOpen;
						if( plb->ListItemOpenHandler )
						{
                     int bDisable = DisableUpdateListBox( pc, TRUE );
							plb->ListItemOpenHandler( plb->psvOpenClose
															, pc
															, pli
															, pli->flags.bOpen );
                     // restore prior disable state...
                     DisableUpdateListBox( pc, bDisable );
						}
						SmudgeCommon( pc );
						UpdateScrollForList( pc );
						goto record_prior_state;
					}

					if( !plb->flags.bLazyMulti && ( plb->flags.bSingle || !( ( b & (MK_SHIFT|MK_CONTROL) ) ) ) )
					{
						ClearSelectedItems( pc );
					}

					if( !plb->first_selected )
						plb->first_selected = pli;
					plb->current = pli;
					if( ( b & MK_SHIFT ) && plb->first_selected )
					{
						if( pli != plb->first_selected )
						{
							SelectRange( plb, plb->first_selected, pli );
						}
					}
					else // with MK_CONTROL or not, toggle selection.  If not contol, then all itms have been cleared
					{
						pli->flags.bSelected = !pli->flags.bSelected;
						// if item is NOT selected but it was not previously selected..
						//if( pli->flags.bSelected && !bWasSelected )
							DispatchSelectionChanged( plb, pc, pli );
					}

					if( ( GetTickCount() - plb->TimeLastClick ) < 250 )
					{
						if( plb->DoubleClickHandler )
							plb->DoubleClickHandler( plb->psvDoubleClick, pc, (PLISTITEM)plb->current );
						// what if double click handler changes the selection again?
						// then selchangehnadler will get called with both?
					}
					plb->TimeLastClick = GetTickCount();
					SmudgeCommon( pc );
				}
			}
		}
		if( b & MK_RBUTTON )
		{
			if( !(plb->b & MK_RBUTTON ) )
			{
				// first down...
				PLISTITEM pli;
				pli = plb->firstshown;
				while( pli )
				{
					if( pli->top >= 0 &&
						 y >= pli->top &&
						 y < ( pli->top + pli->height ) )
                  break;
					pli = pli->next;
				}
            plb->mouseon = pli;
			}
		}
		else
		{
			if( !(plb->b & MK_RBUTTON ) && plb->mouseon )
			{
				// last down...
				PLISTITEM pli;
				pli = plb->firstshown;
				while( pli )
				{
					if( pli->top >= 0 &&
						 y >= pli->top &&
						 y < ( pli->top + pli->height ) )
                  break;
					pli = pli->next;
				}
				if( pli == plb->mouseon )
				{
					if( pli->MenuProc && pli->pPopup )
					{
						// need to update the state NOW - may come back around
						// and have to deal with this...
                  plb->mouseon = NULL;
						plb->x = x;
						plb->y = y;
						plb->b = b;
						{
							_32 result = TrackPopup( pli->pPopup, GetFrame( plb ) );
							if( result != (_32)-1 )
								pli->MenuProc( pli->psvContextMenu, pli, result );
						}
                  return 1;
					}
				}
			}
		}
	record_prior_state:
		plb->x = x;
		plb->y = y;
		plb->b = b;
	}
   return 1;
}

//---------------------------------------------------------------------------

static int CPROC KeyListControl( PSI_CONTROL pc, _32 key )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
      PLISTITEM pli;
		//printf( WIDE("%08x\n"), key );
		if( key & 0x80000000 )
		{
			switch( key & 0xFF )
			{
			case KEY_UP:
            pli = plb->current;
			   if( GetItemIndex( plb, plb->current ) < GetItemIndex( plb, plb->firstshown ) )
			   	plb->firstshown = plb->current;
			   if( GetItemIndex( plb, plb->current ) > GetItemIndex( plb, plb->lastshown ) )
				   plb->firstshown = plb->current;
			   if( plb->current && plb->current->prior )
			   {
			   	if( plb->current == plb->firstshown )
			   	{
			   		plb->firstshown = plb->firstshown->prior;
					}
			   	plb->current = plb->current->prior;
			   }
				else
					if( !plb->current )
						plb->current = plb->items;

				if( plb->flags.bSingle )
				{
					if( pli )
						pli->flags.bSelected = 0;
					if( plb->current )
						plb->current->flags.bSelected = 1;
					plb->current = pli;
					DispatchSelectionChanged( plb, pc, pli );
				}
				Log( WIDE("Key up causes update...") );
				SmudgeCommon( pc );
				UpdateScrollForList( pc );
				break;
			case KEY_DOWN:
				pli = plb->current;
				if( GetItemIndex( plb, plb->current ) < GetItemIndex( plb, plb->firstshown ) )
					plb->firstshown = plb->current;
				if( GetItemIndex( plb, plb->current ) > GetItemIndex( plb, plb->lastshown ) )
					plb->firstshown = plb->current;
				if( plb->current && plb->current->next )
				{
			   		if( plb->current->next == plb->lastshown )
			   		{
			   			plb->firstshown = plb->firstshown->next;
					}
					plb->current = plb->current->next;
				}
				else
					if( !plb->current )
						plb->current = plb->items;
				if( plb->flags.bSingle )
				{
					if( pli )
						pli->flags.bSelected = 0;
					if( plb->current )
						plb->current->flags.bSelected = 1;
					plb->current = pli;
					DispatchSelectionChanged( plb, pc, pli );
				}
				Log( WIDE("Key down causes update...") );
				SmudgeCommon( pc );
            UpdateScrollForList( pc );
				break;
			case KEY_PGUP:
            MoveScrollBar( plb->pcScroll, UPD_RANGEUP );
				break;
			case KEY_PGDN:
            MoveScrollBar( plb->pcScroll, UPD_RANGEDOWN );
				break;
			case KEY_SPACE:
				if( plb->current )
				{
					if( plb->flags.bSingle )
						ClearSelectedItems( pc );
					plb->current->flags.bSelected = !plb->current->flags.bSelected;
				}
				SmudgeCommon( pc );
				break;
			case KEY_ESCAPE:
				InvokeDefault( pc, INV_CANCEL );
				break;
			case KEY_ENTER:
				InvokeDefault( pc, INV_OKAY );
				break;
			}
		}
	}
   return 1;
}	

//---------------------------------------------------------------------------

//CONTROL_PROC_DEF( LISTBOX_CONTROL, LISTBOX, ListBox, (_32 attr) )
int CPROC InitListBox( PSI_CONTROL pc )
{
   //ARG( _32, attr );
	// there are no args to listbox...
	// there are options though - tree list, etc...
   // they should be passed!
   ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		S_32 width = 15;
		ScaleCoords( (PSI_CONTROL)pc, &width, NULL );

		plb->ListSurface = MakeSubImage( pc->Surface
												 , 0, 0
												 , pc->surface_rect.width - width
												 , pc->surface_rect.height );
      //plb->attr = 0; //attr;
		// test options here... but for now
		// we only need SINGLE Select - which should
		// be a 0 flag when choice is made.
		//if( plb->attr & LISTOPT_TREE )
	   //	plb->flags.bTree = TRUE;
		plb->flags.bSingle = TRUE;
		plb->flags.bNoUpdate = TRUE;
		plb->flags.bInitial = TRUE;
      plb->flags.bLazyMulti = FALSE;
		plb->pcScroll = MakePrivateControl( pc, SCROLLBAR_CONTROL
													 , pc->surface_rect.width-width, 0
													 , width, pc->surface_rect.height
													 , pc->nID );
		SetScrollUpdateMethod( plb->pcScroll, ScrollBarUpdate, (PTRSZVAL)pc );
		SetNoFocus( plb->pcScroll );
      return TRUE;
	}
   return FALSE;
}

//---------------------------------------------------------------------------

PSI_PROC( PSI_CONTROL, SetListboxIsTree )( PSI_CONTROL pc, int bTree )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		if( bTree )
			plb->flags.bTree = TRUE;
		else
			plb->flags.bTree = FALSE;
		SmudgeCommon( pc );
	}
   return pc;
}

//---------------------------------------------------------------------------

PLISTITEM InsertListItem( PSI_CONTROL pc, PLISTITEM pPrior, CTEXTSTR text )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		PLISTITEM pli = (PLISTITEM)Allocate( sizeof( LISTITEM ) );
		pli->text = StrDup( text );
      pli->pPopup = NULL;
		pli->flags.bSelected = FALSE;
		pli->flags.bFocused = FALSE;
		pli->flags.bOpen = FALSE;
      pli->nLevel = plb->nLastLevel;
		pli->data = 0;
      pli->within_list = pc;
		if( !pPrior )
		{
			if( ( pli->next = plb->items ) )
				plb->items->prior = pli;
			pli->prior = NULL;
			plb->items = pli;
			if( !plb->last )
			{
				plb->firstshown =
					plb->current =
					plb->last = plb->items;
				//pli->flags.bSelected = TRUE;
				//pli->flags.bFocused = TRUE;
			}
			else
			{
				plb->firstshown = NULL;
            plb->lastshown = NULL;
			}
		}
		else
		{
			if( !plb->last )
			{
				Log( WIDE("Hmm adding after an item in a list which has none?!") );
				DebugBreak();
			}
			if( ( pli->next = pPrior->next ) )
				pPrior->next->prior = pli;
			else
				plb->last = pli;
			pli->prior = pPrior;
			pPrior->next = pli;
		}
		if( !plb->flags.bNoUpdate )
		{
         //Log( WIDE("Added an item, therefore update this list?!") );
         // should only auto adjust when adding items...
         AdjustItemsIntoBox( pc );
			plb->flags.bInitial = TRUE;
			SmudgeCommon( pc );
		}
		return (PLISTITEM)pli;
	}
	return 0;
}

//---------------------------------------------------------------------------

PLISTITEM InsertListItemEx( PSI_CONTROL pc, PLISTITEM pPrior, int nLevel, CTEXTSTR text )
{
	PLISTITEM pli = InsertListItem( pc, pPrior, text );
	pli->nLevel = nLevel;
   return pli;
}

//---------------------------------------------------------------------------

PLISTITEM AddListItemEx( PSI_CONTROL pc, int nLevel, const TEXTCHAR *text )
{
   PLISTITEM pli = NULL;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		// sort by default.
		if( !plb->flags.bTree && plb->flags.bSortNormal )
		{
			PLISTITEM find;
			find = plb->items;
			while( find )
			{
				if( strcmp( find->text, text ) > 0 )
				{
					pli = InsertListItem( pc, find->prior, text );
					break;
				}
				find = find->next;
			}
			if( !find )
				goto add_at_end;
		}
		else
		{
		add_at_end:
         pli = InsertListItem( pc, plb->last, text );
		}
      pli->nLevel = nLevel;
		if( !plb->flags.bNoUpdate )
		{
         int bOpen = TRUE;
			if( nLevel )
			{
				PLISTITEM parent;
				parent = pli->prior;
				while( parent && parent->nLevel >= nLevel )
					parent = parent->prior;
            if( parent )
					bOpen = parent->flags.bOpen;
				else
               bOpen = TRUE; // ROOT of tree is always open..
			}
         //Log( WIDE("Added an item, therefore update this list?!") );
         // should only auto adjust when adding items...
			if( bOpen )
			{
				AdjustItemsIntoBox( pc );
				plb->flags.bInitial = TRUE;
			}
			SmudgeCommon( pc );
		}
		return (PLISTITEM)pli;
	}
	return 0;
}

//---------------------------------------------------------------------------

PLISTITEM AddListItem( PSI_CONTROL pc, CTEXTSTR text )
{
	PLISTITEM pli = AddListItemEx( pc, 0, text );
	return pli;
}

//---------------------------------------------------------------------------

void SetItemSelected( PSI_CONTROL pc, PLISTITEM pli, int bSelect )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		if( !plb->flags.bLazyMulti && ( plb->flags.bSingle && bSelect ) )
			ClearSelectedItems( pc );
		if( pli )
		{
			if( pli->flags.bSelected != bSelect )
			{
				pli->flags.bSelected = bSelect;
				DispatchSelectionChanged( plb, pc, pli );
				SmudgeCommon( pc );
			}
		}
	}	
}

//---------------------------------------------------------------------------

void SetSelectedItem( PSI_CONTROL pc, PLISTITEM hli )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		if( !plb->flags.bLazyMulti && plb->flags.bSingle )
			ClearSelectedItems( pc );
		if( hli )
		{
			PLISTITEM pli = (PLISTITEM)hli;
			pli->flags.bSelected = TRUE;
			plb->current = pli;
			DispatchSelectionChanged( plb, pc, pli );
		}
		SmudgeCommon( pc );
	}	
}

//---------------------------------------------------------------------------

void SetCurrentItem( PSI_CONTROL pc, PLISTITEM hli )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb && hli )
	{
		PLISTITEM pli = (PLISTITEM)hli;
		if( plb->current != pli )
		{
			plb->current = pli;
			SmudgeCommon( pc );
		}
	}
}

//---------------------------------------------------------------------------

PLISTITEM SetItemData( PLISTITEM hli, PTRSZVAL psv )
{
	PLISTITEM pli = (PLISTITEM)hli;
	if( hli )
		pli->data = psv;
	return hli;
}

//---------------------------------------------------------------------------

PTRSZVAL GetItemData( PLISTITEM hli )
{
	PLISTITEM pli = (PLISTITEM)hli;
	if( pli )
		return pli->data;
	return 0;
}

//---------------------------------------------------------------------------

PSI_PROC( void, SetItemContextMenu )( PLISTITEM pli, PMENU pMenu, void (CPROC*MenuProc)(PTRSZVAL, PLISTITEM, _32 menuopt ), PTRSZVAL psv )
{
	if( pli )
	{
		pli->psvContextMenu = psv;
		pli->pPopup = pMenu;
		pli->MenuProc = MenuProc;
	}
}

//---------------------------------------------------------------------------

PSI_PROC( PLISTITEM, GetSelectedItem )( PSI_CONTROL pc )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		PLISTITEM pli;
		pli = plb->items;
		while( pli )
		{
			if( pli->flags.bSelected )
				return (PLISTITEM)pli;
			pli = pli->next;
		}
	}
	return 0;
}

//---------------------------------------------------------------------------

#undef GetItemText
PSI_PROC( void, GetListItemText )( PLISTITEM hli, TEXTSTR buffer, int bufsize )
{
	if( hli )
	{
		StrCpyEx( buffer, ((PLISTITEM)hli)->text, ((bufsize)/sizeof(TEXTCHAR))-1 );
		buffer[bufsize-1] = 0;
	}
}

PSI_PROC( void, GetItemText )( PLISTITEM hli, int bufsize, TEXTSTR buffer )
{
   GetListItemText( hli, buffer, bufsize );
}

//---------------------------------------------------------------------------

PSI_PROC( void, SetItemText )( PLISTITEM hli, CTEXTSTR buffer )
{
	if( hli )
	{
		if( hli->text )
         Release( hli->text );
		hli->text = StrDup( buffer );
      SmudgeCommon( hli->within_list );
	}
}

//---------------------------------------------------------------------------

PSI_PROC( int, GetSelectedItems )( PSI_CONTROL pc, PLISTITEM *pList, int *nSize )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{

	}
	return 0;
}

//---------------------------------------------------------------------------

PSI_PROC( PLISTITEM, FindListItem )( PSI_CONTROL pc, CTEXTSTR text )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		PLISTITEM pli = plb->items;
		while( pli )
		{
			if( !strcmp( pli->text, text ) )
				break;
			pli = pli->next;
		}
		return (PLISTITEM)pli;
	}
	return 0;
}

//---------------------------------------------------------------------------
PSI_PROC( void, SetSelChangeHandler)( PSI_CONTROL pc, SelectionChanged proc, PTRSZVAL psvUser )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		plb->SelChangeHandler = proc;
		plb->psvSelChange = psvUser;
	}
}

//---------------------------------------------------------------------------

PSI_PROC( void, SetDoubleClickHandler )( PSI_CONTROL pc, DoubleClicker proc, PTRSZVAL psvUser )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		plb->DoubleClickHandler = proc;
		plb->psvDoubleClick = psvUser;
	}
}

//---------------------------------------------------------------------------

PSI_PROC( int, DisableUpdateListBox )( PSI_CONTROL pc, LOGICAL bDisable )
{
   int bSaved = 0;
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
      bSaved = plb->flags.bNoUpdate;
		if( !bDisable && plb->flags.bNoUpdate )
		{
         //Log( WIDE("Reenabling scrollbar updates causes an update... ") );
			// duh - must render before we know min/max/range...
			SmudgeCommon( pc );
			UpdateScrollForList( pc );
		}
      plb->flags.bNoUpdate = ( bDisable != FALSE );
	}
   return bSaved;
}

//---------------------------------------------------------------------------

void EnumSelectedListItems( PSI_CONTROL pc
								  , PLISTITEM pliStart
								  , void (CPROC *HandleListItem )(PTRSZVAL,PSI_CONTROL,PLISTITEM)
								  , PTRSZVAL psv )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb && plb->flags.bTree )
	{
		PLISTITEM pli = pliStart;
      int nLevel;
		if( !pli )
			pli =  plb->items;
		if( pli )
		{
			pli = pli->next;
			nLevel = pli->nLevel;
			while( pli )
			{
				if( pli->nLevel == nLevel )
				{
               if( pli->flags.bSelected )
						if( HandleListItem )
							HandleListItem( psv, pc, pli );
				}
				if( pli->nLevel < nLevel )
					break;
            pli = pli->next;
			}
		}
	}
	else if( plb )
	{
		PLISTITEM pli = pliStart;
		if( !pli )
			pli =  plb->items;
		if( pli )
		{
			while( pli )
			{
				if( pli->flags.bSelected )
					if( HandleListItem )
						HandleListItem( psv, pc, pli );
            pli = pli->next;
			}
		}
	}
}

//---------------------------------------------------------------------------

void EnumListItems( PSI_CONTROL pc
						, PLISTITEM pliStart
						, void (CPROC *HandleListItem )(PTRSZVAL,PSI_CONTROL,PLISTITEM)
						, PTRSZVAL psv )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb->flags.bTree )
	{
		PLISTITEM pli = pliStart;
      int nLevel;
		if( !pli )
			pli =  plb->items;
		if( pli )
		{
         pli = pli->next;
			nLevel = pli->nLevel;
			while( pli )
			{
				if( pli->nLevel == nLevel )
				{
					if( HandleListItem )
                  HandleListItem( psv, pc, pli );
				}
				if( pli->nLevel < nLevel )
					break;
            pli = pli->next;
			}
		}
	}
	else
	{
		PLISTITEM pli = pliStart;
		if( !pli )
			pli =  plb->items;
		if( pli )
		{
			while( pli )
			{
				if( HandleListItem )
					HandleListItem( psv, pc, pli );
			}
		}
	}
}

//---------------------------------------------------------------------------

PSI_PROC( void, SetListItemLevel )( PSI_CONTROL pc, int nLevel )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
      plb->nLastLevel = nLevel;
}

//---------------------------------------------------------------------------

PSI_PROC( int, OpenListItem )( PLISTITEM pli, int bOpen )
{
	int prior;
	if( pli )
	{
		prior = pli->flags.bOpen;
		pli->flags.bOpen = ( bOpen != 0 );
		return prior;
	}
	return -1;
}


PSI_PROC( void, SetListItemOpenHandler )( PSI_CONTROL pc, ListItemOpened proc, PTRSZVAL psvUser )
{
	// this routine is called before the branch is actually opened and rendered...
   // allowing an application to fill in the tree dynamically....
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		plb->ListItemOpenHandler = proc;
		plb->psvOpenClose = psvUser;
	}
}

//---------------------------------------------------------------------------

PSI_CONTROL SetListboxSort( PSI_CONTROL pc, int bSortTrue ) // may someday add SORT_INVERSE?
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
      plb->flags.bSortNormal = 0;
		if( bSortTrue == 1 )
		{
         plb->flags.bSortNormal = 1;
		}

	}
   return pc;
}

PSI_CONTROL SetListboxMultiSelectEx( PSI_CONTROL pc, int bEnable, int bLazy )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
		plb->flags.bSingle = !bEnable;
      plb->flags.bLazyMulti = bLazy;
	}
   return pc;
}

PSI_CONTROL SetListboxMultiSelect( PSI_CONTROL pc, int bEnable )
{
	return SetListboxMultiSelectEx( pc, bEnable, FALSE );
}

int GetListboxMultiSelectEx( PSI_CONTROL pc, int *multi, int *lazy )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
   if( plb )
	{
		if( multi )
			(*multi) = !plb->flags.bSingle;
		if( lazy )
			(*lazy) = !plb->flags.bLazyMulti;
		return !plb->flags.bSingle;
	}
   return 0;
}
int GetListboxMultiSelect( PSI_CONTROL pc )
{
	return GetListboxMultiSelectEx( pc, NULL, NULL );
}

PSI_CONTROL GetItemListbox( PLISTITEM pli )
{
	if( pli )
      return pli->within_list;
   return NULL;
}

//---------------------------------------------------------------------------

#include <psi.h>
static CONTROL_REGISTRATION
listbox = { LISTBOX_CONTROL_NAME
			 , { {110, 73}, sizeof( LISTBOX ), BORDER_INVERT_THIN|BORDER_NOCAPTION }
			 , InitListBox
			 , NULL
			 , RenderListBox
          , MouseListBox
			 , KeyListControl
			 , DestroyListBox
};

static void CPROC ResizeListbox( PSI_CONTROL pc )
{
	ValidatedControlData( PLISTBOX, LISTBOX_CONTROL, plb, pc );
	if( plb )
	{
		S_32 width = 15;
		ScaleCoords( (PSI_CONTROL)pc, &width, NULL );
		// resize the scrollbar accordingly...
		MoveSizeCommon( plb->pcScroll , pc->surface_rect.width-width, 0
						  , width, pc->surface_rect.height
						  );
	}
}

PRIORITY_PRELOAD( RegisterListbox, PSI_PRELOAD_PRIORITY )
{
   DoRegisterControl( &listbox );
	SimpleRegisterMethod( PSI_ROOT_REGISTRY WIDE("/control/") LISTBOX_CONTROL_NAME WIDE("/rtti")
							  , ResizeListbox
							  , WIDE("void"), WIDE("resize"), WIDE("(PSI_CONTROL)") );
}

PSI_LISTBOX_NAMESPACE_END

//---------------------------------------------------------------------------
//
// $Log: ctllistbox.c,v $
// Revision 1.73  2005/04/11 18:54:41  jim
// Added smudge when item text is updated... added a listbox pointer to the item... fixed function type return warnings
//
// Revision 1.72  2005/04/11 18:42:00  jim
// Should use smudagecommon instead of updatecommon
//
// Revision 1.71  2005/03/23 02:43:07  panther
// Okay probably a couple more badly initialized 'unusable' flags.. but font rendering/picking seems to work again.
//
// Revision 1.70  2005/03/07 00:12:46  panther
// Ahh need to pass attribute flags to MakeControl(SCROLLBAR)... otherwise it could be horizontal...
//
// Revision 1.69  2005/03/04 19:07:32  panther
// Define SetItemText
//
// Revision 1.68  2005/03/02 22:12:59  panther
// Add enueration method for listbox items - if a tree list - enumerates only those things under the current branch.
//
// Revision 1.67  2005/02/10 16:55:51  panther
// Fixing warnings...
//
// Revision 1.66  2004/12/16 10:46:04  panther
// Scrollbars work again!
//
// Revision 1.65  2004/12/16 10:32:45  panther
// Scroll scrollbar to font... handle rendering the top and bottom buttons better... next to restore function to scrollbars.
//
// Revision 1.64  2004/12/16 06:53:30  panther
// Minor updates for edit controls
//
// Revision 1.63  2004/12/15 03:00:19  panther
// Begin coding to only show valid, renderable fonts in dialog, and update cache, turns out that we'll have to postprocess the cache to remove unused dictionary entries
//
// Revision 1.62  2004/12/14 14:40:23  panther
// Fix slider control to handle the attr flags again for direction.  Fixed font selector dialog to close coorectly, update between controls correctly, etc
//
// Revision 1.61  2004/11/10 21:16:42  d3x0r
// OOPs passed private data instead of common control to update function
//
// Revision 1.60  2004/10/31 17:22:28  d3x0r
// Minor fixes to control library...
//
// Revision 1.59  2004/10/24 20:09:47  d3x0r
// Sync to psilib2... stable enough to call it mainstream.
//
// Revision 1.6  2004/10/10 09:07:58  d3x0r
// Short a couple frames on some of the updates... but looks like progress is positive.
//
// Revision 1.5  2004/10/08 13:07:42  d3x0r
// Okay beginning to look a lot like PRO-GRESS
//
// Revision 1.4  2004/10/07 04:37:16  d3x0r
// Okay palette and listbox seem to nearly work... controls draw, now about that mouse... looks like my prior way of cheating is harder to step away from than I thought.
//
// Revision 1.3  2004/10/06 09:52:16  d3x0r
// checkpoint... total conversion... now how does it work?
//
// Revision 1.2  2004/09/27 20:44:28  d3x0r
// Sweeping changes only a couple modules left...
//
// Revision 1.1  2004/09/19 19:22:31  d3x0r
// Begin version 2 psilib...
//
// Revision 1.57  2004/09/12 02:07:19  d3x0r
// Checkpoint.
//
// Revision 1.56  2004/09/04 18:49:48  d3x0r
// Changes to support scaling and font selection of dialogs
//
// Revision 1.55  2004/09/03 14:43:48  d3x0r
// flexible frame reactions to font changes...
//
// Revision 1.54  2004/09/02 22:01:33  d3x0r
// Extended PSI Controls to have fonts on frames, and some controls to have private fonts.
//
// Revision 1.53  2004/08/29 18:52:12  d3x0r
// Yeah it's still broke.... need to do some work with hide/show child.
//
// Revision 1.52  2004/08/25 15:01:06  d3x0r
// Checkpoint - more vc compat fixes
//
// Revision 1.51  2004/08/24 17:18:00  d3x0r
// Fix last couple c files for new control_proc_def macro
//
// Revision 1.50  2004/07/12 21:21:43  d3x0r
// Extend listbox/treelist a bit - added on open callback...
//
// Revision 1.49  2004/04/16 00:54:51  d3x0r
// checkpoint
//
// Revision 1.48  2004/03/29 21:24:26  d3x0r
// Double click handler was invoked before current selection was current.
//
// Revision 1.47  2003/11/29 00:10:28  panther
// Minor fixes for typecast equation
//
// Revision 1.46  2003/10/24 14:59:21  panther
// Added Load/Unload Function for system shared library abstraction
//
// Revision 1.45  2003/10/23 15:43:39  panther
// Fix disabled text on buttons.  Attempt 2 to fix tree drawing
//
// Revision 1.44  2003/10/22 10:44:22  panther
// Reverse stacking of AddListItem and AddListItemEx
//
// Revision 1.43  2003/10/14 16:24:53  panther
// Remove some logging.  Add new function to externally open/close tree items
//
// Revision 1.42  2003/10/13 02:50:47  panther
// Font's don't seem to work - lots of logging added back in
// display does work - but only if 0,0 biased, cause the SDL layer sucks.
//
// Revision 1.41  2003/10/12 02:47:05  panther
// Cleaned up most var-arg stack abuse ARM seems to work.
//
// Revision 1.40  2003/10/06 16:23:58  panther
// Fix scrolling with tree controls (may break indexed item operations?)
//
// Revision 1.39  2003/10/06 15:50:09  panther
// Somewhat fix scollbar for trees
//
// Revision 1.38  2003/10/06 15:07:03  panther
// Fixes to ctllist, adopting frames...
//
// Revision 1.37  2003/09/23 12:46:08  panther
// Okay Fixed most normal mouse operations - now to phase back in the edit features
//
// Revision 1.36  2003/09/22 15:14:53  panther
// Stupid logging message causing crash...
//
// Revision 1.35  2003/09/22 12:32:23  panther
// Interface to dynamically modify listbox behavior
//
// Revision 1.34  2003/09/22 10:45:08  panther
// Implement tree behavior in standard list control
//
// Revision 1.33  2003/09/11 13:09:25  panther
// Looks like we maintained integrety while overhauling the Make/Create/Init/Config interface for controls
//
// Revision 1.32  2003/09/04 11:18:32  panther
// Fix scrollbar/listbox issues - current item - insertion of sorted items, min/max/range
//
// Revision 1.31  2003/08/27 07:58:39  panther
// Lots of fixes from testing null pointers in listbox, font generation exception protection
//
// Revision 1.30  2003/07/25 10:48:28  panther
// Another fix - another control down.
//
// Revision 1.29  2003/07/24 23:47:22  panther
// 3rd pass visit of CPROC(cdecl) updates for callbacks/interfaces
//
// Revision 1.28  2003/06/04 11:39:08  panther
// Added some logging for listbox/scrollbar controls
//
// Revision 1.27  2003/05/01 21:31:57  panther
// Cleaned up from having moved several methods into frame/control common space
//
// Revision 1.26  2003/03/29 15:52:17  panther
// Fix service PSI compilation (direct link to render/image).  Just for grins make caption text 3dlike
//
// Revision 1.25  2003/03/28 22:37:28  panther
// Move control/frame focus var to common.
//
// Revision 1.24  2003/03/25 08:45:56  panther
// Added CVS logging tag
//
