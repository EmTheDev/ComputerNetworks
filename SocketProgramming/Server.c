/*
 Emil Santos
 3/19/16
 */

#include <stdio.h> /* for printf() and fprintf() */
#include <string.h> /* for memset() */   
#include <stdlib.h> /* for atoi() */    
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <unistd.h> /* for close() */   
#include <pthread.h> //for threading , link with lpthread

//the thread functions
void *connection(void *);
void messageClear();
void populateUsers(void);
char * getUsers(void);
char * getMessage();
int checkPermissions(char info[20]);
int valid();
int writeMessage(char message[1024],  char user[20]);

int numOfUsers = 0;
char theUsers[10][20];
char *currentUser;
char *usersName = "./users";
char messageStore[] = "./";

int main(int argc , char *argv[]){
    int sock_desc;
    int client_sock;
    int size;
    struct sockaddr_in server ,client;
    
    populateUsers();
    sock_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (sock_desc == -1){
        printf("Error: Could not create socket");
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(7777);
    
    if( bind(sock_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        perror("ERROR");
        return 1;
    }
    
    listen(sock_desc , 3);
    
    printf("Server Started!\n");
    printf("Listening on 127.0.0.1:7777\n");
    
    size = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    
    while( (client_sock = accept(sock_desc, (struct sockaddr *)&client, (socklen_t*)&size)) ){
        numOfUsers++;
        printf("Client %i connected\n", numOfUsers);
        
        if( pthread_create( &thread_id , NULL ,  connection , (void*) &client_sock) < 0){
            perror("ERROR: Thread could not be created");
            return 1;
        }
    }
    
    if (client_sock < 0){
        perror("ERROR");
        return 1;
    }
    return 0;
}

void *connection(void *sock_desc){
    int socket2 = *(int*)sock_desc;
    int n;
    char buffer[2000];
    
    while(1){
        bzero(buffer,256);
        n = read(socket2,buffer,255);
        if (n < 0){
            perror("ERROR: Could not read from socket");
        }
        
        if (strcmp(buffer, "0") == 0) {
            n = write(socket2, "Login", strlen("Login"));
            if (n < 0)
                perror("ERROR: Could not write to socket");
            
            bzero(buffer,256);
            n = read(socket2,buffer,255);
            if (n < 0)
                perror("ERROR: Could not read from socket");
            
            if (checkPermissions(buffer)){
                strtok(buffer, " ");
                currentUser = buffer;
                
                n = write(socket2, "granted", strlen("granted"));
                if (n < 0)
                    perror("ERROR: Could not write to socket");
            }
            else{
                n = write(socket2, "denied", strlen("denied"));
            }
        }
        
        else if (strcmp(buffer, "1") == 0) {
            printf("Return user list!\n");
            char *userList = getUsers();
            n = write(socket2, userList, strlen(userList));
            if (n < 0)
                perror("ERROR: Could not write to socket");
        }
        
        else if (strcmp(buffer, "2") == 0) {
            char userDest[20];
            
            n = write(socket2, "user?", strlen("user?"));
            if (n < 0)
                perror("ERROR: Could not write to socket");
            
            bzero(buffer,256);
            n = read(socket2,buffer,255);
            if (n < 0)
                perror("ERROR: Could not read from socket");
            
            if (valid(buffer)) {
                printf("A message to %s\n", buffer);
                strcpy(userDest, buffer);
                n = write(socket2, "valid user", strlen("valid user"));
                
                bzero(buffer,256);
                n = read(socket2,buffer,255);
                if (n < 0)
                    perror("ERROR: Could not read from socket");
                printf("Message: %s\n", buffer);
                if (writeMessage(buffer, userDest)){
                    n = write(socket2, "sent ok", strlen("sent ok"));
                    if (n < 0)
                        perror("ERROR: Could not write to socket");
                }
                else{
                    n = write(socket2, "send error", strlen("send error"));
                    if (n < 0)
                        perror("ERROR: Could not write to socket");
                }
            }
            else {
                printf("User not found!\n");
                n = write(socket2, "User not found!", strlen("User not found!"));
                if (n < 0)
                    perror("ERROR: Could not write to socket");
            } 
        }
        else if (strcmp(buffer, "3") == 0) {
            n = write(socket2, "ok", strlen("ok"));
            bzero(buffer,256);
            n = read(socket2,buffer,255);
            
            if (valid(buffer)) {
                printf("Send back %s's messages(s)!\n", buffer);
                n = write(socket2, getMessage(buffer), strlen(getMessage(buffer)));
                messageClear();
            }
            else {
                printf("User not found!\n");
                n = write(socket2, "User not found!", strlen("User not found!"));
                if (n < 0)
                    perror("ERROR: Could not write to socket");
            }
        }
        else if (strcmp(buffer, "4") == 0) {
            printf("Client disconnected!\n");
        }
    }
    return 0;
}

void messageClear(){
    FILE *userFile;
    char *fileLocation = malloc(strlen(messageStore) + strlen(currentUser) + 1);
    strcpy(fileLocation, messageStore);
    strcat(fileLocation, currentUser);
    userFile = fopen(fileLocation, "w");
    fputs(" ", userFile);
    fclose(userFile);
}

void populateUsers(void){
    FILE *userFile;
    char temp[512];
    int totalUsers = 0;
    
    if((userFile = fopen(usersName, "r")) == NULL) {
        exit(1);
    }
    while(fgets(temp, 512, userFile) != NULL) {
        strtok(temp, "\n ");
        strcpy(theUsers[totalUsers], temp);
        totalUsers++;
    }
    if(userFile) {
        fclose(userFile);
    }
}

char * getUsers(void){
    FILE *userFile;
    char temp[512];
    char users[1024];
    if((userFile = fopen(usersName, "r")) == NULL) {
        perror("Could not find file");
        exit(1);
    }
    users[0] = '\0';
    while(fgets(temp, 512, userFile) != NULL) {
        strtok(temp, "\n ");
        strcat(users, temp);
        strcat(users, " \n");
    }
    if(userFile) {
        fclose(userFile);
    }
    return users;
}


char * getMessage(){
    FILE *userFile;
    char *fileLocation = malloc(strlen(messageStore) + strlen(currentUser) + 1);
    long length;
    char *buffer = 0;
    strcpy(fileLocation, messageStore);
    strcat(fileLocation, currentUser);
    userFile = fopen(fileLocation, "a+");
    fseek(userFile, 0, SEEK_END);
    length = ftell(userFile);
    fseek(userFile, 0, SEEK_SET);
    buffer = malloc(length);
    fread(buffer, 1, length, userFile);
    fclose(userFile);
    return buffer;
}

int checkPermissions(char aUser[20]){
    FILE *userList;
    char line[200];
    userList = fopen(usersName, "r");
    if(!userList){
        perror("Could not find file");
    }
    while (fgets(line, 200, userList) != NULL) {
        strtok(line, "\n");
        if (strcmp(line, aUser) == 0){
            printf("Login username is %s\n", strtok(line, " "));
            printf("Login password is %s\n", strtok(NULL, "\n"));
            return 1;
        }
    }
    return 0;
}

int valid(char *user){
    int i;
    int valid = 0;
    for (i = 0; i < 10; i++){
        if (strcmp(theUsers[i], user) == 0){
            valid = 1;
        }
    }
    return valid;
}

int writeMessage(char message[1024], char user[20]){
    FILE *userFile;
    char *fileLocation = malloc(strlen(messageStore) + strlen(user) + 1);
    strcpy(fileLocation, messageStore);
    strcat(fileLocation, user);
    userFile = fopen(fileLocation, "a");
    strcat(message, "\n");
    fputs(message, userFile);
    fclose(userFile);
    return 1;
}
