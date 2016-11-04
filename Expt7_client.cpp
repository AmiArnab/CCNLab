/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <pthread.h>
// #include <time.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 10 // max number of bytes we can get at once 

#define WAIT_SECS 5

#define P_LOSS 0.2

// #define WINDOW_SIZE 5

#define TOTAL_PACKETS_TO_SEND 20

int sockfd;

// int basepktnum = 1;		//base of sliding window
// int ack[WINDOW_SIZE];
// int ackarrind = 0;

int window_size = 2000;
int ack_rcvd = 0;


pthread_t sender_tid, receiver_tid;

void* sender_thread(void* varg)
{
	// int packet_no = (int) varg;	//will send and track this packet
	char message[MAXDATASIZE];
	int itr, npkts, rcvdpkts;

	sprintf( message, "packet");
	// send packets (window_size in number)
	// wait for some time (RTT) (allow to receive acks)
	// change window_size
	// loop

	while(1){
		
		npkts = window_size;
		printf("\n sending %4d packets\n", npkts);
		ack_rcvd = 0; //reset ack counter

		for(itr = 0; itr < npkts; ++itr){
			if (send(sockfd, message, strlen(message), 0) == -1)
				perror("send");
			//sleep(0.3);
			usleep(500);	//microseconds
		// printf("sending %s\n", message);
		}
		
		// sleep(1);
		usleep(100); 	//microseconds

		rcvdpkts = ack_rcvd;
		printf("received %4d acks\n", rcvdpkts);

		if(rcvdpkts == npkts)
			window_size += 500;
		else window_size /= 2;

	}
	return NULL;
}


void* receiver_thread(void* varg)
{	
	int  numbytes;  
	char buf[MAXDATASIZE];
	int  ack_no, ind, itr, diff, prev_ackarrind;

	while(1){
		if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0){
			buf[numbytes] = '\0';
			++ack_rcvd;
			// printf("acks rcvd = %3d\n", ack_rcvd);
		}
	}
	return NULL;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	float prob;
	// char message[MAXDATASIZE];
	int itr;
	// int ind;
	
	srand(time(NULL));

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// struct timeval tv;
	// tv.tv_sec = WAIT_SECS;
	// tv.tv_usec = 0; 
	// if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
	// 				 (char *)&tv, sizeof tv)){ 
	// 	perror("setsockopt"); 
	// 	return 3; 
	// } 

	pthread_create(&receiver_tid, NULL, receiver_thread, NULL);
	pthread_create(&sender_tid, NULL, sender_thread, NULL);
	
	//pthread_join(receiver_tid, NULL);
	sleep(120);

	pthread_cancel(receiver_tid);
	pthread_cancel(sender_tid);
	
	close(sockfd);

	return 0;
}
