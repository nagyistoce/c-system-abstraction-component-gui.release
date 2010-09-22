#include <windowsx.h>
#include <bingcard.h>
#include <bingfile.h>
//#include <verify.h>


#define MAXCPP	256			// Maximum Cards Per Pack, should this be 256, to account for
							// Having multiple packs per "pack" sold
typedef struct
{
	char	cPackName[32];
    BYTE	byPackSize;
    BYTE	byNumCards;
    COLORREF	PackColor;
	DWORD	dwCardNumArray[MAXCPP];	// The serial number of each card 
    BYTE	byCardData[MAXCPP][25];	// The numbers that appear on each card (moving down each col)
    								// The high bit of each number indicates whether to daub it
	BYTE	bySpecialCells[12];		// Position of each special number in pack
	BYTE	bySpecialCellCount;		// The number of special cells
	BYTE	bySpecialNumberLimit;	// For how many balls the special cells are valid
	WORD	wPackStatus;				// VER_PACKBINGO = pack as whole has bingo (multicard bingo)
    								// VER_CARDBINGO = one or more cards has a bingo
                                    // VER_SPECIALBINGO = the pack has a special number bingo
                                    // VER_NOBINGO = there is no bingo on this pack
                                    // VER_INVALID = this structure does not define a valid pack
    BYTE	byCardStatus[MAXCPP];		// 0=no bingo on card, 1=bingo on that card
	BYTE	byPackType;				// Regular or Double Action GI_ANORMAL or GI_ADOUBLE 
} BINGPACKDATA;

typedef	BINGPACKDATA		* PBINGPACKDATA;
typedef	BINGPACKDATA		FAR * LPBINGPACKDATA;
typedef	BINGPACKDATA		huge * HPBINGPACKDATA;


HWND	PASCAL FAR _export	BINGPACKCreatePackWindow ( 	HWND hParentWnd,
														HINSTANCE hInstance,
														HMENU WindowID,
														WORD wXPos,
														WORD wYPos,
														WORD wWidth,
														WORD wHeight,
                                                        //LPVERPACKDATA lpVerPackData,
                                                        WORD wLastNumberCalled
													);

HWND	PASCAL FAR _export	BINGPACKCreatePackWindow2 (	HWND hParentWnd,
														HINSTANCE hInstance,
														HMENU WindowID,
														WORD wXPos,
														WORD wYPos,
														WORD wWidth,
														WORD wHeight,
                                                        //LPVERPACKDATA lpVerPackData,
                                                        WORD wLastNumberCalled,
														BOOL fWide
													);

//--------------------------------------------------------------------------

// for addition information - any of these may be set to NULL 
// indicating the information is not to be printed, or is not available...
// however the point stands that the line assocated with a NULL parameter
// will not be displayed...

typedef struct PackInformation_tag {
	char *PlayerID;    // Player ID
	char *POSID;       // point of sale ID
	char *SessionID;   // session ID
	char *Series;      // Card series....
	char *PackID;      // 9 digit pack ID
	char *GameNumber;  // 1... 2-3... 1A etc... (bold, large, font)
	char *Price;
	char *TransactionID; 
	char *Game;        // game type (Letter X)
	char *GameTitle1;  // column title (BLUE/RED etc)
	char *GameTitle2;  // column title (BLUE/RED etc)
	char *Date;       // date pack is good for
	char *Time;       // time redundant? may be in sessionID?
	char *Aux1;       // to be determined - expansive space
	char *Aux2;       // to be determined - expansive space
} PRINT_PACKINFO, *PPRINT_PACKINFO;

// these functions currently print a card set at a time
// creating and destroying the device context if appropriate or using
// the device context passed from the caller.  
// These functions return ERROR_SUCCESS(0) if successful, and
// the error result from the printing methods if there was a failure.
// This will allow the user application to handle or report errors, 
// such as paper out, etc... 

int FAR _export BINGCARDPrintCard( LPCSTR driver
											, LPCSTR device
											, LPCSTR output
											, LPBINGPACKDATA lpBINGPackData
											, char *templatename
											, PPRINT_PACKINFO pPackInfo 
											);


int BINGOCARDInitPrinting( char *device );
int BINGOCARDLoadTemplate( char *templatename, char *templatefilename );

enum printerror {
	  PRINT_ERROR_SUCCESS = 0      // 0 = no error
	, PRINT_ERROR_StartDoc    // StartDoc Failed
   , PRINT_ERROR_StartPage    // StartPage failed 
   , PRINT_ERROR_EndPage          // SP_ERROR // result directly from EndPage...
	, PRINT_ERROR_EndPageAbort     // SP_APPABORT
	, PRINT_ERROR_EndPageUserAbort // SP_USERABORT
	, PRINT_ERROR_EndPageOutOfDisk // SP_OUTOFDISK
	, PRINT_ERROR_EndPageOutOfMemory // when endpage returns SP_OUTOFMEMORY
	, PRINT_ERROR_EndDoc  // when end doc failes
	, PRINT_ERROR_NO_DATA // WHEN THE Pack data pointer was NULL
	, PRINT_ERROR_CreateDC // results when the parameters specified to create a DC fail
	, PRINT_ERROR_INVALID_PRINTER_NAME // when any of the name type parameters to print are NULL
	, PRINT_ERROR_NO_TEMPLATE // name specified as a template was not found.
	, PRINT_ERROR_CANTOPEN // OpenPrinter Failed... 
};
