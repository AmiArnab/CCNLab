#include <iostream>
#include <vector>
#include <algorithm>
#include "setupclient.h"

using namespace std;

int main()
{
    int socketfd = 0;
    int windowcount = 0;

    uint32_t pkt = 0, ack = 0;

    fd_set client_fdset;

    vector<uint32_t> buf;

    cout << "------CLIENT-------\n";
    cout << "Connecting to server...\n" << endl;

    setUpClient(&socketfd);

    cout << "Connected!\n";

    while(true)
    {
        FD_ZERO(&client_fdset);
        FD_SET(socketfd,&client_fdset);

        pselect(socketfd+1,&client_fdset,NULL,NULL,NULL,NULL);

        if(FD_ISSET(socketfd,&client_fdset))
        {
            pkt = 0;
            int readlen = recv(socketfd,&pkt,sizeof(uint32_t),0);

            if(readlen==0)
            {
                cout << "Server disconnected!\n";
                close(socketfd);
                exit(EXIT_SUCCESS);
            }
            cout << "Received: " << pkt << endl;

            vector<uint32_t>::iterator it = find(buf.begin(), buf.end(), pkt);

            if(it!=buf.end())
            {
                buf.erase(it);
                //cout << "Erased: " << *it << endl;
            }

            if(rand() > 0.2*RAND_MAX && buf.size() < 6)
            {
                if(!buf.empty())
                {
                    ack = *(buf.begin());
                }
                else
                {
                    ack = 0;
                }
                send(socketfd,&ack,sizeof(uint32_t),0);
                fsync(socketfd);
                cout << "* Sending: " << ack << endl;
            }
            else
            {
                if(buf.size() < 6)
                {
                    buf.push_back(pkt);
                    cout << "Dropped: " << pkt << endl;
                }
                else
                {
                    uint32_t bufpkt = *(buf.begin());
                    buf.erase(buf.begin());
                    send(socketfd,&bufpkt,sizeof(uint32_t),0);
                    fsync(socketfd);
                    cout << "# Sending: " << bufpkt << endl;
                }
            }
        }
    }

    return 0 ;
}

