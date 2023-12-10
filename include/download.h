#ifndef DOWNLOAD_HEADER
#define DOWNLOAD_HEADER

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "../include/url.h"

#define MAX_LENGTH 1000

typedef enum State{
    START,
    SINGLE,
    MULTIPLE,
    END
} State;

#endif
