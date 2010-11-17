
#define RELAY_VERSION "2.11" // 3 characters, string
	
// 2.11 - converted to using configscript lib, filemon lib.
// 2.10 - implemeted connection options
// 2.0 - implement multiple seperate outgoing directories
//     - disabled directory updates
//     - directory outgoing takes a file mask

// 1.8 - changed MESSAGE responce to controllers to include a length char
//     - Removed forced DoScan


#define NL_BUFFER      0

#define NL_ACCOUNT     1
#define NL_DATAMIRROR  1  // same info - different 'side'

#define NL_CONNECTION  2 // my connection information PCONNECT

//#define NL_SYSMIRROR   3 // account info PACCOUNT

//#define NL_MONITOR     4 // file monitor device to close on disconnect

#define NL_SYSUPDATE   5 // file monitor storage PMONITOR

#define NL_LASTMSG     6 // last message (per connection please)

#define NL_LASTMSGTIME 7
#define NL_PINGS_SENT  8

#define NL_VERSION     9

#define NUM_NETLONG    10

