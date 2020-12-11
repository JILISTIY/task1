#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define PAGE_SIZE 4096

#define SERVER_FIFO "server_fifo"

struct message {                
	pid_t pid;                  
	char filename[256];
};


int main(int argc, char *argv[]) {
	int serverFd, clientFd;
	char clientfifo[100];
	struct message msg;

	umask(0);                           
	if (mkfifo(SERVER_FIFO, 0666) == -1 && errno != EEXIST) {
		perror("mkfifo");
		exit(1);
	}

   
	if ((serverFd = open(SERVER_FIFO, O_RDWR)) < 0) {
		perror("open fifo");
		return errno;
	}
	

	while(1) {                          
		if (read(serverFd, &msg, sizeof(struct message)) != sizeof(struct message)) {
			fprintf(stderr, "Error reading message\n");
			continue;                  
		}

		sprintf(clientfifo, "%ld", (long) msg.pid);
		int clientFd = open(clientfifo, O_WRONLY | O_NONBLOCK);
		if (clientFd == -1) {           
			perror("CLIENT");
			continue;
		}
       

		FILE* flin = fopen(msg.filename, "rb");
		if (flin == NULL) {           
			perror("CLIENT");
			continue;
		}

		char buf[PAGE_SIZE] = "";
		int reallength = 0;

		while((reallength = fread(buf + 1, sizeof(char),  PAGE_SIZE - 1, flin)) == PAGE_SIZE - 1 ){
			buf[0] = 0;
			if(write(clientFd, buf, PAGE_SIZE) == -1){
				perror("CLIENT");
				close(clientFd); 
				break;
			}
		}
        
		buf[0] = 1;
		write(clientFd, buf, reallength + 1);
  
		close(clientFd);           
	}
}
