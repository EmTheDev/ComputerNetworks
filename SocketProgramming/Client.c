/*
 Emil Santos
 3/19/16
 This program handles client side options.
 */

#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>

#define SA struct sockaddr

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
char user[20];

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr; /* Echo server address */
    struct hostent *serv; 
    int sock; /* socket descriptor */
    int portNum; /* stores port number */
    int readWrite; /* stores read and write size */
    int userCommand; /* The chosen option from the user */
    int loggedIn = 0; /* indication to see if user is logged in 1 = yes */
    char userIP[20]; /* allocate twenty spaces for the user IP address */
    char userName[20]; /* stores user name given by the user */
    char passWord[20]; /* stores user passWord given by the user */
    char buffer[1024]; /* buffer for recieved and sent messages */
    
    /* User Command List */
    do{
        printf("Attempt 6\n");
        printf("Command:\n0. Connect to the server");
        printf("\n1. Get the user list");
        printf("\n2. Send a message");
        printf("\n3. Get my messages");
        printf("\n4. Initiate a chat with my friend");
        printf("\n5. Chat with my friend");
        printf("\nEnter option number: ");
        scanf(" %i", &userCommand);
        
        //Option 0: Connect to the server
        if (userCommand == 0){
            if (loggedIn) {
                close(sock);
            }
             
            //get server IP address from user
            printf("Enter IP address: ");
            scanf(" %s", userIP);
            
            //get port number from user
            printf("Enter port number: ");
            scanf(" %d", &portNum);

            sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            
            //Error code if socket is invalid
            if (sock < 0)
                error("ERROR: Could not open socket");
            serv = gethostbyname(userIP);
            
            //Error code if host is invalid
            if (serv == NULL) {
                fprintf(stderr,"ERROR: Trouble reaching a host");
                exit(0);
            }
            
            //bzero sets all values in a given buffer to zero
            bzero((char *) &serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)serv->h_addr,
                  (char *)&serv_addr.sin_addr.s_addr,
                  serv->h_length);
            serv_addr.sin_port = htons(portNum);
            
            if (connect(sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
                error("ERROR: Could not connect");
                exit(1);
            }
            
            printf("Welcome! Please log in.\n");

            //get user username
            printf("userName: ");
            scanf("%s", &userName);

            //get user password
            printf("passWord: ");
            scanf("%s", &passWord);
            
            readWrite= write(sock,"0",strlen("0"));
            if (readWrite< 0)
                error("ERROR: Could not write to socket");
            
            //set buffer to zero
            bzero(buffer,1024);

            readWrite = read(sock,buffer,1023);
            if (readWrite< 0)
                error("ERROR: Could not read from socket");
            
            //if not equal, send user name and password to be checked
            if (strcmp(buffer, "Login") == 0) {
                bzero(buffer,1024);
                strcat(buffer, userName);
                strcat(buffer, " ");
                strcat(buffer, passWord);
                readWrite= write(sock, buffer, strlen(buffer));
                if (readWrite< 0)
                    error("Error: Could not write to socket");
                
                //feedback from the server authenticating the username and password
                bzero(buffer,1024);
                readWrite= read(sock,buffer,1023);
                if (strcmp(buffer, "granted") == 0) {
                    printf("Login Success!\n");
                    loggedIn = 1;
                    strcpy(user, userName);
                }
                else if (strcmp(buffer, "denied") == 0){
                    printf("Username and password invalid.\n");
                }
            }
        }


        //Option 1: Get the user list
        else if (userCommand == 1 && loggedIn){
            int count = 0, i;
            readWrite= write(sock,"1",strlen("1"));
            if (readWrite< 0)
                error("Error: Could not write to socket");
            
            bzero(buffer,1024);
            readWrite= read(sock,buffer,1023);
            if (readWrite< 0)
                error("Error: Could not read from socket");
            
            for (i = 0;buffer[i] != '\0';i++)
            {
                if (buffer[i] == ' ')
                    count++;
            }
            printf("Total users: %i\n",count);
            printf("%s\n",buffer);
        }


        //Option 2: Send a message
        else if (userCommand == 2 && loggedIn){
            char whichUser[20];
            char message[1024];
            
            //test to see if host is reachable with response
            readWrite= write(sock, "2", strlen("2"));
            if (readWrite< 0)
                error("Error: Could not write to socket");
            bzero(buffer,1024);
            readWrite= read(sock,buffer,1023);
            if (readWrite< 0)
                error("Error: Could not read from socket");
            
            //stores destination username to be sent
            if (strcmp(buffer, "user?") == 0) {
                printf("Please enter the destination userName: ");
                scanf(" %[^\n]s", whichUser);
                readWrite= write(sock, whichUser, strlen(whichUser));
                if (readWrite< 0)
                    error("Error: Could not write to socket");
                
                //checks to see if user is valid. if so, write a message
                bzero(buffer,1024);
                readWrite= read(sock,buffer,1023);
                if (readWrite< 0)
                    error("Error: Could not read from socket");
                if (strcmp(buffer, "valid user") == 0) {
                    printf("Please enter the message: ");
                    scanf(" %[^\n]s", message);
                    
                    // send message
                    readWrite= write(sock, message, strlen(message));
                    if (readWrite< 0)
                        error("Error: Could not write to socket");
                    bzero(buffer,1024);
                    readWrite= read(sock,buffer,1023);
                    if (readWrite< 0)
                        error("Error: Could not read from socket");
                    
                    //notifies the user that the message has been sent successfully
                    if (strcmp(buffer, "sent ok") == 0) {
                        printf("Message sent!\n");
                    }
                    else if(strcmp(buffer, "send error") == 0){
                        printf("Error sending message!\n");
                    }
                }
            }
        }


        //Option3: Get my messages
        else if (userCommand == 3 && loggedIn){
            
            
            readWrite= write(sock, "3", strlen("3"));
            if (readWrite< 0)
                error("Error: Could not write to socket");
            bzero(buffer,1024);
            readWrite= read(sock,buffer,1023);

            //write the message
            readWrite= write(sock, user, strlen(user));
            bzero(buffer,1024);
            readWrite= read(sock,buffer,1023);
            if (readWrite< 0)
                error("Error: Could not read from socket");
            
            printf("Message(s): \n");
            printf("%s\n", buffer);
        }  


        //Option4: Initiate chat with my friend
        else if (userCommand == 4 && loggedIn){
            //test to see if host is reachable with response
            readWrite= write(sock, "4", strlen("4"));
            if (readWrite< 0)
                error("Error: Could not write to socket");
            
            close(sock);
            loggedIn = 0;
            
            printf("Enter port number to listen on: ");
            scanf(" %d", &portNum);
            fgetc(stdin);
            
            int a;
            int n;
            int existingClient = 0;
            int isConnected = 1;
            char buffer1[1024];
            struct sockaddr_in servaddr,client1;
            sock=socket(AF_INET,SOCK_STREAM,0);
            if(sock==-1)
            {
                printf("socket creation failed...\n");
                exit(0);
            }
            bzero(&servaddr,sizeof(servaddr));
            servaddr.sin_family=AF_INET;
            servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
            servaddr.sin_port=htons(portNum);
            if((bind(sock,(SA*)&servaddr, sizeof(servaddr)))!=0)
            {
                printf("socket bind failed...\n");
                exit(0);
            }
            if((listen(sock,5))!=0)
            {
                printf("Listen failed...\n");
                exit(0);
            }
            else
                printf("I am listening on 127.0.0.1:%i\n", portNum);
            
            while (isConnected) {
                n=sizeof(client1);
                a=accept(sock,(SA *)&client1,&n);
                existingClient = 1;
                if(a<0){
                    printf("server acccept failed...\n");
                    exit(0);
                }
                else{
                    
                    bzero(buffer,1024);
                    readWrite = read(a,buffer,1023);
                    printf("%s is connected\n", buffer);
                    printf("(Type 'Bye' to stop the conversation\n");
                    write(a, userName, sizeof(userName));
                }
                
                while(existingClient){
                    bzero(buffer1,1024);
                    read(a,buffer1,sizeof(buffer1));
                    printf("%s: %s", buffer, buffer1);
                    
                    if (strcasecmp(buffer1, "bye\n") == 0) {
                        existingClient = 0;
                    }
                    else {
                        printf("%s: ", userName);
                        bzero(buffer1,1024);
                        readWrite=0;
                        fgets (buffer1, 1024, stdin);
                        write(a,buffer1,sizeof(buffer1));
                        if(strcasecmp("bye\n",buffer1)==0){
                            existingClient = 0;
                        }
                    }
                }
            }
            
            close(sock);
        }
        //Option 5: Chat with my friend
        else if (userCommand == 5 && loggedIn){
            //test to see if host is reachable with response
            readWrite = write(sock, "4", strlen("4"));
            if (readWrite < 0)
                error("Error: Could not write to socket");
            printf("------------------Disconnected from server--------------------\n");
            
            close(sock);
            
            printf("Enter you friends IP address: ");
            scanf(" %s", &userIP);
            
            printf("Enter you friends port number: ");
            scanf(" %d", &portNum);
            fgetc(stdin);
            int clientSocket;
            char buffer1[1024];
            int readWrite ;
            struct sockaddr_in servaddr;
            clientSocket=socket(AF_INET,SOCK_STREAM,0);
            if(clientSocket==-1)
            {
                printf("socket creation failed...\n");
                exit(0);
            }
            bzero(&servaddr,sizeof(servaddr));
            servaddr.sin_family=AF_INET;
            servaddr.sin_addr.s_addr=inet_addr(userIP);
            servaddr.sin_port=htons(portNum);
            if(connect(clientSocket,(SA *)&servaddr,sizeof(servaddr))!=0){
                printf("connection with the server failed...\n");
                exit(0);
            }
            else{
                write(clientSocket, userName, sizeof(userName));
                bzero(buffer,1024);
                readWrite = read(clientSocket, buffer, 1023);
            }
            
            for(;;){
                bzero(buffer1,sizeof(buffer1));
                printf("%s: ", userName);
                readWrite=0;
                fgets (buffer1, 1024, stdin);
                write(clientSocket,buffer1,sizeof(buffer1));
                if((strcasecmp(buffer1,"bye\n"))==0){
                    break;
                }
                
                bzero(buffer1,sizeof(buffer1));
                read(clientSocket,buffer1,sizeof(buffer1));
                printf("%s: %s", buffer, buffer1);
                
                if((strcasecmp(buffer1,"bye\n"))==0){
                    break;
                }
            }
            close(clientSocket);
        }
        else if(userCommand > 6 || userCommand <0){
            printf("Invalid 0ption! Please choose an option between 1-5 or 6 to exit.\n");
        }
        else if ((userCommand <= 5 && userCommand >= 0) && !loggedIn){
            printf("Please log in first by entering 0\n");
        }
        
    } while (userCommand != 6);
    if (loggedIn) {
        close(sock);
    }
    return 0;
}