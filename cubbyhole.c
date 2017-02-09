/*

  Cubbyhole Telnet Server
  ==========
  Based on the work of Paul Griffiths, 1999
  http://www.paulgriffiths.net/program/c/echoserv.php

*/

#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <sys/shm.h>          /*  for shm                   */

#include "helper.h"           /*  our own helper functions  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <semaphore.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/*  Global constants  */

#define DEFAULT_PORT (1337)
#define MAX_LINE (1024)

static const char HELP_MSG[] = "!help: A cubby hole is a small hiding place where one can hide things.\n"
                                "!help: We now define the cubby hole protocol that allows users to store\n"
                                "!help: one line messages on a server.\n"
                                "!help: As the hole is really small, the server will only store one\n"
                                "!help: message at a time, but keeps and shares it across different\n"
                                "!help: connections. If a new message is put in the cubby hole, the\n"
                                "!help: old message is lost.\n"
                                "!help:\n"
                                "!help: The following commands should be supported:\n"
                                "!help:\n"
                                "!help: PUT <msg> Places a new message in the cubby hole.\n"
                                "!help: GET       Takes the message out of the cubby hole and displays it.\n"
                                "!help: LOOK      Displays message without taking it out of the cubby hole.\n"
                                "!help: DROP      Takes the message out of the cubby hole without displaying it.\n"
                                "!help: HELP      Displays some help message.\n"
                                "!help: QUIT      Terminates the connection.\n"
                                "!help:\n"
                                "!help: Have Fun and play around!\n";

static const char WELCOME_MSG[] = "!hello: welcome brave adventurer\n";
static const char ERROR_MSG[] = "!error: unknown command - try HELP\n";
static const char GOODBYE_MSG[] = "!goodbye: see you next time\n";
static const char PUT_OK[] = "!PUT: ok\n";
static const char DROP_OK[] = "!DROP: ok\n";
static const char GET_OK[] = "!GET: ";
static const char LOOKUP_OK[] = "!LOOKUP: ";
static const char GET_EMPTY[] = "!GET: No data stored in the server\n";
static const char LOOKUP_EMPTY[] = "!LOOKUP: No data stored in the server\n";

