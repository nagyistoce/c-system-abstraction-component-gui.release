
#ifndef TEXT_LABEL_TYPES_DEFINED
#define TEXT_LABEL_TYPES_DEFINED

#include "intershell_export.h"

//typedef struct page_label *PPAGE_LABEL;

// result in substituted text from variables registered for InterShell
// if called from a context that does not have PPAGE_LABEL, pass NULL
//InterShell_PROC( CTEXTSTR, InterShell_GetLabelText )( PPAGE_LABEL label, CTEXTSTR variable );
// use of this is preferred, otherwise thread conflicts will destroy the buffer.
//InterShell_PROC( CTEXTSTR, InterShell_TranslateLabelText )( PPAGE_LABEL label, TEXTSTR output, int buffer_len, CTEXTSTR variable );

/* actual worker function for InterShell_GetLabelText - but suport dispatch to bProcControlEx*/
//InterShell_PROC( CTEXTSTR, InterShell_GetControlLabelText )( PMENU_BUTTON button, PPAGE_LABEL label, CTEXTSTR variable );


#endif
