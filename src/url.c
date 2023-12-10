#include "../include/url.h"

struct Args buildArgs(char *url) {
    struct Args args;

    char *type = strtok(url, ":");
    if (type == NULL || strcmp(type, "ftp") != 0) {
        printf("ftp is not being used");
        exit(1);
    }

    char *user_password_host = strtok(NULL, "/");
    char *path = strtok(NULL, "");
    
    char *user_password_delimiter = strchr(user_password_host, '@');
    if (user_password_delimiter != NULL) { //found credentials
        char *user_password = strtok(user_password_host, "@");
        char *host = strtok(NULL, "");
        char *user = strtok(user_password, ":");
        char *password = strtok(NULL, "");
        
        // in case the user and password are not present but the structure is the same
        strcpy(args.user, user ? user : "anonymous");
        strcpy(args.password, password ? password : "password");
        strcpy(args.host, host ? host : "");
    } else {
        strcpy(args.user, "anonymous");
        strcpy(args.password, "password");
        strcpy(args.host, user_password_host ? user_password_host : "");
    }

    strcpy(args.path, path ? path : "");

    getIp(args.host, &args);
    getFilename(&args);

    return args;
}


int getIp(char *host, struct Args *args){
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    strcpy(args->hostname,h->h_name);
    strcpy(args->ip,inet_ntoa(*((struct in_addr *) h->h_addr)));
    
    return 0;
}

int getFilename(struct Args *args){
    char tempPath[256];
    strcpy(tempPath,args->path);
    char* token = strtok(tempPath,"/");
    while(token != NULL){
        strcpy(args->filename,token);
        token = strtok(NULL,"/");
    }

    return 0;
}
