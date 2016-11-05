#include "setupserver.h"

using namespace std;

int setUpServer(int *serversocket,int *clientsocket)
{
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    fd_set client_fdset;
    int cfd = 0;

    memset(&host_info, 0, sizeof host_info);

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    getaddrinfo("localhost", "5555", &host_info, &host_info_list);

    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);

    int yes = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);

    cout << "Listning for clients on port 5555...\n";
    listen(socketfd, 1);

    while(true)
    {
        int act=0;
        sockaddr their_addr;
        socklen_t addr_size = sizeof(their_addr);

        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);

        act = pselect(socketfd+1,&client_fdset,NULL,NULL,NULL,NULL);

        if(FD_ISSET(socketfd,&client_fdset))
        {
            cfd = accept(socketfd,&their_addr,&addr_size);
            break;
        }

    }

    *serversocket = socketfd;
    *clientsocket = cfd;

    return 0;
}
