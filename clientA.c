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
#include <sys/time.h>

#define BUFFER_LEN 1470

void main( int argc, char **argv ) {
	
	int n_sent,rate,sk,total_bytes,delta = 0;
	unsigned long packets_per_sec,us_diff;
	long long tx_rate = 0;
	char buf[BUFFER_LEN];
	struct sockaddr_in server;
	struct hostent *hp;
	struct timeval  tv1, tv2;
	
	if (argc!=4){
		printf("Usage: %s <server name> <port number> <rate (Mb/s)>\n",argv[0]);
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
	
	/*in order to get the actual bandwidth value need to know the UDP packet size*/
	rate  = atoi (argv[3]);
	packets_per_sec = ((rate * 1024 * 1024)/ (BUFFER_LEN * 8)) + 1;
	printf("send rate %dMb/s, packets per sec %lu\n",rate,packets_per_sec);
	
	while (1) {
		total_bytes = 0;
		gettimeofday(&tv1, NULL);
		
		while(packets_per_sec) {
			n_sent = sendto(sk,buf,BUFFER_LEN,0, (struct sockaddr*) &server,sizeof(server));
			if (n_sent<0) {
				perror("Problem sending data");
				exit(1);
			}
			total_bytes += n_sent;
			packets_per_sec--;
			if (packets_per_sec && (packets_per_sec%40) == 0)
				usleep(3000);
		}

		packets_per_sec = ((rate * 1024 * 1024)/ (BUFFER_LEN * 8)) + 1;

		gettimeofday(&tv2, NULL);
		us_diff = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
		if ((us_diff/1000000) > 0) {
			tx_rate = (total_bytes * 8);
			continue;
		}
		
		usleep(1000000 - delta - (us_diff%1000000));
		gettimeofday(&tv2, NULL);
		us_diff = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
		printf("actual exec time %lu.%06lu sec,", us_diff/1000000, us_diff%1000000);
		delta = us_diff - 1000000;
		tx_rate = (total_bytes * 8);
		printf("TX rate = %llu.%llu Mb/s \n ", tx_rate/(1024*1024), tx_rate%(1024*1024));
		//printf("total bytes = %d\n", total_bytes);
	}
}
