#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <getopt.h>

#define BUF_SIZE 1048576

int
main(int argc, char *argv[])
{
	int sockfd;
	socklen_t len = sizeof(struct sockaddr);
	int opt;
	int my_port;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	char buf[BUF_SIZE];
	
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
		case 'p':
			my_port = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Invalid argument\n");
			fprintf(stderr, "Usage: %s [-p PORT]\n", argv[0]);
			return EXIT_FAILURE;
		}
	}	

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port   = htons(my_port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *)&my_addr,
		 sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return EXIT_FAILURE;
	}

	printf("Server port number: %d\n", my_port);

	while (1) {
		recvfrom(sockfd, buf, BUF_SIZE, 0,
			 (struct sockaddr *)&their_addr, &len);
	}
	
	return EXIT_SUCCESS;
}
