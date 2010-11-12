#include <stdhdrs.h>
#include <fractions.h>
#include <interface.h>
#include <keybrd.h>
#include <system.h>

#include "controlstruc.h"
#include <psi.h>
#include "mouse.h"
#include "borders.h"
#include "resource.h"

//#define DEBUG_UPDAATE_DRAW 4

PSI_NAMESPACE

static void CPROC FileDroppedOnFrame( PTRSZVAL psvControl, CTEXTSTR filename, S_32 x, S_32 y )
{
	PSI_CONTROL frame = (PSI_CONTROL)psvControl;
	if( frame )
	{
		x -= frame->surface_rect.x;
		y -= frame->surface_rect.y;
		{
			int found = 0;
			PSI_CONTROL current;
			for( current = frame->child; current; current = current->next )
			{
				if( ( x < current->original_rect.x ) || 
					( y < current->original_rect.y ) || 
					( SUS_GT( x, S_32, ( current->original_rect.x + current->original_rect.width ) , _32 ) ) || 
					( SUS_GT( y, S_32, ( current->original_rect.y + current->original_rect.height ), _32 ) ) )
				{
					continue;
				}
				found = 1;
				FileDroppedOnFrame( (PTRSZVAL)current, filename
						, x - (current->original_rect.x )
						, y - (current->original_rect.y ) );
			}
			if( !found )
			{
				InvokeMethod( frame, AcceptDroppedFiles, (frame, filename, x, y ) );
			}
			///////////////
		}
	}
}

//---------------------------------------------------------------------------

static void CPROC FrameClose( PTRSZVAL psv )
{
	PPHYSICAL_DEVICE device = (PPHYSICAL_DEVICE)psv;
	PSI_CONTROL common;
	device->pActImg = NULL;
	DeleteLink( &g.shown_frames, device->common );
	common = device->common;
	DestroyCommon( &common );
}

//---------------------------------------------------------------------------

static void CPROC FrameRedraw( PTRSZVAL psvFrame, PRENDERER psvSelf )
{
   PPHYSICAL_DEVICE pf = (PPHYSICAL_DEVICE)psvFrame;
	_32 update = 0;
	PSI_CONTROL pc;
	//lprintf( WIDE("frame %p"), pf );
		
	pc = pf->common;
	if( !pc ) // might (and probalby isn't) attached to anything yet.
	{
#ifdef DEBUG_UPDAATE_DRAW
		Log( WIDE("no frame... early return") );
#endif
		return;
	}
#ifdef DEBUG_UPDAATE_DRAW
	lprintf( WIDE( " ------------- BEGIN FRAME DRAW -----------------" ) );
#endif
   pc->flags.bShown = 1;
	GetCurrentDisplaySurface(pf);
	if( pc->flags.bDirty || pc->flags.bResizedDirty )
	{
		pc->flags.bResizedDirty = 0;
      pc->flags.bDirty = 1;
#ifdef DEBUG_UPDAATE_DRAW
		Log( WIDE("Redraw frame...") );
#endif
		AddUse( pc );

		if( pc->flags.bTransparent && pc->flags.bFirstCleaning )
		{
			Image OldSurface;
#ifdef DEBUG_UPDAATE_DRAW
			lprintf( WIDE( "!!Saving old image... (on frame)" ) );
#endif
			if( ( OldSurface = CopyOriginalSurface( pc, pc->OriginalSurface ) ) )
			{
				pc->OriginalSurface = OldSurface;
			}
			else
				if( pc->OriginalSurface )
				{
#ifdef DEBUG_UPDAATE_DRAW
					lprintf( WIDE( "------------ Restoring old image..." ) );
					lprintf( WIDE( "Restoring orignal background... " ) );
#endif
					BlotImage( pc->Surface, pc->OriginalSurface, 0, 0 );
				}
		}
		//pc->flags.bParentCleaned = 1; // has now drawn itself, and we must assume that it's not clean.

		// surface could have changed...
		if( pc->Window->width != pc->rect.width )
		{
			pc->rect.width = pc->Window->width;
			update++;
		}
		if( pc->Window->height != pc->rect.height )
		{
			pc->rect.height = pc->Window->height;
			update++;
		}
		// but then again...
		//update++; // what if we only moved, and the driver requires a refresh?

#ifdef __DISPLAY_NO_BUFFER__
		lprintf( WIDE("REDRAW?!") );
#endif
		// if using "displaylib"
		//if( update )
		{
			//lprintf( WIDE("Recomputing border...") );
			// fix up surface rect.
			{
            extern void UpdateSurface( PSI_CONTROL pc );
            UpdateSurface( pc );
			}
			//SetCommonBorder( pc, pc->BorderType );
			if( pc->DrawBorder )
			{
#ifdef DEBUG_BORDER_DRAWING
				lprintf( "Drawing border here too.." );
#endif
				pc->DrawBorder( pc );
			}
			// probably should just invoke draw... but then we won't get marked
			// dirty - so redundant smudges wont be merged... and we'll do this all twice.
#ifdef DEBUG_UPDAATE_DRAW
			lprintf( WIDE( "Smudging the form... %p" ), pc );
#endif
		}
		//SmudgeCommon( pc );
		//else
#ifdef DEBUG_UPDAATE_DRAW
		lprintf( WIDE("delete use should refresh rectangle. %p"), pc );
#endif
		DeleteUse( pc );
	}
	else
	{
#ifdef DEBUG_UPDAATE_DRAW
		lprintf( WIDE( "trusting that the frame is already drawn to the stable buffer..." ) );
#endif
		UpdateDisplay( pf->pActImg );
	}
}

