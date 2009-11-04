%include "c32.mac"

%ifdef BCC32
section _TEXT USE32
%else
section .text 
%endif


;enum config_types {
;     CONFIG_UNKNOWN
%define CONFIG_UNKNOWN 0
;   // must match case-insensative exact length.
;   // literal text
;   , CONFIG_TEXT
%define CONFIG_TEXT 1
;   // a yes/no field may be 0/1, y[es]/n[o], on/off
;   //
;   , CONFIG_YESNO
;   , CONFIG_TRUEFALSE = CONFIG_YESNO
;   , CONFIG_ONOFF = CONFIG_YESNO
;   , CONFIG_OPENCLOSE = CONFIG_YESNO
;   , CONFIG_BOOLEAN = CONFIG_YESNO
%define CONFIG_BOOLEAN 2
;
;   // may not have a . point - therefore the . is a terminator and needs
;   // to match the next word.
;    , CONFIG_INTEGER
%define CONFIG_INTEGER 3
;    // has to be a floating point number (perhaps integral ie. no decimal)
;    , CONFIG_FLOAT
%define CONFIG_FLOAT 4
;
;    // formated number [+/-][[ ]## ]##[/##]
;    , CONFIG_FRACTION
%define CONFIG_FRACTION 5
;
;   // matches any single word.
;    , CONFIG_SINGLE_WORD
%define CONFIG_SINGLE_WORD 6
;
;   // protocol://[user[:password]](ip/name)[:port][/filepath][?cgi]
;   // by convention this will not contain spaces... but perhaps
;   // &20; (?)
;    , CONFIG_URL
%define CONFIG_URL 7
;
;   // matches several words in a row - the end word to match is supplied.
;    , CONFIG_MULTI_WORD
%define CONFIG_MULTI_WORD 8
;   // file name - does not have any path part.
;   // the following are all treated like multi_word since file names/paths
;   // may contain spaces
;    , CONFIG_FILE
%define CONFIG_FILE 9
;   // ends in a / or \,
;    , CONFIG_PATH
%define CONFIG_PATH 10
;   // may have path and filename
;    , CONFIG_FILEPATH
%define CONFIG_FILEPATH 11
;
;   // (IP/name)[:port]
;    , CONFIG_ADDRESS
%define CONFIG_ADDRESS 2
;
;    // end of configuration line (match assumed)
;    , CONFIG_PROCEDURE
%define CONFIG_PROCEDURE 13
%define CONFIG_COLOR     14
;};
;
;typedef struct config_element_tag
;{
%define CE_type_ofs   0
;    enum config_types type;
%define CE_next_ofs   CE_type_ofs + 4
%define CE_prior_ofs  CE_next_ofs + 4
%define CE_ppMe_ofs   CE_prior_ofs + 4
;    struct config_test_tag *next, *prior; // if a match is found, follow this to next.
%define CE_flags_ofs  CE_ppMe_ofs + 4
;    struct {
;        _32 vector : 1;
;        _32 multiword_terminator : 1; // prior == actual segment...
;    } flags;
%define CE_element_count_ofs CE_flags_ofs + 4
;    _32 element_count; // used with vector fields.
%define CE_data_ofs  CE_element_count_ofs + 4
;    union {
;        PTEXT pText;
;        struct {
;            LOGICAL bTrue : 1;
;        } truefalse;
;        _64 integer_number;
;        double float_number;
;        char *pWord; // also pFilename, pPath, pURL
;        // maybe pURL should be burst into
;        //   ( address, user, password, path, page, params )
;        SOCKADDR *psaSockaddr;
;        struct {
;            char *pWords;
;     // next thing to match...
;     // this is probably a constant text thing, but
;     // may be say an integer, filename, or some known
;     // format thing...
;            struct config_element_tag *pEnd;
;        } multiword;
;        FRACTION fraction;
;        PTRSZVAL (*Process)( PTRSZVAL, ... );
;        CDATA  color;
;    } data[1]; // these are value holders... if there is a vector field,
;            // either the count will be specified, or this will have to
;            // be auto expanded....
;} CONFIG_ELEMENT, *PCONFIG_ELEMENT;


