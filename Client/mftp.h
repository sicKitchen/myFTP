#ifndef MFTP_H
#define MFTP_H

int my_read(int fd, char *buff, int size);
void my_write(int fd, void *buff, int size);
//----------------------------------------------
int my_read(int fd, char *buff, int size) {
	int tmp;
	tmp = read(fd, buff, size);
	
    if (tmp < 0){
    	perror("ERROR reading from socket");
        exit(-1);
	} 
	buff[strcspn(buff, "\n")] = '\0';
	return tmp;
}

void my_write(int fd, void *buff, int size) {
	int tmp;
	tmp = write(fd, buff, size);
	if (tmp <= 0) {
		fprintf(stderr, "ERROR: Error calling write()\n");
        exit(1);
	}
}

#endif /* MFTP_H */