//---------------------------------------------------------------------------

static void CPROC FrameFocusProc( PTRSZVAL psvFrame, PRENDERER loss )
{
   int added_use = 0;
	PPHYSICAL_DEVICE frame = (PPHYSICAL_DEVICE)psvFrame;
	//PFRAME frame = (PFRAME)psvFrame;
	PSI_CONTROL pc = frame->common;
	if( pc->flags.bShown )
	{
      added_use = 1;
		AddUse( pc );
	}
	GetCurrentDisplaySurface(frame);
	if( loss )
	{
		if( frame->pFocus )
			frame->pFocus->flags.bFocused = 0;
		pc->flags.bFocused = 0;
	}
	else
	{
		pc->flags.bFocused = 1;
		if( frame->pFocus && ( frame->pFocus != frame->common ) )
		{
			frame->pFocus->flags.bFocused = 1;
		}
	}
	if( pc->flags.bInitial )
	{
		// still in the middle of displaying... this is a false draw point.
      if( added_use )
			DeleteUse( pc );
		return;
	}
#ifdef DEBUG_UPDAATE_DRAW
	else
      lprintf( WIDE( "Frame is not initial..." ) );
#endif

//#ifdef DEBUG_UPDAATE_DRAW
#ifdef DEBUG_FOCUS_STUFF
	Log1( WIDE("PSI Focus change called: %p"), loss );
#endif
//#endif
	if( loss )
	{
		if( frame->pFocus && ( frame->pFocus != frame->common ) )
		{
#ifdef DEBUG_FOCUS_STUFF
			lprintf( WIDE("Dispatch to current focused control also?") );
#endif
			frame->pFocus->flags.bFocused = 0;
			if( frame->pFocus->ChangeFocus )
				frame->pFocus->ChangeFocus( frame->pFocus, FALSE );
		}
#ifdef DEBUG_FOCUS_STUFF
		lprintf( WIDE("Control lost focus. (the frame itself loses focus)") );
#endif
		pc->flags.bFocused = 0;
		if( pc->ChangeFocus )
         pc->ChangeFocus( pc, FALSE );
	}
	else
	{
		pc->flags.bFocused = 1;
#ifdef DEBUG_FOCUS_STUFF
		lprintf( WIDE("Control gains focus. (the frame itself gains focus)") );
#endif
		if( pc->ChangeFocus )
         pc->ChangeFocus( pc, TRUE );
		if( frame->pFocus && ( frame->pFocus != frame->common ) )
		{
         AddUse( frame->pFocus );
			// cause we need to unfocus it's control also
			// otherwise we get stupid cursors...
			frame->pFocus->flags.bFocused = 1;
#ifdef DEBUG_FOCUS_STUFF
			lprintf( WIDE("Dispatch to current focused control also?") );
#endif
			if( frame->pFocus->ChangeFocus )
				frame->pFocus->ChangeFocus( frame->pFocus, FALSE );
			DeleteUse( frame->pFocus );
		}
	}
	if( !pc->flags.bHidden )
	{
		//void DrawFrameCaption( PSI_CONTROL pc );

		DrawFrameCaption( pc );
		// update just the caption portion?
		if( pc->surface_rect.y && !pc->flags.bRestoring )
		{
#ifdef DEBUG_FOCUS_STUFF
			lprintf( WIDE("Updating the frame caption...") );
			lprintf( WIDE("Update portion %d,%d to %d,%d"), 0, 0, pc->rect.width, pc->surface_rect.y );
			lprintf( WIDE( "Updating just the caption portion to the display" ) );
#endif
#ifdef DEBUG_UPDAATE_DRAW
			lprintf( WIDE("updating display portion %d,%d")
					 , pc->rect.width
					 , pc->surface_rect.y );
#endif
			UpdateDisplayPortion( frame->pActImg
									  , 0, 0
									  , pc->rect.width
									  , pc->surface_rect.y );
		}
		// and draw here...
	}
#ifdef DEBUG_UPDAATE_DRAW
	else
		lprintf( WIDE("Did not draw frame of hidden frame.") );
#endif
	if( added_use )
		DeleteUse( pc );
}

