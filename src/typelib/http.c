
#include <stdhdrs.h>

#include "http.h"

HTTP_NAMESPACE


struct HttpField {
	PTEXT name;
	PTEXT value;
};

struct HttpState {
   // add input into pvt_collector
	PVARTEXT pvt_collector;
	PTEXT partial;  // an accumulator that moves data from collector into whatever we've got leftover

	PTEXT response_status; // the first line of the http responce... 
	PLIST fields; // list of struct HttpField *
	_32 content_length;

	PTEXT content;

	int final; // boolean flag - indicates that the header portion of the http request is finished.
};

void GatherHttpData( struct HttpState *pHttpState )
{
	if( pHttpState->content_length )
	{
		PTEXT pMergedLine;
		PTEXT pInput = VarTextGet( pHttpState->pvt_collector );
		PTEXT pNewLine = SegAppend( pHttpState->partial, pInput );
		pMergedLine = SegConcat( NULL, pNewLine, 0, GetTextSize( pHttpState->partial ) + GetTextSize( pInput ) );
		LineRelease( pNewLine );
		pHttpState->partial = pMergedLine;
		if( GetTextSize( pHttpState->partial ) >= pHttpState->content_length ) 
		{
			pHttpState->content = SegSplit( &pHttpState->partial, pHttpState->content_length );
			pHttpState->partial = NEXTLINE( pHttpState->partial );		
			SegGrab( pHttpState->partial );
		}
	}
	else
	{
		PTEXT pMergedLine;
		PTEXT pInput = VarTextGet( pHttpState->pvt_collector );
		PTEXT pNewLine = SegAppend( pHttpState->partial, pInput );
		pMergedLine = SegConcat( NULL, pNewLine, 0, GetTextSize( pHttpState->partial ) + GetTextSize( pInput ) );
		LineRelease( pNewLine );
		pHttpState->partial = pMergedLine;

		pHttpState->content = pHttpState->partial;
	}
}

int ProcessHttp( struct HttpState *pHttpState )
{
	if( pHttpState->final )
	{
		GatherHttpData( pHttpState );
	}
	else
	{
		PTEXT pCurrent, pStart;
		PTEXT pLine = NULL,
			 pHeader = NULL;
		char *c, *l;
		int size, pos, len;
		int bLine;
		int start = 0;
		PTEXT pMergedLine;
		PTEXT pInput = VarTextGet( pHttpState->pvt_collector );
		PTEXT pNewLine = SegAppend( pHttpState->partial, pInput );
		pMergedLine = SegConcat( NULL, pNewLine, 0, GetTextSize( pHttpState->partial ) + GetTextSize( pInput ) );
		LineRelease( pNewLine );
		pHttpState->partial = pMergedLine;
		pCurrent = pHttpState->partial;
		pStart = pCurrent; // at lest is this block....
		len = 0;

		// we always start without having a line yet, because all input is already merged
		bLine = 0;
		{
			size = GetTextSize( pCurrent );
			c = GetText( pCurrent );
			if( bLine < 4 )
			{
				//start = 0; // new packet and still collecting header....
				for( pos = 0; ( pos < size ) && !pHttpState->final; pos++ )
				{
					if( c[pos] == '\r' )
						bLine++;
					else if( c[pos] == '\n' )
						bLine++;
					else // non end of line character....
					{
	FinalCheck:
						if( bLine >= 2 ) // had an end of line...
						{
							//CTEXTSTR tmp = StrChr( 
							if( pHttpState->response_status )
							{
								CTEXTSTR field_start;
								CTEXTSTR colon;
								CTEXTSTR field_end;
								CTEXTSTR val_start;
								PTEXT field_name;
								PTEXT value;
								pLine = SegCreate( pos - start - bLine );
								MemCpy( l = GetText( pLine ), c + start, pos - start - bLine);
								l[pos-start-bLine] = 0;
								field_start = GetText( pLine );
								colon = StrChr( field_start, ':' );
								if( colon )
								{
									PTEXT trash;
									val_start = colon;
									field_end = colon;
									while( ( field_end > field_start )&& field_end[-1] == ' ' )
										field_end--;
									while( ( val_start[0] && ( val_start[1] == ' ' ) ) )
										val_start++;

									SegSplit( &pLine, val_start - field_start );
									value = NEXTLINE( pLine );
									field_name = SegSplit( &pLine, field_end - field_start );
									trash = NEXTLINE( field_name );
									{
										struct HttpField *field = New( struct HttpField );
										field->name = SegGrab( field_name );
										field->value = SegGrab( value );
										LineRelease( trash );
										AddLink( &pHttpState->fields, field );
									}
								}
								else
								{
									lprintf( "Header field [%s] invalid", GetText( pLine ) );
									LineRelease( pLine );
								}
							}
							else
							{
								pLine = SegCreate( pos - start - bLine );
								MemCpy( l = GetText( pLine ), c + start, pos - start - bLine);
								l[pos-start-bLine] = 0;
								pHttpState->response_status = pLine;
							}
							// could perhaps append a newline segment block...
							// but probably do not need such a thing....
							// since the return should be assumed as a continuous
							// stream of datas....
							start = pos;
							if( bLine == 2 )
								bLine = 0;
						}
						// may not receive anything other than header information?
						if( bLine == 4 )
						{
							// end of header
							// copy the previous line out...
							pStart = pCurrent;
							len = size - pos; // remaing size
							break;
						}
					}
					if( bLine == 4 )
					{
						pos++;
						pHttpState->final = 1;
						goto FinalCheck;
					}
				}
				if( pos == size &&
					bLine == 4 &&
					start != pos )
				{
					pHttpState->final = 1;
					goto FinalCheck;
				}
			}
			else
				len += size;
			//pCurrent = NEXTLINE( pCurrent );
			/* Move the remaining data into a single binary data packet...*/
		}
		if( start )
		{
			PTEXT tmp = SegSplit( &pCurrent, start );
			pHttpState->partial = NEXTLINE( pCurrent );
			LineRelease( SegGrab( pCurrent ) );
			start = 0;
		}

		if( pHttpState->final )
		{
			INDEX idx;
			struct HttpField *field;
			LIST_FORALL( pHttpState->fields, idx, struct HttpField *, field )
			{
				if( TextLike( field->name, "content-length" ) )
				{
					pHttpState->content_length = atol( GetText( field->value ) );
					break;
				}
			}
			// do one gather here... with whatever remainder we had.
			GatherHttpData( pHttpState );
		}
	}
	return pHttpState->final && pHttpState->content;
}

