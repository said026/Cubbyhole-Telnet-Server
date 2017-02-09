echoserv: echoserv.o helper.o
	gcc -o echoserv echoserv.o helper.o -Wall -lpthread -std=gnu99

echoserv.o: echoserv.c helper.h
	gcc -o echoserv.o echoserv.c -c -ansi -pedantic -Wall -lpthread -std=gnu99

helper.o: helper.c helper.h
	gcc -o helper.o helper.c -c -ansi -pedantic -Wall









