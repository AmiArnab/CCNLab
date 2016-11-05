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

using namespace std;

int main()
{

    int status;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    fd_set client_fdset;

    int cfd = 0, maxfd=0;

    memset(&host_info, 0, sizeof host_info);

    std::cout << "Setting up the structures..."  << std::endl;

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo("localhost", "5555", &host_info, &host_info_list);
    if (status != 0)  std::cout << "getaddrinfo error" << gai_strerror(status) ;

    std::cout << "Creating socket..."  << std::endl;
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);
    if (socketfd == -1)  std::cout << "socket error " ;

    std::cout << "Setting socket options..."  << std::endl;
    int yes = 1;
    status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    std::cout << "Setting up client...\nConnecting to server through port 5555 ..."  << std::endl;
    status = connect(socketfd,host_info_list->ai_addr,host_info_list->ai_addrlen);
    if (status == -1)  {std::cout << "listen error" << std::endl; exit(EXIT_FAILURE);}

    timeval wt;
    wt.tv_sec = 1;
    wt.tv_usec = 0;
    select(NULL,NULL,NULL,NULL,&wt);

    string wmsg = "Hi Server!\n";
    write(socketfd,wmsg.data(),wmsg.size());

    cout << "Type \"quit\" to exit.\n";

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);
        FD_SET(STDIN_FILENO,&client_fdset);

        maxfd = (socketfd > STDIN_FILENO)?socketfd:STDIN_FILENO;

        pselect(maxfd+1,&client_fdset,NULL,NULL,NULL,NULL);

        if(FD_ISSET(STDIN_FILENO,&client_fdset))
        {
            string msg;
            getline(cin,msg);
            if(msg.compare("quit") == 0)
            {
                cout << "Disconnecting...\n";
                close(socketfd);
                freeaddrinfo(host_info_list);
                exit(EXIT_SUCCESS);
            }
            msg+='\n';
            write(socketfd,msg.data(),msg.size());
        }

        if(FD_ISSET(socketfd,&client_fdset))
        {
            char buff[1024];
            int readlen = read(socketfd,buff,1023);
            if(readlen==0)
            {
                cout << "Server disconnected!\n";
                close(socketfd);
                freeaddrinfo(host_info_list);
                exit(EXIT_SUCCESS);
            }
            buff[readlen] = '\0';
            std::cout << "Server: " << buff;
        }
    }

    return 0 ;
}
