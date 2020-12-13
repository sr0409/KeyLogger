.SUFFIXES: .c

all:
	g++ keylogger.c -o MyFunApp
	g++ keyloggerServer.c -o keyloggerServer

clean:
	/bin/rm -f *.o *~ *.dat core keylogger keyloggerServer