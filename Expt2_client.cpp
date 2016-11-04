#include <iostream>
#include <cstring>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
	
using namespace std;

int main()
	{
	
	int status;
	fd_set client_fdset;
	
	int cfd = 0, maxfd=0;
	int count = 0;

	int sockfd, portno, n;
    	struct sockaddr_in serv_addr;
    	struct hostent *server;
    	struct in_addr ipv4addr;

    	portno = 5555;

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) 
        	printf("Can not open socket!\n");

    	server = gethostbyname("localhost");
    	if (server == NULL) {
        	printf("Invalid host!\n");
        	exit(0);
    	}

    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    	serv_addr.sin_port = htons(portno);

    	if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    	{
        	printf("Could not connect!\n");
    	}
	
	timeval wt;
	wt.tv_sec = 1;
	wt.tv_usec = 0;
	select(NULL,NULL,NULL,NULL,&wt);
	
	string wmsg = "\n";
	write(sockfd,wmsg.data(),wmsg.size());
	
	while(true && (count<100))
	{
		FD_ZERO(&client_fdset);
		FD_SET(sockfd,&client_fdset);
	
		count++;
		cout << count << endl;
		int tosend = htonl(count);

		write(sockfd,&tosend,sizeof(tosend));
		sync();
		select(NULL,NULL,NULL,NULL,&wt);

		pselect(sockfd+1,&client_fdset,NULL,NULL,NULL,NULL);

		if(FD_ISSET(sockfd,&client_fdset))
		{
			char buff[8];
			int readlen = read(sockfd,buff,8);
			if(readlen==0)
			{
				cout << "Server disconnected!\n";
				close(sockfd);
				exit(EXIT_SUCCESS);
			}
			buff[7] = '\0';
			std::cout << "Server: " << buff << endl;
		}
	}
	
	return 0 ;
}
