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
#include <thread>

using namespace std;

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

    getaddrinfo(NULL, "5555", &host_info, &host_info_list);

    int socketfd;
    socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,host_info_list->ai_protocol);

    int yes = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);

    cout << "Listning for clients...";
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

int sendData(int fd, bool *ackreceived, int *lastSent, int *lastACK)
{
    const int N=5;
    int count = 1;

    while(count < 100)
    {
        if(*ackreceived)
        {
            count = *lastACK + 1;
            sendPackets(N,fd,&count);
            *lastSent = count-1;
            *ackreceived = false;
        }
    }
    return 0;
}

int recvACK(int fd,bool *ackreceived, int *lastSent, int *lastACK)
{
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
                *lastACK = pkt;
            }
            else
            {
                pkt = oldpkt;
            }
        }
        else
        {
            cout << "---------------\nlastSent: " << *lastSent << " lastACK: " << *lastACK << "\n----------------\n";
            oldpkt = *lastACK;
            *ackreceived = true;
        }
    }

    return 0;
}

int main()
{   
    bool *ackrcvd = new bool;
    int *lastACK = new int;
    int *lastSent = new int;
    *ackrcvd = true;
    *lastSent = 0;
    *lastACK = 0;
    int srvrfd, cfd;

    cout << "------SERVER-------\n";

    setUpServer(&srvrfd,&cfd);

    thread SendPkt(sendData,cfd,ackrcvd,lastSent,lastACK);
    thread RecvAck(recvACK,cfd,ackrcvd,lastSent,lastACK);
    SendPkt.join();
    RecvAck.join();

    delete ackrcvd;
    delete lastACK;
    delete lastSent;
    return 0 ;
}
