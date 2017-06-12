// mftpserve.c
// Created by Spencer Kitchen on 4/10/16.
//
// SERVER

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "mftp.h"

#define MY_PORT_NUMBER 49999

// FUNCTION
void serv_Read( int fd, char * output );

//==================================================================
int main(int argc, const char * argv[]) {
    pid_t pid;
    //-make socket----------------------------------------------
    int listenfd;
    struct sockaddr_in servAddr;
    /*
     *  AF_INET is the domain -> Internet
     *  SOCK_STREAM is the protocol family (TCP)
     *  If the result is < 0, there is an error, use perror() to print the message
     */
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "ERROR: Error creating listening socket().\n");
        exit(1);
    }
    //-Bind socket to a port------------------------------------
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(MY_PORT_NUMBER);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ( bind( listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
        // doesnt work
        perror("bind");
        exit(1);
    }
    int connectfd;
    unsigned int length = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
    int numConnect = 0;
    //-listen for connection------------------------------------
    if(listen( listenfd, 4) < 0){
        fprintf(stderr, "Error calling listen()\n");
        exit(1);
    }
    while (1) {
        connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        //-accept connection, Sets up a connection queue 4 level deep-----
        if (connectfd < 0) {
            fprintf(stderr, "ERROR: Error calling accept()\n");
            exit(1);
        }
        numConnect = numConnect + 1;    // client i.d.
        // Fork ---------------------------------
        pid = fork();
        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }
        // child ----------------------------------------------------------
        if (pid == 0) {
            close(listenfd);
            //-Getting a text hostname------------
            struct hostent* hostEntry;
            char* hostName;
            hostEntry = gethostbyaddr(&(clientAddr.sin_addr), sizeof(struct in_addr), AF_INET);
            hostName = hostEntry->h_name;
            if (hostName == NULL) {
                fprintf(stderr, "ERROR: Error calling gethostbyaddr()\n");
                exit(1);
            }
            else {
            	printf("\n** Client [%d] Connected **\n", numConnect);
            	printf("ClientName: %s\n",hostName);
            }
            char  *buffer= calloc(512, sizeof(char));
            //-loop to record to stdout of server--------------------------
            while(1) {
                //-read from client-----------------------------------------
                serv_Read(connectfd, buffer);
                printf("\nFrom client [%d]: %s\n", numConnect, buffer);
        		//-execute commands here------------------------------------
                //-Q for quit <Q>------------------------------------
        		if (buffer[0] == 'Q') {
                    // accepted command response
                    my_write(connectfd, "A\n", 2);
                    close(connectfd);
                    printf("CLIENT [%d] DISCONNECTED\n", numConnect);
                    exit(0);
        		}
        		//-D for data <D>------------------------------------
        		else if (buffer[0] == 'D' ) {
        			int datafd;
        			struct sockaddr_in sockAddr;
        			if((datafd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        				fprintf(stderr, "ERROR: Error creating listening socket().\n");
        				exit(1);
    				}
    				memset(&sockAddr, 0, sizeof(sockAddr));
    				sockAddr.sin_family = AF_INET;
    				sockAddr.sin_port = htons(0);
    				sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    				if ( bind( datafd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) < 0) {
        				// doesnt work
        				perror("bind");
        				exit(1);
    				}
    				struct sockaddr_in dataAddr;
    				socklen_t len = sizeof(dataAddr);
    				getsockname(datafd,(struct sockaddr *) &dataAddr, &len);
    				if(listen( datafd, 1) < 0){
        				fprintf(stderr, "Error calling listen()\n");
        				exit(1);
    				}
    				printf("opening data connection on: %d\n", ntohs(dataAddr.sin_port));
    				//converting port num to int
    				int port = ntohs(dataAddr.sin_port);

    				//storing portnum int in a string
    				char portnum[10];
                    snprintf(portnum, 10, "%d", port);
                    int size = strlen(portnum);
                    char *send;
                    send = calloc(size+2, sizeof(char )); //length of portnumber + \n + command
    				// add in A<port#>
                    send[0]='A';
                    memcpy(&send[1], portnum, size);
                    send[size+1] = '\n'; // add new line
    				my_write(connectfd, send, size + 2);  
    				int clientfd;
    				clientfd = accept(datafd, (struct sockaddr *) &dataAddr, &length);
                    //-sub Data call ------------------------------------------
                    //-RLS <RLS> <L>-----------------------------------------------
                    serv_Read(connectfd, buffer);
                    buffer[strcspn(buffer, "\n")] = '\0';
                    if (buffer[0]=='L'){
                        printf("From client [%d]: %s\n", numConnect, buffer);
                        pid_t ls;
                        if((ls = fork()) == 0) {
                            //child
                            close(1);
                            dup2(clientfd, STDOUT_FILENO);
                            close(clientfd);
                            char *lsArg[] = { "ls","-l",0 };
                            execvp(lsArg[0],lsArg);
                            perror("execvp of ls failed");
                            exit(1);
                        } else {
                            //parent
                            close(datafd);
                            close(clientfd);
                            my_write(connectfd, "A\n", 2);
                            wait(0);
                        }
                    }
                    //-GET <GET> <G>-------------------------------------------------
                    else if (buffer[0]=='G'){
                        printf("From client [%d]: %s\n", numConnect, buffer);
                        int a = access(&buffer[1], R_OK);
                        if (a < 0) {
                            printf("FILE does not EXISTS\n");
                            my_write(connectfd, "Efile does not exist\n", strlen("Efile does not exist\n"));
                        } else {
                            int file = open(&buffer[1], O_RDONLY, S_IRUSR); //readable by user
                            if (file < 0){
                                fprintf(stderr, "fprintf: %s\n", strerror(errno) );
                                my_write(connectfd, strerror(errno), strlen(strerror(errno)));
                            } else {
                                char *get_file = NULL;
                                int  fSize = 0;
                                int  amount = 0;
                                char buff[512];
                                while((amount = my_read(file, buff, 512)) != 0) {
                                    get_file = realloc(get_file, fSize+amount);
                                    memcpy(&get_file[fSize], buff, amount);
                                    fSize = fSize + amount;
                                }
                                close(file);
                                get_file[strcspn(get_file, "\0")] = '\n';
                                my_write(connectfd, "A\n", 2);
                                my_write(clientfd, get_file, fSize);
                            }
                        }
                        close(clientfd);
                    }
                    //-PUT <PUT> <P>----------------------------------------------------
                    else if (buffer[0] == 'P'){
                        printf("From client [%d]: %s\n", numConnect, buffer);
                        if((access(&buffer[1], F_OK)) != 0) {
                            printf("FILE DOES NOT EXISTS\n");
                            char *put_file = NULL;
                            int  fSize = 0;
                            int bSize = 0;
                            char buff[512];
                            my_write(connectfd, "A\n", 2);
                            while((bSize = my_read(clientfd, buff, 512)) != 0) {
                                put_file = realloc(put_file, fSize+bSize);
                                memcpy(&put_file[fSize], buff, bSize);
                                fSize = fSize + bSize;
                            }
                            int file = open(&buffer[1], O_CREAT | O_WRONLY, 0x777);
                            if (file < 0){
                                fprintf(stderr, "fprintf: %s\n", strerror(errno) );
                                my_write(connectfd, "Ecan not create file\n", strlen("Ecan not create file\n"));
                            } else {
                                write(file,put_file, fSize);
                                close(file);
                            }
                        }else {
                            fprintf(stderr, "FILE DOES EXIST\n");
                            my_write(connectfd, "Efile exist\n", strlen("Efile exist\n"));
                        }
                        close(clientfd); // data connection
                    }
                    else {
                        printf("sumthin wrong: %s\n", buffer);
                    }
        		}
        		//-C for remote cd <RCD> <C>-------------------------------------------
        		else if (buffer[0] == 'C') {
                    buffer[strcspn(buffer, "\n")] = '\0';
            		if (chdir(&buffer[1]) != 0){
                    	fprintf(stderr, "failed to open provided directory:%s\n%s\n", &buffer[1], strerror(errno));
                    	int s = strlen(strerror(errno));
                        char *temp = calloc(2+s, sizeof(char));
                        temp[0] = 'E';
                        memcpy(&temp[1], strerror(errno),s);
                        temp[s+1] = '\n';
                        my_write(connectfd, temp, s+2 );
                        free(temp);
                	} else {
                		my_write(connectfd, "A\n", 2);
                	}
        		}
                //-error, input not reconized--------------------
                else {
                    my_write(connectfd, "E", sizeof("E\n"));
                }
            }
        }
        //parent ------------------------------
        else {
            close(connectfd);
        }
    }
    return 0;
}
//==========================
void serv_Read( int fd, char * output ){
    int buff_size =0;
    int amount =0;
    bool run = true;
    char buffer[512];
    char *tmp = NULL;
    while(run) {
        amount = read(fd, buffer, 512);
        if (amount < 0){
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }
        if (amount == 0 ){
            run = false;
        }else {
            tmp = realloc(output, buff_size + amount );
            if (tmp == NULL){
                fprintf(stderr, "%s\n", strerror(errno));
                exit(1);
            }
            output = tmp;
            memcpy(&output[buff_size], buffer, amount);
            buff_size = buff_size + amount;
        }
        if (buffer[amount-1] == '\n' ){
            run = false;
        }
    }
    if (output != NULL){
        output[strcspn(output, "\n")] = '\0';
    }
}