//---------------------------------------------------------------------------

static int CPROC FrameKeyProc( PTRSZVAL psvFrame, _32 key )
{
   PPHYSICAL_DEVICE pf = (PPHYSICAL_DEVICE)psvFrame;
	//PFRAME pf = (PFRAME)psvFrame;
	PSI_CONTROL pc = pf->common;
   int result = 0;
	if( pc->flags.bDestroy )
      return 0;
	AddUse( pc );
#ifdef DEBUG_KEY_EVENTS
	lprintf( WIDE("Added use for a key %08lx"), key );
#endif
	{
		if( pf->EditState.flags.bActive && pf->EditState.pCurrent )
		{
			//if( pf->EditState.pCurrent->KeyProc )
			//	pf->EditState.pCurrent->KeyProc( pf->EditState.pCurrent->psvKey, key );
			AddUse( pf->EditState.pCurrent );
#ifdef DEBUG_KEY_EVENTS
			lprintf( WIDE("invoking control use...") );
#endif
         InvokeResultingMethod( result, pf->EditState.pCurrent, _KeyProc, ( pf->EditState.pCurrent, key ) );
         DeleteUse( pf->EditState.pCurrent );
		}
		else if( pf->pFocus )
		{
			//if( pf->pFocus->KeyProc )
			//	pf->pFocus->KeyProc( pf->pFocus->psvKey, key );
			AddUse( pf->pFocus );
#ifdef DEBUG_KEY_EVENTS
			lprintf( WIDE("invoking control focus use...") );
#endif
			//	lprintf( WIDE("dispatch a key event to focused contro... ") );
			InvokeResultingMethod( result, pf->pFocus, _KeyProc, ( pf->pFocus, key ) );
			DeleteUse( pf->pFocus );
		}
	}
	// passed the key to the child window first...
   // if it did not process, then the frame can get a shot at it..
	if( !result && pc && pc->_KeyProc )
	{
#ifdef DEBUG_KEY_EVENTS
		lprintf( WIDE("Invoking control key method.") );
#endif
		InvokeResultingMethod( result, pc, _KeyProc, (pc,key));
		DeleteUse( pc );
		return result;
	}
	if( !result )
	{
		if( (KEY_CODE(key) == KEY_TAB) && (key & KEY_PRESSED))
		{
			//DebugBreak();
			if( !(key & (KEY_ALT_DOWN|KEY_CONTROL_DOWN)) ) // not control or alt...
			{
				if( key & KEY_SHIFT_DOWN ) // shift-tab backwards
				{
					FixFrameFocus( pf, FFF_BACKWARD );
					result = 1;
				}
				else
				{
					FixFrameFocus( pf, FFF_FORWARD );
					result = 1;
				}
			}
		}
		if( KEY_CODE(key) == KEY_ESCAPE && (key & KEY_PRESSED))
			result = InvokeDefault( pc, TRUE );
		if( KEY_CODE(key) == KEY_ENTER && (key & KEY_PRESSED))
			result = InvokeDefault( pc, FALSE );
	}
	DeleteUse( pc );
	return result;
}

