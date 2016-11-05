#include <iostream>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include "setupserver.h"

using namespace std;

typedef struct{
    int cfd;
    int N;
    bool sendreq;
} ThreadArg;

int sendPkt(int fd, int *count, int N)
{
    timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;

    for(int i=0;i<N;++i)
    {
        uint32_t tosend = *count;
        cout << "* Sending : " << tosend << endl;
        send(fd,&tosend,sizeof(tosend),0);
        fsync(fd);
        (*count)++;
        pselect(1,NULL,NULL,NULL,&timeout,NULL);
    }
    return 0;
}

int sendPkt(int fd, uint32_t val)  //Overladed function
{
    timespec timeout;
    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;

    uint32_t tosend = val;
    cout << "# Sending : " << tosend << endl;
    send(fd,&tosend,sizeof(tosend),0);
    fsync(fd);
    pselect(1,NULL,NULL,NULL,&timeout,NULL);
    return 0;
}

void *sendData(void *myarg)
{
    ThreadArg *arg = (ThreadArg*)myarg;
    int count=1;
    while(count < 50)
    {
        if(arg->sendreq == true)
        {
            cout << "\n----------\n";
            sendPkt(arg->cfd,&count,arg->N);
            cout << "\n----------\n";
            arg->sendreq = false;
        }
    }
}

void *recvACK(void *myarg)
{
    ThreadArg *arg = (ThreadArg *)myarg;
    int fd = arg->cfd;

    fd_set client_fdset;
    uint32_t pkt = 0;
    vector<uint32_t> buf;
    int windowcount = 0;

    timespec timeout;
    timeout.tv_sec = 1*arg->N;
    timeout.tv_nsec = 0;

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(fd,&client_fdset);

        pselect(fd+1,&client_fdset,NULL,NULL,&timeout,NULL);

        if(FD_ISSET(fd,&client_fdset) && (windowcount < arg->N))
        {
            int readlen = recv(fd,&pkt,sizeof(uint32_t),0);
            if(readlen==0)
            {
                cout << "Client disconnected!\n";
                close(fd);
                exit(EXIT_SUCCESS);
            }
            cout << "Received: " << pkt << endl;

            if(pkt!= 0)
            {
                vector<uint32_t>::iterator it = find(buf.begin(), buf.end(), pkt);
                if(it==buf.end())
                {
                    buf.push_back(pkt);
                }
            }
            windowcount++;
        }
        else
        {
            for(unsigned int i = 0; i < buf.size();++i)
            {
                uint32_t resend = *(buf.begin());
                sendPkt(fd,resend);
                buf.erase(buf.begin());
            }
            arg->sendreq = true;
            windowcount = 0;
            pselect(fd+1,&client_fdset,NULL,NULL,&timeout,NULL);
        }
    }
}

int main()
{
    int serversocket = 0, clientsocket = 0;

    ThreadArg arg;
    arg.N = 5;
    arg.sendreq = true;

    pthread_t senddata, recvack;
    pthread_attr_t attr;

    cout << "Starting server..." << endl;
    setUpServer(&serversocket,&clientsocket);
    cout << "Connected!\n";

    arg.cfd = clientsocket;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    pthread_create(&senddata,&attr,sendData,(void*)(&arg));
    pthread_create(&recvack,&attr,recvACK,(void*)(&arg));
    pthread_join(senddata,NULL);
    pthread_join(recvack,NULL);
    pthread_exit(NULL);

    close(clientsocket);
    close(serversocket);

    return 0;
}
