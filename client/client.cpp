// name: Phạm Hoàng Nam
// mssv: 17021164
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>       // basic system data types
#include <sys/socket.h>      // basic socket definitions
#include <netinet/in.h>      // sockaddr_in{} and other Internet defns
#include <arpa/inet.h>       // inet(3) functions
#include <unistd.h>
#include <sys/un.h>
#include <iostream>

using namespace std;

#define SERVER_PORT 9999
#define BUFFER_SIZE 1024

int main(int argc, char **argv) {
    char serverAddress[100];
    int  serverPort;
    int     connSock;
    struct  sockaddr_in servAddr;
    char sentBuf[BUFFER_SIZE], recvBuf[BUFFER_SIZE], writeBuf[BUFFER_SIZE];
    int  sentDataLen, recvDataLen;
    string inputServAddress;

    memset(writeBuf, '0', sizeof(writeBuf));

    cout << "Enter server adress: \n";
    cin >> inputServAddress;
    cin.ignore();

    if (argc >= 3){
        strcpy(serverAddress,argv[1]);
        serverPort = atoi(argv[2]);
    }
    else{
        strcpy(serverAddress,inputServAddress.c_str());
        serverPort = SERVER_PORT;
    }

    connSock = socket(AF_INET, SOCK_STREAM, 0);
    if (connSock < 0){
        cout << "Error when creating socket\n";
        exit(1);
    }

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(serverAddress);
    servAddr.sin_port        = htons(serverPort);
    if (connect(connSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0){
        cout << "Error when connecting!";
        exit(1);
    }

    cout << "connect to server... \n";

    /* Create file where data will be stored */
    FILE *fp;
    fp = fopen("fifoserver.txt","ab");
        if(NULL == fp)
    {
        printf("Error opening file");
        return 1;
    }

    bool startDownload = false;
    bool haveFile = false;


    while(1){
        bzero(recvBuf, BUFFER_SIZE);
        cout << "Input a message to the echo server (QUIT for exit): ";
        cin.getline(sentBuf, BUFFER_SIZE);

        
        if (strcmp(sentBuf,"QUIT") == 0 || strcmp(sentBuf,"quit") == 0){
            write(connSock, sentBuf, strlen(sentBuf));
            break;
        }

        sentDataLen = write(connSock, sentBuf, strlen(sentBuf));

        if (sentDataLen < 0){
            cout << "Error when sending data\n";
            break;
        }

         if(!startDownload) {
             recvDataLen = read(connSock, recvBuf, sizeof(recvBuf));
            recvBuf[recvDataLen] = '\0';
         }
          
        if(strcmp(recvBuf, "TYPE 'start' TO START") == 0)
        { 
            startDownload = true;
            cout << "test";
            continue;
        }

        if(!startDownload) cout << "Message received from server: " << recvBuf << endl;


        if (recvDataLen < 0){
            cout << "Error when receiving data\n";
            break;
        }
        

        if(startDownload) {
            recvDataLen = read(connSock, recvBuf, sizeof(recvBuf));
            recvBuf[recvDataLen] = '\0';

            int fileLenght = atoi(recvBuf);
            cout << recvBuf;
            cout << "file's size: " << fileLenght;
            while(1) {

                recvDataLen = recv(connSock, recvBuf, BUFFER_SIZE, 0);
                
                if(recvBuf < 0)
                {
                    cout << "Read Error \n";
                }

                fwrite(recvBuf, 1,recvDataLen,fp);

                cout << recvBuf; 

            }

            startDownload = false;
        }

    }
    close(connSock);
}