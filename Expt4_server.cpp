#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;

typedef struct {
    int cfd;
    bool ackrcvd;
    int lastACK;
    int lastSent;
} ThreadArg;

int setUpServer(int *serversocket=NULL,int *clientsocket=NULL)
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

    cout << "Listning for clients...";
    listen(socketfd, 1);

    while(true)
    {
        sockaddr their_addr;
        socklen_t addr_size = sizeof(their_addr);

        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);

        pselect(socketfd+1,&client_fdset,NULL,NULL,NULL,NULL);

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

void sendPackets(int N,int fd,int *count)
{
    timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;

    cout << "\n--------------------\nSending "<< N <<" packets!\n";

    for(int i=0;i<N;++i)
    {
        uint32_t tosend = *count;
        cout << "Sending: " << tosend << endl;
        send(fd,&tosend,sizeof(tosend),0);
        fsync(fd);
        (*count)++;
        pselect(1,NULL,NULL,NULL,&timeout,NULL);
    }

    cout << "Sent " << N << " packets!\n---------------------\n";
}

void *sendData(void *myarg)
{
    ThreadArg *arg = (ThreadArg *)myarg;
    int fd = arg->cfd;

    const int N=5;
    int count = 1;

    while(count < 100)
    {
        if(arg->ackrcvd)
        {
            count = arg->lastACK + 1;
            sendPackets(N,fd,&count);
            arg->lastSent = count - 1;
            arg->ackrcvd = false;
        }
    }
}

void *recvACK(void *myarg)
{
    ThreadArg *arg = (ThreadArg *)myarg;
    int fd = arg->cfd;

    fd_set client_fdset;
    const int N=5;
    uint32_t pkt = 0, oldpkt = 0;

    timespec timeout;
    timeout.tv_sec = 1*N;
    timeout.tv_nsec = 0;

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(fd,&client_fdset);

        pselect(fd+1,&client_fdset,NULL,NULL,&timeout,NULL);

        if(FD_ISSET(fd,&client_fdset))
        {
            oldpkt = pkt;
            int readlen = recv(fd,&pkt,sizeof(uint32_t),0);

            cout << "Old packet: " << oldpkt << " New packet: " << pkt << endl;
            
            if(readlen==0)
            {
                cout << "Client disconnected!\n";
                close(fd);
                exit(EXIT_SUCCESS);
            }
            if((oldpkt+1) == pkt)
            {
                arg->lastACK = pkt;
            }
            else
            {
                pkt = oldpkt;
            }
        }
        else
        {
            cout << "-------------\nLast Sent: " << arg->lastSent << " Last ACK: " << arg->lastACK << "\n--------------\n";
            oldpkt = arg->lastACK;
            arg->ackrcvd = true;
        }
    }
}

int main()
{   
    ThreadArg arg;
    arg.ackrcvd = true;
    arg.lastACK = 0;
    arg.lastSent = 0;

    int srvrfd, cfd;
    pthread_t sendpkt,recvack;
    pthread_attr_t attr;

    cout << "------SERVER-------\n";

    setUpServer(&srvrfd,&cfd);
    arg.cfd = cfd;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    pthread_create(&sendpkt,&attr,sendData,(void*)(&arg));
    pthread_create(&recvack,&attr,recvACK,(void*)(&arg));
    pthread_join(sendpkt,NULL);
    pthread_join(recvack,NULL);
    pthread_exit(NULL);

    return 0 ;
}
