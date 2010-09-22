

// some inter module linkings...
// these are private for access between utils and the core interface lib

#define OPTION_ROOT_VALUE 0

typedef struct sack_option_tree_family OPTION_TREE;
typedef struct sack_option_tree_family *POPTION_TREE;

struct sack_option_tree_family {
	PFAMILYTREE option_tree;
	PODBC odbc;  // each option tree associates with a ODBC connection.
	struct {
		BIT_FIELD bNewVersion : 1;
		BIT_FIELD bCreated : 1;
	} flags;
};

struct sack_option_global_tag {
   struct {
		BIT_FIELD  bInited  : 1;
		BIT_FIELD bUseProgramDefault : 1;
		BIT_FIELD bUseSystemDefault : 1;
		BIT_FIELD bPromptDefault : 1;
   } flags;
   char SystemName[128];
   INDEX SystemID;
   _32 Session;
	//PFAMILYTREE option_tree;
   PLIST trees; // list of struct sack_option_family_tree's
	PODBC Option;
   CRITICALSECTION cs_option;
};

INDEX GetOptionIndexEx( INDEX parent, const char *file, const char *pBranch, const char *pValue, int bCreate DBG_PASS );
#define GetOptionIndex( f,b,v ) GetOptionIndexEx( OPTION_ROOT_VALUE, f, b, v, FALSE DBG_SRC )

INDEX SetOptionValueEx( PODBC odbc, INDEX optval, INDEX iValue );
INDEX SetOptionValue( INDEX optval, INDEX iValue );

INDEX DuplicateValue( INDEX iOriginalValue, INDEX iNewValue );
INDEX NewDuplicateValue( PODBC odbc, INDEX iOriginalValue, INDEX iNewValue );
void InitMachine( void );

POPTION_TREE GetOptionTreeEx( PODBC odbc );



POPTION_TREE GetOptionTreeEx( PODBC odbc );
PFAMILYTREE* GetOptionTree( PODBC odbc );


INDEX NewGetOptionIndexExx( PODBC odbc, INDEX parent, const char *file, const char *pBranch, const char *pValue, int bCreate DBG_PASS );
_32 NewGetOptionStringValue( PODBC odbc, INDEX optval, char *buffer, _32 len DBG_PASS );
INDEX NewCreateValue( PODBC odbc, INDEX value, CTEXTSTR pValue );

void NewEnumOptions( PODBC odbc, INDEX parent
					 , int (CPROC *Process)(PTRSZVAL psv, CTEXTSTR name, _32 ID, int flags )
												  , PTRSZVAL psvUser );

void NewDeleteOption( PODBC odbc, INDEX iRoot );

