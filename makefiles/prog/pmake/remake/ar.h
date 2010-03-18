

typedef long (*arproc)(int desc , char *mem, int truncated,
		  long int hdrpos , long int datapos ,
                  long int size , long int date,
                  int uid , int gid , int mode , char *name  );
