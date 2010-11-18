

PTRSZVAL CreateSecurityContext( PMENU_BUTTON button );
void CloseSecurityContext( PMENU_BUTTON button, PTRSZVAL psv_context_to_Destroy );

void CPROC EditSecurity( PTRSZVAL psv, PSI_CONTROL button );
void CPROC EditSecurityNoList( PTRSZVAL psv, PSI_CONTROL button );
void CPROC SelectEditSecurity( PTRSZVAL psv, PSI_CONTROL listbox, PLISTITEM pli );

