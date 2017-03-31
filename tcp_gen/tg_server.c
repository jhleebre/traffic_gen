#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define BUF_SIZE 1048576

int listenfd;

void
signal_handler(int signum)
{
  /* close the listen socket and finish */
  close(listenfd);
  exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
  int opt;
  int sockfd;
  socklen_t sin_size;
  struct sockaddr_in saddr;
  struct sockaddr_in addr;
  char buf[BUF_SIZE];
  unsigned long long ret;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s [-p PORT]\n", argv[0]);
    return EXIT_FAILURE;
  }

  /* parse main arguments to set socket address */
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
    case 'p':
      saddr.sin_port = htons((short)atoi(optarg));
      break;
    default:
      fprintf(stderr, "Invalid argument: %c\n", opt);
      fprintf(stderr, "Usage: %s [-p PORT]\n", argv[0]);
      return EXIT_FAILURE;
    }
  }
  bzero(&(saddr.sin_zero), 8);

  /* check infomation */
  printf("Server port number : %hd\n", ntohs(saddr.sin_port));

  /* create a socket for listening */
  if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("socket");
    return EXIT_FAILURE;
  }

  /* bind on the port */
  if (bind(listenfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) < 0) {
    perror("bind");
    close(listenfd);
    return EXIT_FAILURE;
  }

  /* listen on the port */
  if (listen(listenfd, 1024) < 0) {
    perror("listen");
    close(listenfd);
    return EXIT_FAILURE;
  }

  while (1) {
    sin_size = sizeof(struct sockaddr);
    if ((sockfd = accept(listenfd, (struct sockaddr *)&addr, &sin_size)) < 0) {
      perror("accept");
      continue;
    }

    printf("New connection from %s\n", inet_ntoa(addr.sin_addr));

    if (!fork()) {		/* child process to handle a client */
      /* receive data from client */
      while (1) {
	ret = read(sockfd, buf, BUF_SIZE);
	if (ret <= 0)
	  break;
      }

      printf("End of connection with %s\n", inet_ntoa(addr.sin_addr));

      close(sockfd);
      exit(EXIT_SUCCESS);
    }

    close(sockfd);		/* parent does not need it */
    
    while (waitpid(-1, NULL, WNOHANG) > 0); /* clean up child processes */
  }

  return EXIT_SUCCESS;
}