int main(int argc, char *argv[]) {
    int       list_s;                /*  listening socket          */
    int       conn_s;                /*  connection socket         */
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr, clientaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *endptr;                /*  for strtol()              */
    int pid;                         /*  for fork()                */
    int c, client_port;
    char *client_ip;

    /*  Get port number from the command line, and
        set to default port if no arguments were supplied  */
    if ( argc == 2 ) {
	     port = strtol(argv[1], &endptr, 0);
  	  if ( *endptr ) {
  	     fprintf(stderr, "ECHOSERV: Invalid port number.\n");
         exit(EXIT_FAILURE);
  	  }
    } else if ( argc < 2 ) {
      port = DEFAULT_PORT;
    } else {
	     fprintf(stderr, "ECHOSERV: Invalid arguments.\n");
       exit(EXIT_FAILURE);
    }

    /*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
      fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
      exit(EXIT_FAILURE);
    }

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

    /* Somewhere in the kernel, maybe there's still some information about our
    previous socket hanging around. Tell the kernel that you are willing
    to re-use the port anyway*/

    int yes = 1;
    if (setsockopt(list_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      fprintf(stderr, "ECHOSERV: setsockopt()\n");
      exit(EXIT_FAILURE);
    }

    /*Bind our socket addresss to the
      listening socket, and call listen()  */
    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
      fprintf(stderr, "ECHOSERV: Error calling bind()\n");
      exit(EXIT_FAILURE);
    }

    if ( listen(list_s, LISTENQ) < 0 ) {
      fprintf(stderr, "ECHOSERV: Error calling listen()\n");
      exit(EXIT_FAILURE);
    }
    /* Place a semaphore in a shared memory */

    sem_t *sema = mmap(NULL, sizeof(*sema),
                       PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);

    if (sema == MAP_FAILED) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }

    /* Create/initialize the semaphore */
    if ( sem_init(sema, 1, 1) < 0) {
      perror("sem_init");
      exit(EXIT_FAILURE);
    }

    /*  Enter an infinite loop to respond
        to client requests and echo input  */
    fprintf(stdout, "Server started, listenting for clients on port %d...\n", port);
    while (1) {

	    /*  Wait for a connection, then accept() it  */

      if ((conn_s = accept(list_s, (struct sockaddr *)&clientaddr, (socklen_t*)&c)) < 0 ) {
          fprintf(stderr, "ECHOSERV: Error calling accept() %d\n", conn_s);
          exit(EXIT_FAILURE);
      }

      /* Print the IP of the new client */
      client_ip = inet_ntoa(clientaddr.sin_addr);
      client_port = (int) ntohs(clientaddr.sin_port);
      fprintf(stdout, "New client connected from %s:%d \n", client_ip, client_port);

      /* Create a child process */

      if ((pid = fork()) < 0) {
        fprintf(stderr, "ECHOSERV : Error on fork()\n");
        exit(EXIT_FAILURE);
      }

      /* This is the client process */
      if (pid == 0) {
        Writeline(conn_s, WELCOME_MSG, strlen(WELCOME_MSG));
        while (1) {

          /* Read the command from the client */
          Readline(conn_s, buffer, MAX_LINE-1);

          /* Unlock the semaphore*/
          if (sem_wait(sema) < 0) {
            fprintf(stderr, "ECHOSERV: Error calling sem_wait()\n");
          }

          /* Checking the nature of the command */
          if(strstr(buffer, "PUT") != NULL || strstr(buffer, "put") != NULL)  {
            FILE *fp;
            fp = fopen ("cubbyhole","ab+");
            if (fp == NULL) {
                fprintf(stderr, "ECHOSERV: Error calling fopen()\n");
                exit(EXIT_FAILURE);
            }
            char *toStore;

            /* write the message to the text file */
            toStore = strchr(buffer, ' ');
            if (strlen(toStore) == 0) {
              /* If nothing to store */
              break;
            }
            fprintf (fp, toStore);
            Writeline(conn_s, PUT_OK, strlen(PUT_OK));
            fclose (fp);
          } else if(strncasecmp(buffer, "GET", 3) == 0)  {
              /* Create a text file to store the message */
              FILE *fp;
              fp = fopen ("cubbyhole","ab+");
              if (fp == NULL) {
                  fprintf(stderr, "ECHOSERV: Error calling fopen()\n");
                  exit(EXIT_FAILURE);
              }
              /* get the content from the file */
              char fromFile[MAX_LINE + 1];
              if (fp != NULL) {
                  size_t newLen = fread(fromFile, sizeof(char), MAX_LINE, fp);
                  fseek (fp, 0, SEEK_END);
                  int size = ftell(fp);
                  if ((ferror(fp) != 0) && (size != 0)) {
                      fprintf(stderr, "ECHOSERV : Error reading file\n");
                  } else {
                      fromFile[newLen++] = '\0'; /* Just to be safe. */
                  }
              }

              /* Check if the content is empty*/
              if ((fromFile != NULL) && (fromFile[0] == '\0')) {
                Writeline(conn_s, GET_EMPTY, strlen(GET_EMPTY));
              } else {
                Writeline(conn_s, GET_OK, strlen(GET_OK));
                Writeline(conn_s, fromFile, strlen(fromFile));
              }

              /* clear the contents of the file*/
              fclose (fp);
              fp = fopen ("cubbyhole","w");
              fclose (fp);
            } else if(strncasecmp(buffer, "LOOK", 4) == 0)  {
                FILE *fp;
                fp = fopen ("cubbyhole","ab+");
                if (fp == NULL) {
                    fprintf(stderr, "ECHOSERV: Error calling fopen()\n");
                    exit(EXIT_FAILURE);
                }
                /* get the content from the file */
                char fromFile[MAX_LINE + 1];
                if (fp != NULL) {
                    size_t newLen = fread(fromFile, sizeof(char), MAX_LINE, fp);
                    fseek (fp, 0, SEEK_END);
                    int size = ftell(fp);
                    if ((ferror(fp) != 0) && (size != 0)) {
                        fprintf(stderr, "ECHOSERV : Error reading file\n");
                    } else {
                        fromFile[newLen++] = '\0'; /* Just to be safe. */
                    }
                }
                /* Check if the content is empty*/
                if ((fromFile != NULL) && (fromFile[0] == '\0')) {
                  Writeline(conn_s, LOOKUP_EMPTY, strlen(LOOKUP_EMPTY));
                } else {
                  Writeline(conn_s, LOOKUP_OK, strlen(LOOKUP_OK));
                  Writeline(conn_s, fromFile, strlen(fromFile));
                }

          } else if(strncasecmp(buffer, "DROP", 4) == 0)  {
              FILE *fp;
              fp = fopen ("cubbyhole","ab+");
              if (fp == NULL) {
                  fprintf(stderr, "ECHOSERV: Error calling fopen()\n");
                  exit(EXIT_FAILURE);
              }
              /* clear the contents of the file*/
              fclose (fp);
              fp = fopen ("cubbyhole","w");
              fclose (fp);
              Writeline(conn_s, DROP_OK, strlen(DROP_OK));

          } else if(strncasecmp(buffer, "HELP", 4) == 0)  {
              Writeline(conn_s, HELP_MSG, strlen(HELP_MSG));

          } else if(strncasecmp(buffer, "QUIT", 4) == 0) {
              Writeline(conn_s, GOODBYE_MSG, strlen(GOODBYE_MSG));

              /* Close the connected socket */
              if ( close(conn_s) < 0 ) {
                  fprintf(stderr, "ECHOSERV: Error calling close()\n");
                  exit(EXIT_FAILURE);
              }
              fprintf(stdout, "Client disconnected %s:%d \n", client_ip, client_port);
              exit(EXIT_SUCCESS);

          } else {
            /* If command not recognized */
              Writeline(conn_s, ERROR_MSG, strlen(ERROR_MSG));
          }
          /* Child unlocks semaphore */
          if (sem_post(sema) < 0) {
            fprintf(stderr, "ECHOSERV: sem_post()\n");
          }
        }
      }
    }

    /* Delete the semaphore */
    if (sem_destroy(sema) < 0) {
      fprintf(stderr, "ECHOSERV: sem_destroy failed\n");
      exit(EXIT_FAILURE);
    }

    if (munmap(sema, sizeof(sema)) < 0) {
      perror("munmap failed");
      fprintf(stderr, "ECHOSERV: munmap failed\n");
      exit(EXIT_FAILURE);
    }
}
