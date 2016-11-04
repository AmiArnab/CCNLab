#include <iostream>
#include <cstring>
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

int setUpClient(int *clientfd)
{
    int status;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof host_info);

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, "5555", &host_info, &host_info_list);
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);
    int yes = 1;
    status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = connect(socketfd,host_info_list->ai_addr,host_info_list->ai_addrlen);

    cout << "Connecting to server...";
    fsync(socketfd);

    *clientfd = socketfd;

    return 0;
}

int main()
{
    int socketfd = 0;

    fd_set client_fdset;

    cout << "------CLIENT-------\n";

    setUpClient(&socketfd);

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);

        pselect(socketfd+1,&client_fdset,NULL,NULL,NULL,NULL);

        if(FD_ISSET(socketfd,&client_fdset))
        {
            uint32_t pkt = 0;
            int readlen = recv(socketfd,&pkt,sizeof(uint32_t),0);

            if(readlen==0)
            {
                cout << "Server disconnected!\n";
                close(socketfd);
                exit(EXIT_SUCCESS);
            }
            cout << "Received: " << pkt << endl;
            if(rand() > 0.2*RAND_MAX)
            {
                send(socketfd,&pkt,sizeof(uint32_t),0);
                fsync(socketfd);
                cout << "Sending: " << pkt << endl;
            }
            else
            {
                fsync(socketfd);
            }
        }
    }

    return 0 ;
}