//---------------------------------------------------------------------------
static int IsMeOrInMe( PSI_CONTROL isme, PSI_CONTROL pc )
{
	while( pc )
	{
		if( pc->child )
			if( IsMeOrInMe( isme, pc->child ) )
				return TRUE;
		if( isme == pc )
			return TRUE;
      pc = pc->next;
	}
   return FALSE;
}
//---------------------------------------------------------------------------


PPHYSICAL_DEVICE OpenPhysicalDevice( PSI_CONTROL pc, PSI_CONTROL over, PRENDERER pActImg, PSI_CONTROL under )
{
	if( pc && !pc->device )
	{
		//Image surface;
		PPHYSICAL_DEVICE device = (PPHYSICAL_DEVICE)Allocate( sizeof( PHYSICAL_DEVICE ) );
		MemSet( device, 0, sizeof( PHYSICAL_DEVICE ) );
		device->common = pc;
		pc->device = device;
		device->nIDDefaultOK = BTN_OKAY;
		device->nIDDefaultCancel = BTN_CANCEL;
		if( under )
			under = GetFrame( under );
		if( over )
			over = GetFrame( over );
		else
			if( pc->parent )
			{
				over = pc->parent;
				{
					PSI_CONTROL parent;
					for( parent = pc->parent; parent; parent = parent->parent )
					{
						if( parent->device )
						{
							if( IsMeOrInMe( parent->device->pFocus, pc ) )
							{
								//lprintf( "!!!!!!!!!!!! FIXED THE FOCUS!!!!!!!!!!" );
								parent->device->pFocus = NULL;
								break;
							}
						}
					}
				}
				//DebugBreak();
				//OrphanCommon( pc );
				// some other method to save this?
				// for now I can guess...
				// pc->device->EditState.parent = parent ?
				// leave it otherwise linked into the stack of controls...
				pc->parent = NULL;
			}
		if( !pActImg )
		{
#ifdef DEBUG_CREATE
			lprintf( WIDE("Creating a device to show this control on ... %d,%d %d,%d")
					 , pc->rect.x
					 , pc->rect.y
					 , pc->rect.width
					 , pc->rect.height );
#endif
         //lprintf( WIDE("Original show - extending frame bounds...") );
			pc->original_rect.width += FrameBorderX(pc->BorderType);
			pc->original_rect.height += FrameBorderY(pc, pc->BorderType, GetText( pc->caption.text ) );
         // apply scale to rect from original...
			pc->rect.width += FrameBorderX(pc->BorderType);
			pc->rect.height += FrameBorderY(pc, pc->BorderType, GetText( pc->caption.text ) );
			device->pActImg = OpenDisplayAboveUnderSizedAt( 0
																  , pc->rect.width
																  , pc->rect.height
																  , pc->rect.x
																  , pc->rect.y
																  , (over&&over->device)?over->device->pActImg:NULL
																  , (under&&under->device)?under->device->pActImg:NULL);
#ifdef WIN32
			WinShell_AcceptDroppedFiles( device->pActImg, FileDroppedOnFrame, (PTRSZVAL)pc );
#endif
         AddLink( &g.shown_frames, pc );
         SetRendererTitle( device->pActImg, GetText( pc->caption.text ) );
#ifdef DEBUG_CREATE
			lprintf( WIDE("Resulting with surface...") );
#endif
		}
		else
		{
			// have to resize the frame then to this display...
#ifdef DEBUG_CREATE
			lprintf( WIDE("Using externally assigned render surface...") );
			lprintf( WIDE("Adjusting the frame to that size?!") );
#endif
			if( pc->rect.x && pc->rect.y )
				MoveDisplay( pActImg, pc->rect.x, pc->rect.y );

			if( pc->rect.width && pc->rect.height )
				SizeDisplay( pActImg, pc->rect.width, pc->rect.height );
			else
			{
				_32 width, height;
				GetDisplaySize( &width, &height );
				SizeCommon( pc, width, height );
			}
			device->pActImg = pActImg;
#ifdef WIN32
			WinShell_AcceptDroppedFiles( device->pActImg, FileDroppedOnFrame, (PTRSZVAL)pc );
#endif
			AddLink( &g.shown_frames, pc );
         
		}
		pc->BorderType |= BORDER_FRAME; // mark this as outer frame... as a popup we still have 'parent'
		GetCurrentDisplaySurface( device );

		// sets up the surface iamge...
		// computes it's offset based on border type and caption
		// characteristics...
		// readjusts surface (again) after adoption.
		//lprintf( WIDE("------------------- COMMON BORDER RE-SET on draw -----------------") );

		TryLoadingFrameImage();

		if( g.BorderImage )
			SetCommonTransparent( pc, TRUE );
		SetCommonBorder( pc, pc->BorderType );

      // this routine is in Mouse.c
		SetMouseHandler( device->pActImg, AltFrameMouse, (PTRSZVAL)device );
		SetCloseHandler( device->pActImg, FrameClose, (PTRSZVAL)device );
		SetRedrawHandler( device->pActImg, FrameRedraw, (PTRSZVAL)device );
		SetKeyboardHandler( device->pActImg, FrameKeyProc, (PTRSZVAL)device );
		SetLoseFocusHandler( device->pActImg, FrameFocusProc, (PTRSZVAL)device );
		// these methods should aready be set by the creation above...
		// have to attach the mouse events to this frame...
	}
	if( pc )
		return pc->device;
   return NULL;
}

