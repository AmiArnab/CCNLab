#include "setupclient.h"

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
