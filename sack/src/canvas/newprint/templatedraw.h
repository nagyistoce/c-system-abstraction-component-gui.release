


#include <../canvas.h>
#include "template.h"

void PutImageOnDevice( PCANVAS hDC
							, int x, int y
							, int w, int h
							, Image pImage );


void DrawTexts( PCANVAS hdc, PTEMPLATE template );
void DrawRects( PCANVAS hdc, PTEMPLATE template );
void DrawLines( PCANVAS hdc, PTEMPLATE template );
void DrawImages( PCANVAS hdc, PTEMPLATE template );
void DrawMacros( PCANVAS hdc, PTEMPLATE template );
void DrawBarcodes( PCANVAS hdc, PTEMPLATE template );