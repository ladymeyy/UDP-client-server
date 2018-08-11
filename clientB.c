#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>

#define BUFFER_LEN_RX 1024*1024

static const char *command[] = {"CONNECT","DISCONNECT"};
static long long rx_rate = 0;
static int print = 1;

static void* receiver_thread(void *data){
	
	int sk = *(int*)data;
	int n_read = 0;
	long long n_total = 0;
	char buf[BUFFER_LEN_RX];
	unsigned long us_diff;
	struct timeval  tv1, tv2;

	gettimeofday(&tv1, NULL);

	while (1) {
		n_read = recvfrom(sk,buf,BUFFER_LEN_RX,0,NULL,NULL);
		if (n_read<0) {
			perror("Problem in recvfrom");
			exit(1);
		}

		gettimeofday(&tv2, NULL);

		n_total += n_read;

		us_diff = (tv2.tv_sec - tv1.tv_sec)*1000000 +  (tv2.tv_usec - tv1.tv_usec);
		if (us_diff >= 1000000) {
			rx_rate = (n_total * 8)/(us_diff/1000000);
			tv1 = tv2;
			
			if(print){
				printf("RX rate = %llu.%llu Mb/s ", rx_rate/(1024*1024), rx_rate%(1024*1024));
				printf("RX n_total bytes %llu\n", n_total);
			}
			
			n_total = 0;
		}
	}
	return NULL;
}

void main( int argc, char **argv ) {

	int sk,n_sent,rate,delta = 0;
	char st;
	unsigned long packets_per_sec,us_diff;
	struct timeval tv1, tv2;
	struct sockaddr_in server;
	struct hostent *hp;
	pthread_t thread_id;

	if (argc!=3) {
		printf("Usage: %s <server name> <port number>\n",argv[0]);
		exit(0);
	}

	if ((sk = socket( PF_INET, SOCK_DGRAM, 0 )) < 0){
		printf("Problem creating socket\n");
		exit(1);
	}

	server.sin_family = AF_INET;
	if ((hp = gethostbyname(argv[1]))==0) {
		printf("Invalid or unknown host\n");
		exit(1);
	}

	memcpy( &server.sin_addr.s_addr, hp->h_addr, hp->h_length);

	/* establish the server port number - we must use network byte order! */
	server.sin_port = htons(atoi(argv[2]));
	
	pthread_create(&thread_id, NULL, &receiver_thread, &sk);

	printf("Client typeB is waiting for command:\n");
	printf("------------------------------------\n");
	printf("'c' \t\t connect\n");
	printf("'d' \t\t disconnect\n");
	printf("'p' \t\t toggle bandwidth output\n");
	
	while (1){
		while ((st = getchar()) != EOF && st != '\n')
		switch(st){
			case 'c':
				n_sent = sendto(sk,command[0],strlen(command[0]),0, (struct sockaddr*) &server,sizeof(server));
				if (n_sent<0) {
					perror("Problem sending data");
					exit(1);
				}
				printf("Sent %s command\n",command[0]);
				break;
			case 'd':
				n_sent = sendto(sk,command[1],strlen(command[1]),0, (struct sockaddr*) &server,sizeof(server));
				if (n_sent<0) {
					perror("Problem sending data");
					exit(1);
				}
				printf("Sent %s command\n",command[1]);
				break;	
			case 'p':
				print = !print;
				printf("Print bandwidth %s\n",print? "ON":"OFF");
				break;
			default:
				printf("Wrong usage: insert 'c', 'd' or 'p'...\n");
		}	
	}
}
