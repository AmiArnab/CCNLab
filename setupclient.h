#ifndef SETUPCLIENT
#define SETUPCLIENT

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

int setUpClient(int *clientfd);

#endif // SETUPCLIENT

