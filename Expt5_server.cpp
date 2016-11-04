#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include "setupserver.h"

using namespace std;

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

int sendData(int fd,int N,bool *sendreq)
{
    int count=1;
    while(count < 50)
    {
        if(*sendreq == true)
        {
            cout << "\n----------\n";
            sendPkt(fd,&count,N);
            cout << "\n----------\n";
            *sendreq = false;
        }

    }
    return 0;
}

int recvACK(int fd, int N, bool *sendreq)
{
    fd_set client_fdset;
    uint32_t pkt = 0, oldpkt = 0;
    vector<uint32_t> buf;
    int windowcount = 0;

    timespec timeout;
    timeout.tv_sec = 1*N;
    timeout.tv_nsec = 0;

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(fd,&client_fdset);

        pselect(fd+1,&client_fdset,NULL,NULL,&timeout,NULL);

        if(FD_ISSET(fd,&client_fdset) && (windowcount < 5))
        {
            oldpkt = pkt;
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
            //cout << "Window Count: " << windowcount << endl;
        }
        else
        {
            //cout << "Ok! 1.1\n";
            //*sendreq = false;
            for(int i = 0; i < buf.size();++i)
            {
                uint32_t resend = *(buf.begin());
                sendPkt(fd,resend);
                buf.erase(buf.begin());
            }
            *sendreq = true;
            windowcount = 0;
            pselect(fd+1,&client_fdset,NULL,NULL,&timeout,NULL);
            //cout << "Ok! 1.2\n";
        }
        //cout << "Ok! 2.0\n";
    }
    return 0;
}

int main()
{
    int serversocket = 0, clientsocket = 0;
    int N = 5;
    int *reqsend = new int;
    bool *sendreq = new bool;

    *reqsend = 0;
    *sendreq = true;

    cout << "Starting server..." << endl;
    setUpServer(&serversocket,&clientsocket);
    cout << "Connected!\n";

    thread SendData(sendData,clientsocket,N,sendreq);
    thread RecvACK(recvACK,clientsocket,N,sendreq);

    SendData.join();
    RecvACK.join();

    close(clientsocket);
    close(serversocket);

    delete reqsend;
    delete sendreq;

    return 0;
}

