#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <getopt.h>

#define BUF_SIZE 1024
#define Mbps(bytes, usec) ((bytes) * 8. / (usec))

int
main(int argc, char *argv[])
{
	int opt;
	int sockfd;
	char buf[BUF_SIZE];
	struct sockaddr_in server_addr;
	unsigned int rate = 0;
	
	memset(buf, 'a', BUF_SIZE);

	server_addr.sin_family = AF_INET;
	
	while((opt = getopt(argc, argv, "h:p:r:")) != -1) {
		switch (opt) {
		case 'h':
			server_addr.sin_addr.s_addr = inet_addr(optarg);
			break;
		case 'p':
			server_addr.sin_port = htons((short)atoi(optarg));
			break;
		case 'r':
			rate = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Invalid arguments\n");
			fprintf(stderr, "Usage: %s [-h HOST] [-p PORT][-r Mbps RATE]\n", argv[0]);
			return EXIT_FAILURE;
		}
	}
	bzero(&(server_addr.sin_zero), 8);

	printf("Server IP address  : %s\n", inet_ntoa(server_addr.sin_addr));
	printf("Server port number : %hd\n", ntohs(server_addr.sin_port));
	printf("Target rate        : %u\n", rate);
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	struct timeval curr, prev;
	unsigned long long usec_diff;
	unsigned long long byte_cnt = 0;
	unsigned long long ret;
	
	gettimeofday(&prev, NULL);
	
	while (1) {
		gettimeofday(&curr, NULL);
		usec_diff = (curr.tv_sec * 1000000 + curr.tv_usec -
			     prev.tv_sec * 1000000 - prev.tv_usec);

		if (usec_diff == 0)
			continue;

		if (Mbps(byte_cnt, usec_diff) >= rate)
			continue;

		ret = sendto(sockfd, buf, BUF_SIZE, 0,
			     (struct sockaddr *)&server_addr,
			     sizeof(struct sockaddr));

		if (ret == -1)
			break;

		byte_cnt += ret;

		if (curr.tv_sec > prev.tv_sec) {
			double throughput = Mbps(byte_cnt, usec_diff);

			if (throughput >= 1000.) {
				printf("Throughput: %.3f Gbps\n", throughput / 1000.);
			}
			else {
				printf("Throughput: %.3f Mbps\n", throughput);
			}

			prev = curr;
			byte_cnt = 0;
		}
	}

	return EXIT_SUCCESS;
}
