all: client server

client: mftp.c mftp.h
	gcc -g -std=c99 -Wall -pedantic -o client mftp.c


server: mftpserve.c mftp.h
	gcc -g -std=c99 -Wall -pedantic -o server mftpserve.c

clean:
	rm -rf  server client *.dSYM
