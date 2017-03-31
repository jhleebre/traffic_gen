#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

//#define BUF_SIZE 1048576
#define BUF_SIZE 1024
#define Mbps(bytes, usec) ((bytes) * 8 / (usec))

int end_flag = 0;

void
signal_handler(int signum)
{
	end_flag++;
}

int
main(int argc, char **argv)
{
	int opt;
	int sockfd;
	struct sockaddr_in saddr;
	unsigned int rate = 0;
	char buf[BUF_SIZE];
	unsigned long long byte_cnt = 0;
	unsigned long long ret;
	struct timeval curr, prev;
	unsigned long long usec_diff;
	
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perror("signal");
		return EXIT_FAILURE;
	}

	if (argc != 7) {
		fprintf(stderr, "Usage: %s [-h HOST] [-p PORT] [-r Mbps RATE]\n", argv[0]);
		return EXIT_FAILURE;
	}

	memset(buf, 'a', BUF_SIZE);

	/* parse main arguments to set socket address and target rate*/
	saddr.sin_family = AF_INET;
	while ((opt = getopt(argc, argv, "h:p:r:")) != -1) {
		switch (opt) {
		case 'h':			/* server host address */
			saddr.sin_addr.s_addr = inet_addr(optarg); 
			break;
		case 'p':			/* server port number */
			saddr.sin_port = htons((short)atoi(optarg));
			break;
		case 'r':			/* target rate in Mbps */
			rate = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Invalid argument: %c\n", opt);
			fprintf(stderr, "Usage: %s [-h HOST] [-p PORT] [-r Mbps RATE]\n", argv[0]);
			return EXIT_FAILURE;
		}
	}
	bzero(&(saddr.sin_zero), 8);

	/* check information */
	printf("Server IP address  : %s\n", inet_ntoa(saddr.sin_addr));
	printf("Server port number : %hd\n", ntohs(saddr.sin_port));
	printf("Target rate        : %u\n", rate);

	/* create socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	/* connect to server */
	if (connect(sockfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr))
	    == -1) {
		perror("connect");
		close(sockfd);
		return EXIT_FAILURE;
	}

	gettimeofday(&prev, NULL);

	/* send data to server */
	while (!end_flag) {
		gettimeofday(&curr, NULL);
		usec_diff = (curr.tv_sec * 1000000 + curr.tv_usec - 
			     prev.tv_sec * 1000000 - prev.tv_usec);

		if (usec_diff == 0)
			continue;

		if (Mbps(byte_cnt, usec_diff) >= rate)
			continue;

		ret = write(sockfd, buf, BUF_SIZE);
		if (ret <= 0)
			break;
		byte_cnt += ret;

		if (curr.tv_sec > prev.tv_sec) {
			double throughput = (double)byte_cnt * 8. / (double)usec_diff;
			if (throughput >= 1000.) {
				printf("Throughput: %.3f Gbps\n", throughput/1000.);
			}
			else {
				printf("Throughput: %.3f Mbps\n", throughput);
			}
			prev = curr;
			byte_cnt = 0;
		}
	}

	/* close the connection and finish */
	close(sockfd);

	return EXIT_SUCCESS;
}
