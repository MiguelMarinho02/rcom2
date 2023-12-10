#include "../include/download.h"

/*
ftp://ftp.up.pt/pub/kodi/timestamp.txt
ftp://netlab1.fe.up.pt/pub.txt
ftp://rcom:rcom@netlab1.fe.up.pt/pipe.txt
*/

int createSocket(char *ip, int port){
    int sockfd;
    struct sockaddr_in server_addr;
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port); 

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    if (connect(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;

    }
    return sockfd;
}

int readResponse(int socket, char *answer){
    char byte;
    int i = 0;
    int response_code;
    enum State state = START;

    memset(answer, 0, MAX_LENGTH);
    printf("Reading response\n");
    while (state != END)
    {
        read(socket, &byte, 1);
        switch (state)
        {
        case START:
            if (byte == ' '){ //encountering a space means the response is only one line
                state = SINGLE;
                answer[i++] = byte;
            }
            else if (byte == '-'){  //encountering a '-' means the response as atleast one more line
                state = MULTIPLE;
                answer[i++] = byte;
            }
            else if (byte == '\n'){ //encountering a '\n' means the response is finished
                state = END;
            }
            else{                    //parsing the numbers
                answer[i++] = byte;
            }
            break;
        case SINGLE:
            // Handle parsing of a single-line response
            if (byte == '\n'){
                state = END;
            }
            else{
                answer[i++] = byte;
            }
            break;
        case MULTIPLE:
            // Handle parsing of a multi-line response
            if (byte == '\n'){
                state = START;
                i = 0;
                memset(answer, 0, MAX_LENGTH);

            }
            else{
                answer[i++] = byte;
            }
            break;
        case END:
            break;
        default:
            break;
        }
    }  
    sscanf(answer, "%d", &response_code);
    printf("Answer: %s\n", answer);
    return response_code;
}

int auth(int socket, char *user, char *password){
    char answer[MAX_LENGTH];

    char userCommand[5 + strlen(user) + 1];  //we add 5 to insert the USER/PASS + ' ' field and 1 to include the '\n' 
    sprintf(userCommand, "USER %s\n", user);

    char passwordCommand[5 + strlen(password) + 1];
    sprintf(passwordCommand, "PASS %s\n", password);

    write(socket, userCommand, strlen(userCommand));
    if(readResponse(socket,answer) != 331){
        printf("Error passing username\n");
        return -1;
    }

    memset(answer,0,MAX_LENGTH);

    write(socket, passwordCommand, strlen(passwordCommand));
    if(readResponse(socket,answer) != 230){
        printf("Error passing password\n");
        return -1;
    }

    return 0;
}

int handlingPassiveMode(int socket, int *port,char *ip){
    char answer[MAX_LENGTH];
    char *n1,*n2,*n3,*n4,*n5,*n6;

    char pasvCommand[5] = "pasv\n";
    write(socket, pasvCommand, 5);

    if(readResponse(socket,answer) != 227){
        printf("Error with pasv command\n");
        return -1;
    }

    strtok(answer, "(");
    n1 = strtok(NULL, ",");
    n2 = strtok(NULL, ",");
    n3 = strtok(NULL, ",");
    n4 = strtok(NULL, ",");
    n5 = strtok(NULL, ",");
    n6 = strtok(NULL, ")");

    int p1, p2;

    sscanf(n5,"%d",&p1);
    sscanf(n6,"%d",&p2);
    
    *port = p1 * 256 + p2;
    sprintf(ip, "%s.%s.%s.%s", n1, n2, n3, n4);
    return 0;
}

int request_resource(int socket, char *path){
    char answer[MAX_LENGTH];

    char resourceCommand[6+strlen(path)];

    sprintf(resourceCommand, "retr %s\n", path);
    write(socket, resourceCommand, strlen(resourceCommand));

    if(readResponse(socket,answer) != 150){
        return -1;
    }

    return 0;
}

int get_resource(int socket1, int socket2, char *filename){
    FILE *fd = fopen(filename, "wb");

    if(fd==NULL){
        return -1;
    }

    char byte[1];
    int bytes;
    while((bytes = read(socket2, byte, 1)) > 0){
        if(fwrite(byte, 1, bytes, fd) == 0){
            printf("error writing to file");
            return -1;
        }
    }

    fclose(fd);
    char answer[MAX_LENGTH];
    if(readResponse(socket1,answer) != 226){
        printf("error downloading file\n");
        return -1;
    }
    return 0;
}

int closeSocket(int socket){
    if (close(socket)<0) {
        perror("close()");
        return -1;
    }
    return 0;
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(1);
    }

    struct Args url = buildArgs(argv[1]);

    char answer[MAX_LENGTH];
    int socket1 = createSocket(url.ip,21);

    printf("Socket1: %d\n", socket1);
    if (socket1 < 0 || readResponse(socket1, answer) != 220){
        printf("Error creating socket1\n"); // 220 is the response code for connection established
        exit(-1);
    }

    printf("Going to authenticate\n");
    printf("User: %s\n", url.user);
    printf("Password: %s\n", url.password);
    if (auth(socket1, url.user, url.password) != 0){
        exit(-1);
    }

    printf("Going into passive mode\n");
    int port;
    char ip[MAX_LENGTH];
    if(handlingPassiveMode(socket1,&port,ip) != 0){
        exit(1);
    }

    printf("Creating second socket for passive mode\n");
    int socket2 = createSocket(ip,port);
    memset(answer,0,MAX_LENGTH);
    if (socket2 < 0){
        printf("Error creating socket2\n");
        exit(-1);
    }

    printf("Creating request\n");
    if(request_resource(socket1, url.path) != 0){
        exit(-1);
    }

    printf("Getting resource\n");
    if(get_resource(socket1, socket2, url.filename) != 0){
        exit(-1);
    }

    printf("Closing connection\n");
    if(closeSocket(socket1) != 0){
        return -1;
    }

    if(closeSocket(socket2) != 0){
        return -1;
    }

    return 0;
}