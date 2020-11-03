// name: Phạm Hoàng Nam
// mssv: 17021164
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h> 
#include <unistd.h>
#include <iostream>

using namespace std;

#define SERVER_PORT 9000
#define BUFFER_SIZE 1024


const string HELLO_200 = "200 Hello Client";
const string DOWNLOAD_FILE_OK = "210 Download file OK";
const string FILE_NOT_FOUND = "211 File not found";
const string WRONG_SYNTAX = "WRONG SYNTAX";
const string NOT_READY = "ENTER HELLO SERVER TO START";
const string TYPE_START = "TYPE 'start' TO START";

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

void * process(void * ptr)
{
	char buffer[BUFFER_SIZE];
	int len;
	connection_t * conn;

    if (!ptr) pthread_exit(0); 
        conn = (connection_t *)ptr;

    bool typingFileName = false, ready = false, downloadReady = false;
    bool wrongSyntax = false;
    FILE *fp = NULL;

    while(1) {
        
        buffer[BUFFER_SIZE] = 0;

        /* read message */
        len = read(conn->sock, buffer, BUFFER_SIZE-1);

        buffer[len] = '\0';

        if(len > 0) {
            /* print message */
            cout << "Received a message from the client: "  << buffer << endl;

            // write(conn->sock, buffer, BUFFER_SIZE);

            if(!typingFileName) {
                for(int i=0; i < len; i++){
                    if (buffer[i] >= 97  && buffer[i] <= 122)
                        buffer[i] -= 32;
                }
            }
            
            // client close connection
            if (strcmp(buffer,"QUIT") == 0){
                close(conn->sock);
                cout << "Close the connection\n";
                break;
            }
            
            if(strcmp(buffer, "HELLO SERVER") == 0) {
                write(conn->sock, HELLO_200.c_str(), HELLO_200.length());
                ready = true;
            }
            else if(strcmp(buffer, "DOWNLOAD FILE") == 0) {
                if(ready) {
                    write(conn->sock, DOWNLOAD_FILE_OK.c_str(), DOWNLOAD_FILE_OK.length());
                    typingFileName = true;
                }
                else {
                    write(conn->sock, NOT_READY.c_str(), NOT_READY.length());
                }

            } 
            else if(strcmp(buffer, "START") == 0) {
                downloadReady = true;
            }
            else if(typingFileName) {
    
                fp = fopen(buffer,"r");
                if(fp == NULL)
                {
                    write(conn->sock, FILE_NOT_FOUND.c_str(), FILE_NOT_FOUND.length());
                }
                else 
                {
                    fseek(fp, 0L, SEEK_END);
                    int fileSize = ftell(fp);
                    fseek(fp, 0, SEEK_SET); // seek back to beginning of file

                    cout << "file 's name: " << buffer;
                    cout << "file 's size:" << fileSize << endl;

                    write(conn->sock, TYPE_START.c_str(), TYPE_START.length());
                    
                }
                typingFileName = false;
            } 
            else wrongSyntax = true;
    
            if(downloadReady) {
                if(downloadReady) {
                        char buff[BUFFER_SIZE]={0};
                        while(fgets(buff, BUFFER_SIZE, fp) != NULL) {

                            int nsent = 0;
                            nsent = write(conn->sock, buff, BUFFER_SIZE);

                            if (nsent <= 0) {
                                exit(1);
                            }
                            cout << buff;
                            cout << nsent << "----------------------------------------------------------------------" << endl;
                            bzero(buff, BUFFER_SIZE);
                        }
                    
                        // fseek(fp, 0L, SEEK_END);
                        // int fileSize = ftell(fp);
                        // fseek(fp, 0, SEEK_SET); // seek back to beginning of file
                        // cout << fileSize << endl;
                        // /* First read file in chunks of 256 bytes */
                        // unsigned char buff[fileSize]={0};
                        // int nread = fread(buff,1,fileSize,fp);

                        // /* If read was success, send data. */
                        // if(nread > 0)
                        // {
                        //     write(conn->sock, buff, nread);
                        // } 

                    }

                    wrongSyntax = false;
            }
            
            if(wrongSyntax) {
                write(conn->sock, WRONG_SYNTAX.c_str(), WRONG_SYNTAX.length());
            }
        }

    }

	/* close socket and clean up */
	close(conn->sock);
	free(conn);
	pthread_exit(0);
}

int main(int argc, char **argv) {
    int sock = -1;
	struct sockaddr_in address;
	int port = 9999;
	connection_t * connection;
	pthread_t thread;

	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock <= 0)
	{
		cout << "error: cannot create socket " << argv[0] << endl;
		return -3;
	}

	/* bind socket to port */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
	{
		cout << "error: cannot bind socket to port " << port << endl;
		return -4;
	}

	/* listen on port */
	if (listen(sock, 5) < 0)
	{
		cout <<  "error: cannot listen on port\n";
		return -5;
	}

	cout << "ready and listening\n";
	
	while (1)
	{
		/* accept incoming connections */
		connection = (connection_t *)malloc(sizeof(connection_t));
		connection->sock = accept(sock, &connection->address, (socklen_t *) &connection->addr_len);
        cout << "connect to a client" << endl;
		if (connection->sock <= 0)
		{
			free(connection);
		}
		else
		{
			/* start a new thread but do not wait for it */
			pthread_create(&thread, 0, process, (void *)connection);
			pthread_detach(thread);
		}
	}
	
	return 0;
}

