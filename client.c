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



char clientfifo[100];


int main(int argc, char *argv[]) {
	struct message msg;
	int clientFd;
	int serverFd;
	
	if (argc != 2 ) {
		printf("invalid argc\n");
		exit(1);
	}
                

	umask(0);                   
	sprintf(clientfifo, "%ld", (long) getpid());
	if (mkfifo(clientfifo, 0666) == -1  && errno != EEXIST){
		perror("mkfifo");
		exit(1);
	}


	msg.pid = getpid();
	strcpy(msg.filename, argv[1]);

	if ((serverFd = open(SERVER_FIFO, O_WRONLY | O_NONBLOCK)) < 0) {
		perror("open fifo");
		return errno;
	}
  
	if ((clientFd = open(clientfifo, O_RDONLY | O_NONBLOCK)) < 0) {
		perror("open fifo");
		return errno;
	}

	write(serverFd, &msg, sizeof(struct message));

	char buf[PAGE_SIZE] = "";
	int indicator = 0;
	int reallength = PIPE_BUF;


	while(reallength == PAGE_SIZE && !buf[0] ) {   
		int i = 0;
		while(i < 5) {
			ioctl(clientFd, FIONREAD, &indicator);  
			if (indicator) {
				break;
			}
			i++;
			sleep(1);
		}

		if (i == 5) {     
			printf("server failed or going too slow\n");
			exit(1);
		}
		reallength = read(clientFd, buf,  PIPE_BUF); 
		write(1, buf + 1 , reallength - 1); 

	}

	close(clientFd);
	close(serverFd);     
}
