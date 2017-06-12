// mftp.c
// Created by Spencer Kitchen on 4/10/16.
//
// CLIENT

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include "mftp.h"

#define MY_PORT_NUMBER 49999
// Function
int control_connection(const char *host, int port);

//=========================================================================
int main(int argc, const char * argv[]) {
    // check argv[]
    if (argc != 2) {
        fprintf(stderr, "ERROR: Not enough arguments, enter IP address\n");
        exit(1);
    }
    int socketfd = control_connection(argv[1], MY_PORT_NUMBER);
    char *buffer = calloc(512, sizeof(char));
    char **command_path;
    command_path = calloc(2, sizeof(char *));
    printf("\n** Connection Established **\n");
    bool run = true;        // keep loop running
    while(run) {
        //-get message from client--------------------
        printf("MFTP> ");
        fgets(buffer,512,stdin);

        //-If client wants to exit <EXIT>---------------------
        if (strcmp(buffer, "exit\n") == 0) {
            my_write(socketfd, "Q\n", 2);
            //-get last message from server-----------
            my_read(socketfd, buffer, 512);
            printf("%s\n", buffer);
            free (command_path);
            exit(0);    // exit
        } 
        //-Local change directory <CD>------------------------- 
        else if (strncmp(buffer, "cd", 2) == 0) {
            command_path[0] = strtok(buffer," ");
            command_path[1] = strtok(NULL," ");
            if (command_path[1] != NULL) command_path[1][strcspn(command_path[1], "\n")] = '\0';

            if (command_path[1] == NULL){
                fprintf(stderr, "pathway not provided. cd <pathname>\n");
            } else {
                if (chdir(command_path[1]) != 0){
                    fprintf(stderr, "failed to open provided directory:%s\n%s\n", command_path[1], strerror(errno));
                }else {
                    char buf[512];
                    getcwd(buf, 512 );
                    printf("current dir: %s\n", buf );
                } 
            }
        } 
        //-Local ls-l <LS>-------------------------------------
        else if (strncmp(buffer, "ls", 2) == 0) {
            pid_t ls, more;
            int fd[2];
            int rdr;
            int wtr;
            assert(pipe(fd) >= 0);
            rdr = fd[0];
            wtr = fd[1];
            // fork twice for ls and more command
            //1st fork
            ls = fork();
            if (ls == 0){
                // ls code 
                close(rdr);
                close(1); dup2(wtr,STDOUT_FILENO); 
                close(wtr);
                char *lsArg[] = { "ls","-l",0 };
                execvp(lsArg[0],lsArg);
                perror("execvp of ls failed");
                exit(1);
            } else {
                close(wtr);
                more = fork();
                if (more == 0) {
                    // more code
                    close(0); dup2(rdr,STDIN_FILENO); close(rdr);

                    //change these based on wwhat 'more' you have
                    // -osx-
                    //char* moreArg[] = { "more","-n 20",0 };
                    // -unix-
                    char* moreArg[] = { "more","-20",0 };

                    execvp(moreArg[0], moreArg);
                    perror("execvp of more failed");
                    exit(1);
                } else {
                    // parent code 
                    close(rdr);
                    wait(0);
                    wait(0);
                }
            }
        }
        // remote cd <RCD>--------------------------------------
        else if (strncmp(buffer, "rcd", 3) == 0) {
            command_path[0] = strtok(buffer," ");
            command_path[1] = strtok(NULL," ");

            if (command_path[1] == NULL){
                fprintf(stderr, "pathway not provided. rcd <pathname>\n");
            } else {
                int size = (strlen(command_path[1]) + 1); // extra space for 'C'
                char *rmote_path;
                rmote_path = calloc(size, sizeof(char));
                rmote_path[0] = 'C';
                memcpy(&rmote_path[1], command_path[1], strlen(command_path[1]));
                my_write(socketfd, rmote_path, strlen(rmote_path));
                my_read(socketfd, buffer, 512);
                printf("%s\n", buffer); // prints A
                free(rmote_path);
            }
        }
        //-remote ls <RLS>--------------------------------------
        else if (strncmp(buffer, "rls", 3) == 0){
            my_write(socketfd, "D\n", strlen("D\n"));
            my_read(socketfd, buffer, 512);
            int port;
            char tmp;
            sscanf(buffer, "%c%d",&tmp, &port);
            int datafd = control_connection(argv[1], port);
            my_write(socketfd, "L\n", 2);
            my_read(socketfd, buffer, 512);
            printf("%s\n", buffer );   // accept a
            pid_t more;
            if((more = fork()) == 0) {
                //child
                close(0);
                dup2(datafd, STDIN_FILENO);
                close(datafd);

                // osx
                //char* moreArg[] = { "more","-n 20",0 };
                // unix
                char* moreArg[] = { "more","-20",0 };

                execvp(moreArg[0], moreArg);
                perror("more didnt execute\n");
                exit (1);
            } else {
                //parent
                close(datafd);
                wait(0);
            }
            close(datafd);
        }
        //- SHOW <SHOW>----------------------------------------
        else if (strncmp(buffer, "show", 4) == 0){
            char *fName;
            int fSize = strlen(&buffer[5]);
            fName = calloc(fSize+1, sizeof(char));
            fName[0] = 'G';
            memcpy(&fName[1], &buffer[5], fSize);
            fName[fSize+1] = '\n';
            my_write(socketfd, "D\n", 2);
            my_read(socketfd, buffer, 512);
            int port;
            char tmp;
            sscanf(buffer, "%c%d",&tmp, &port);
            int datafd = control_connection(argv[1], port);
			my_write(socketfd, fName, 1+fSize);
            my_read(socketfd, buffer, 512);
            printf("%s\n", buffer );  // accept A

            if (buffer[0] == 'E'){
            	fprintf(stderr, "Error:%s\n", &buffer[1] );
            }else {
             	pid_t more;
            	if((more = fork()) == 0) {
                	//child
                	close(0);
                	dup2(datafd, STDIN_FILENO);
                	close(datafd);

                	// osx
                	//char* moreArg[] = { "more","-n 20",0 };
                	// unix
                	char* moreArg[] = { "more","-20",0 };

                	execvp(moreArg[0], moreArg);
                	perror("more didnt execute\n");
                	exit (1);
            	} else {
                	//parent
                	close(datafd);
                	wait(0);
            	}
            }
        }
        //-GET <GET>------------------------------------------
        else if (strncmp(buffer, "get", 3) == 0){
            char *fName;
            int bSize = strlen(&buffer[4]);
            fName = calloc(bSize+1, sizeof(char));
            fName[0] = 'G';
            memcpy(&fName[1], &buffer[4], bSize);
            fName[bSize] = '\0';
            if((access(&fName[1], F_OK)) == 0) {
                printf("FILE EXISTS\n");
            } else {
                fName[bSize] = '\n';          // set back to new line
                my_write(socketfd, "D\n", 2);
                my_read(socketfd, buffer, 512);
                int port;
                char tmp;
                sscanf(buffer, "%c%d",&tmp, &port);
                int datafd = control_connection(argv[1], port);
                my_write(socketfd, fName, 1+bSize);
                my_read(socketfd, buffer, 512);
                fName[bSize] = '\0';   
                printf("%s\n", buffer );
                if (buffer[0] != 'E'){
                	char *get_file = NULL;
                	int  fSize = 0;
                	while((bSize = my_read(datafd, buffer, 512)) != 0) {
                    	get_file = realloc(get_file, fSize+bSize);
                    	memcpy(&get_file[fSize], buffer, bSize);
                    	fSize = fSize + bSize;
                	}
                	int file = open(&fName[1], O_CREAT | O_WRONLY, 0x777);
                	if (file < 0){
                    	fprintf(stderr, "fprintf: %s\n", strerror(errno) );
                	} else {
                    	write(file,get_file, fSize);
                    	close(file);
                	}
                }else {
                	fprintf(stderr, "Error:%s\n", &buffer[1] );
                }
                close(datafd);
            }
        }
        //-PUT <PUT>-------------------------------------------
        else if (strncmp(buffer, "put", 3) == 0){
            char *fName;
            int bSize = strlen(&buffer[4]);
            fName = calloc(bSize+1, sizeof(char));
            fName[0] = 'P';
            memcpy(&fName[1], &buffer[4], bSize);
            fName[bSize] = '\0';
            if((access(&fName[1], F_OK)) != 0) {
                printf("FILE does not EXISTS\n");
            } else { 
                my_write(socketfd, "D\n", 2);
                my_read(socketfd, buffer, 512);
                int port;
                char tmp;
                sscanf(buffer, "%c%d",&tmp, &port);
                int datafd = control_connection(argv[1], port);
                int file = open(&fName[1], O_RDONLY, S_IRUSR); //readable by user
                if (file < 0){
                    fprintf(stderr, "fprintf: %s\n", strerror(errno) );
                } else {
                    char *put_file = NULL;
                    int  fSize = 0;
                    int  amount = 0;
                    while((amount = my_read(file, buffer, 512)) != 0) {
                        put_file = realloc(put_file, fSize+amount);
                        memcpy(put_file, buffer, amount);
                        memcpy(&put_file[fSize], buffer, amount);
                        fSize = fSize + amount;
                    }
                    my_write(datafd, put_file, fSize);
                    fName[bSize] = '\n';          // set back to new line
                    my_write(socketfd, fName, 1+bSize);
                    my_read(socketfd, buffer, 512);
                    if (buffer[0] == 'E'){
                    	fprintf(stderr, "ERROR: %s\n", &buffer[1] );
                    }
                    printf("%s\n", buffer ); // accept A
                    close(file);
                }
                close(datafd);
            }
        }
        else {
            printf("unknown command: %s\n", buffer); 
        }
    }
}
//=====================================================

int control_connection(const char *host, int port) {
    //-Making a connection from a client----------
    int socketfd;
    socketfd = socket( AF_INET, SOCK_STREAM, 0);
    // make an Internet socket using TCP protocol
    
    //-Set up address of the server--------------
    struct sockaddr_in servAddr;
    struct hostent* hostEntry;
    struct in_addr **pptr;
    
    memset( &servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    
    hostEntry = gethostbyname(host);
    /* this is magic, unless you want to dig into the man pages */
    pptr = (struct in_addr **) hostEntry->h_addr_list;
    memcpy(&servAddr.sin_addr, *pptr, sizeof(struct in_addr));
   /*
    * Plug in the family and port number
    * Get the numeric form of the server address
    * Copy it into the address structure
    */
    
    //-Connect to server-------------------
    if (connect(socketfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        fprintf(stderr, "ERROR: Error calling connect()\n");
        exit(1);
    }
    return socketfd;
}