void AddHttpData( struct HttpState *pHttpState, POINTER buffer, int size )
{
	VarTextAddData( pHttpState->pvt_collector, (CTEXTSTR)buffer, size );
}

struct HttpState *CreateHttpState( void )
{
	struct HttpState *pHttpState;

	pHttpState = New( struct HttpState );
	pHttpState->pvt_collector = VarTextCreate();
	pHttpState->partial = NULL;
	pHttpState->content_length = 0;
	pHttpState->fields = NULL;
	pHttpState->response_status = NULL;
	pHttpState->final = 0;
	pHttpState->content = NULL;
	return pHttpState;
}


void EndHttp( struct HttpState *pHttpState )
{
	pHttpState->final = 0;

	pHttpState->content_length = 0;
	LineRelease( pHttpState->content );
	pHttpState->content = NULL;

	LineRelease( pHttpState->response_status );
	pHttpState->response_status = NULL;

	{
		INDEX idx;
		struct HttpField *field;
		LIST_FORALL( pHttpState->fields, idx, struct HttpField *, field )
		{
			LineRelease( field->name );
			LineRelease( field->value );
			Release( field );
		}
		EmptyList( &pHttpState->fields );
	}

}

PTEXT GetHttpContent( struct HttpState *pHttpState )
{
	if( pHttpState->final )
		return pHttpState->content;
	return NULL;
}

void ProcessHttpFields( struct HttpState *pHttpState, void (CPROC*f)( PTRSZVAL psv, PTEXT name, PTEXT value ), PTRSZVAL psv )
{
	INDEX idx;
	struct HttpField *field;
	LIST_FORALL( pHttpState->fields, idx, struct HttpField *, field )
	{
		f( psv, field->name, field->value );
	}
}

PTEXT GetHttpResponce( struct HttpState *pHttpState )
{
	return pHttpState->response_status;
}

void DestroyHttpState( struct HttpState *pHttpState )
{
	EndHttp( pHttpState );
	DeleteList( &pHttpState->fields );
   VarTextDestroy( &pHttpState->pvt_collector );
   Release( pHttpState );
}


HTTP_NAMESPACE_END
