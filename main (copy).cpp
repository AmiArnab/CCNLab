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

    int fdlist[5], maxfd=0;

    for(int i=0;i<5;++i)
    {
        fdlist[i] = -1;
    }

    memset(&host_info, 0, sizeof host_info);

    std::cout << "Setting up the structs..."  << std::endl;

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, "5555", &host_info, &host_info_list);
    if (status != 0)  std::cout << "getaddrinfo error" << gai_strerror(status) ;

    std::cout << "Creating a socket..."  << std::endl;
    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);
    if (socketfd == -1)  std::cout << "socket error " ;

    std::cout << "Binding socket..."  << std::endl;
    int yes = 1;
    status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1)  std::cout << "bind error" << std::endl ;

    std::cout << "Listen()ing for connections..."  << std::endl;
    status =  listen(socketfd, 5);
    if (status == -1)  std::cout << "listen error" << std::endl ;


    while(true)
    {
        int act = 0;
        sockaddr their_addr;
        socklen_t addr_size = sizeof(their_addr);
        char *msg = "Hello client!\n";

        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);
        FD_SET(STDIN_FILENO,&client_fdset);
        maxfd = socketfd;

        for(int i=0;i<5;++i)
        {
            int sd = fdlist[i];
            if(sd >=0)
            {
                FD_SET(sd,&client_fdset);
            }
            if(sd>maxfd)
            {
                maxfd = sd;
            }
        }

        act = pselect(maxfd+1,&client_fdset,NULL,NULL,NULL,NULL);

        if(FD_ISSET(socketfd,&client_fdset))
        {
            int newsockfd = accept(socketfd,&their_addr,&addr_size);
            send(newsockfd,msg,strlen(msg),0);
            for(int i=0;i<5;++i)
            {
                if(fdlist[i] == -1)
                {
                    cout << newsockfd <<" arrived!\n";
                    fdlist[i] = newsockfd;
                    break;
                }
            }
        }

        if(FD_ISSET(STDIN_FILENO,&client_fdset))
        {
            //char str[10];
            string msg;
            cin >> msg;
            write(fdlist[0],msg.data(),msg.length());
        }

        for(int i=0;i<5;++i)
        {
            int sd = fdlist[i];
            char buff[1024];
            if(FD_ISSET(sd,&client_fdset))
            {
                int readlen = read(sd,buff,1023);
                buff[readlen] = '\0';
                std::cout << sd << " : " << buff << std::endl;
            }
        }
    }

    /*int new_sd;
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    new_sd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_sd == -1)
    {
        std::cout << "listen error" << std::endl ;
    }
    else
    {
        std::cout << "Connection accepted. Using new socketfd : "  <<  new_sd << std::endl;
    }

    std::cout << "Waiting to recieve data..."  << std::endl;
    ssize_t bytes_recieved;
    char incomming_data_buffer[1000];
    bytes_recieved = recv(new_sd, incomming_data_buffer,1000, 0);
    // If no data arrives, the program will just wait here until some data arrives.
    if (bytes_recieved == 0) std::cout << "host shut down." << std::endl ;
    if (bytes_recieved == -1)std::cout << "recieve error!" << std::endl ;
    std::cout << bytes_recieved << " bytes recieved :" << std::endl ;
    incomming_data_buffer[bytes_recieved] = '\0';
    std::cout << incomming_data_buffer << std::endl;


    std::cout << "send()ing back a message..."  << std::endl;
    const char *msg = "thank you.";
    int len;
    ssize_t bytes_sent;
    len = strlen(msg);
    bytes_sent = send(new_sd, msg, len, 0);

    std::cout << "Stopping server..." << std::endl;*/
    freeaddrinfo(host_info_list);

    //close(new_sd);
    //close(socketfd);

    return 0 ;


}
