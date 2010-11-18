
typedef struct handle_info_tag
{
	//struct mydatapath_tag *pdp;
   PTEXT pLine; // partial inputs...
   char *name;
	int       bNextNew;
   PTHREAD   hThread;
#ifdef WIN32
   HANDLE    handle;   // read/write handle
#else
   int       pair[2];
   int       handle;   // read/write handle
#endif
} HANDLEINFO, *PHANDLEINFO;


//typedef void (CPROC*TaskEnd)(PTRSZVAL, struct task_info_tag *task_ended);
struct task_info_tag {
	struct {
		BIT_FIELD closed : 1;
	} flags;
	TaskEnd EndNotice;
	TaskOutput OutputEvent;
	PTRSZVAL psvEnd;
	HANDLEINFO hStdIn;
	HANDLEINFO hStdOut;
	//HANDLEINFO hStdErr;
#if defined(WIN32)

	HANDLE hReadOut, hWriteOut;
	//HANDLE hReadErr, hWriteErr;
	HANDLE hReadIn, hWriteIn;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
   _32 exitcode;
#elif defined( __LINUX__ )
   int hReadOut, hWriteOut;
   //HANDLE hReadErr, hWriteErr;
	int hReadIn, hWriteIn;
   pid_t pid;
   _32 exitcode;
#endif
};