;//---------------------------------------------------------------------
%ifdef BCC32
section _TEXT USE32
%else
section .data
%endif
switch_vector:
	dd procname(CallProcedure).ConfigUnknown
	dd procname(CallProcedure).ConfigTextPush
        dd procname(CallProcedure).ConfigBoolean
        dd procname(CallProcedure).ConfigInteger
        dd procname(CallProcedure).ConfigFloat
        dd procname(CallProcedure).ConfigFraction
        dd procname(CallProcedure).ConfigSingleWord
        dd 0 ;procname(CallProcedure).ConfigURL
        dd procname(CallProcedure).ConfigMultiWord
        dd procname(CallProcedure).ConfigFile
        dd procname(CallProcedure).ConfigPath
        dd procname(CallProcedure).ConfigFilePath
        dd 0 ;procname(CallProcedure).ConfigAddress
        dd 0 ;procname(CallProcedure).ConfigProcedure
        dd procname(CallProcedure).ConfigColor
%ifdef BCC32
section _TEXT USE32
%else
section .text
%endif
proc CallProcedure
;PTRSZVAL *ppsvUser
arg %$ppsvUser
;PCONFIG_ENTRY pce
arg %$pce

	sub esp, 8 ; temp variables ebp-4 == argsize, -8 == pce to push
        mov dword [ebp-4], 0
        mov eax, [ebp+%$pce]
        mov eax, [eax + CE_prior_ofs]
        mov [ebp-8], eax
        jmp .looptest
;     int argsize = 0;
;     PCONFIG_ELEMENT pcePush = pce->prior;
;     // push arguments in reverse order...
;     Log( WIDE("Calling process... ") );
;     while( pcePush )
;     {
.pusharg
;         Log( WIDE("To push...") );
;         LogElement( WIDE("pushing"), pcePush );
	mov eax, [ebp-8]
        mov ecx, [eax+CE_type_ofs]
        shl ecx, 2
        jmp [switch_vector + ecx]
;         switch( pcePush->type )
;         {
;         case CONFIG_TEXT:
;             break;
	.ConfigBoolean:
        	add dword [ebp-4], 4
                push dword [eax + CE_data_ofs]
                jmp .endswitch
;         case CONFIG_BOOLEAN:
;             {
;                 LOGICAL val = pcePush->data[0].truefalse.bTrue;
;                  argsize +=
;                     PushArgument( val );
;             }
;             break;
	.ConfigInteger:
        	add dword [ebp-4], 8
                push dword [eax + CE_data_ofs+4]
                push dword [eax + CE_data_ofs]
                jmp .endswitch
;         case CONFIG_INTEGER:
;             argsize +=
;                 PushArgument( pcePush->data[0].integer_number );
;             break;
	.ConfigFloat:
        	add dword [ebp-4], 8
                push dword [eax + CE_data_ofs+4]
                push dword [eax + CE_data_ofs]
                jmp .endswitch
;         case CONFIG_FLOAT:
;             argsize +=
;                 PushArgument( pcePush->data[0].float_number );
;             break;
	.ConfigFraction:
        	add dword [ebp-4], 8
                push dword [eax + CE_data_ofs+4]
                push dword [eax + CE_data_ofs]
                jmp .endswitch
;         case CONFIG_FRACTION:
;             argsize +=
;                 PushArgument( pcePush->data[0].fraction );
;             break;
	.ConfigSingleWord:
	.ConfigMultiWord:
	.ConfigColor:
        .ConfigFile:
        .ConfigPath:
        .ConfigFilePath:
        	add dword [ebp-4], 4
                push dword [eax + CE_data_ofs]
                jmp .endswitch
;         case CONFIG_SINGLE_WORD:
;             argsize +=
;                 PushArgument( pcePush->data[0].pWord );
;             break;
;         case CONFIG_MULTI_WORD:
;             argsize +=
;                 PushArgument( pcePush->data[0].multiword.pWords );
;             break;
;         }
        .ConfigTextPush:
        .ConfigUnknown:
        .endswitch
;         Log1( WIDE("Total args are now: %d"), argsize );
;         pcePush = pcePush->prior;
	mov eax, [eax+CE_prior_ofs]
        mov [ebp-8], eax
;     }
.looptest:
	cmp dword [ebp-8], 0
        jne .pusharg
        
        mov eax, [ebp + %$ppsvUser]
	push dword [eax]
	mov ecx, [ebp + %$pce]
	mov ecx, [ecx + CE_data_ofs]
        call ecx
        mov ecx, [ebp + %$ppsvUser]
        mov [ecx], eax
;     (*ppsvUser) = pce->data[0].Process( *ppsvUser );

	add esp, [ebp-4]
;     PopArguments( argsize );

endp



mproc PushArgument
        pop ecx     ; get return address to jump to
        mov eax, [esp]
        sub esp, [esp]     ; get argument size
	jmp ecx         ; goto return address


mproc PopArguments
	pop ecx
	mov eax, [esp]
	add esp, [esp] 
	jmp ecx