//---------------------------------------------------------------------------

void DetachFrameFromRenderer(PSI_CONTROL pc )
{
	if( pc->device )
	{
		PPHYSICAL_DEVICE pf = pc->device;
		//ValidatedControlData( PFRAME, CONTROL_FRAME, pf, pc );
		//lprintf( WIDE("Closing physical frame device...") );
		if( pf->EditState.flags.bActive )
		{
			// there may also be data in the edit state to take care of...
		}
		if( pf )
		{
			SetCloseHandler( pf->pActImg, NULL, 0 );
			// closedevice
			OrphanSubImage( pc->Surface );
			//UnmakeImageFile( pc->Surface );
			if( pf->pActImg )
				CloseDisplay( pf->pActImg );
			// closing the main image closes all children?
			pc->Window = NULL;
			// this is a subimage of window, and as such, is invalid now.
			//pc->Surface = NULL;
			Release( pf );
			// and that's about it, eh?
		}
		pc->device = NULL;
	}
}


//---------------------------------------------------------------------------
PSI_PROC( PSI_CONTROL, AttachFrameToRenderer )( PSI_CONTROL pc, PRENDERER pActImg )
{
	OpenPhysicalDevice( pc
							, pc?pc->parent:NULL
							, pActImg, NULL );
	return pc;
}

PSI_PROC( PSI_CONTROL, CreateFrameFromRenderer )( CTEXTSTR caption
                                           , _32 BorderTypeFlags
                                           , PRENDERER pActImg )
{
	PSI_CONTROL pc = NULL;
   S_32 x, y;
	_32 width, height;
#ifdef USE_INTERFACES
	GetMyInterface();
	if( !g.MyImageInterface )
		return NULL;
#endif
	GetDisplayPosition( pActImg, &x, &y, &width, &height );
	pc = MakeCaptionedControl( NULL, CONTROL_FRAME
									 , x, y, width, height
									 , 0
									 , caption
									 );
   AttachFrameToRenderer( pc, pActImg );
	SetCommonBorder( pc, BorderTypeFlags|((BorderTypeFlags & BORDER_WITHIN)?0:BORDER_FRAME) );
	return pc;
}

PSI_NAMESPACE_END

