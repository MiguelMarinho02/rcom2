#ifndef URL_HEADER
#define URL_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include<arpa/inet.h>


typedef struct Args
{
    char user[128];
    char password[128];
    char host[256];
    char path[256];
    char ip[128];
    char filename[128];
    char hostname[256];
} Args;

struct Args buildArgs(char *url);

int getIp(char *host,struct Args *args);

int getFilename(struct Args *args);

#endif
