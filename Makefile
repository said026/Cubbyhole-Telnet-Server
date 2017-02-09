cubbyhole: cubbyhole.o helper.o
	gcc -o cubbyhole cubbyhole.o helper.o -Wall -lpthread -std=gnu99

cubbyhole.o: cubbyhole.c helper.h
	gcc -o cubbyhole.o cubbyhole.c -c -ansi -pedantic -Wall -lpthread -std=gnu99

helper.o: helper.c helper.h
	gcc -o helper.o helper.c -c -ansi -pedantic -Wall









