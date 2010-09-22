CFLAGS=-g
all: getVersion hello helloNS helloAttr helloMillion Guide.html

Guide.html:	guideIN.html *.c
	perl makeGuide.pl < guideIN.html > Guide.html

hello: hello.c ../libgenx.a
	cc $(CFLAGS) -o hello hello.c ../libgenx.a
helloNS: helloNS.c ../libgenx.a
	cc $(CFLAGS) -o helloNS helloNS.c ../libgenx.a
helloAttr: helloAttr.c ../libgenx.a
	cc $(CFLAGS) -o helloAttr helloAttr.c ../libgenx.a
helloMillion: helloMillion.c ../libgenx.a
	cc $(CFLAGS) -o helloMillion helloMillion.c ../libgenx.a
getVersion: getVersion.c ../libgenx.a
	cc $(CFLAGS) -o getVersion getVersion.c ../libgenx.a

clean:
	rm -f hello helloNS helloAttr helloMillion getVersion *.o *~
