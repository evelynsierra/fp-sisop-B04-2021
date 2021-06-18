#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>


#define SIZE_BUF 100

int main(int argc, char *argv[])
{
    int sock,root,commandTrue;
    struct sockaddr_in server;
    char username[1024];
    char password[1024];
    char unpass[30001];
    char request[1024];
    char response[1024];
    
    //membuat socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(getuid()){ 
        if (sock == -1) //jika gagal
        {
            printf("Could not create socket");
        }
        puts("Socket created");

        server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip address
        server.sin_family = AF_INET;
        server.sin_port = htons(7000); //port

        //koneksikan ke remote server
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("Connection failed");
            return 1;
        }

        puts("Connected");

        strcpy(username, argv[2]);
        strcpy(password, argv[4]);
        sprintf(unpass, "%s:::%s:::", username, password);
        send(sock, unpass, sizeof(unpass), 0);
        if (recv(sock, response, 1024, 0) < 0)
        {
            printf("Login gagal\n");
            exit(1);
        }
        root = 0;
        commandTrue = 0;
        while(true){
            //kirim command ke server
            printf("%s >> ", argv[2]);
            fgets(request, sizeof(request), stdin);

            if (send(sock, request, 1024, 0) < 0) //jika tidak ada command
            {
                puts("Request failed");
                return 1;
            }
	     
	    //untuk mengambil respons dari server ke client
            if (recv(sock, response, 1024, 0) < 0)
            {
                puts("Response failed");
                break;
            }

            puts("respon server :");
            puts(response);
            memset(&response, '\0', sizeof(response));
        }
    }
    else{
        if (sock == -1) //jika gagal
        {
            printf("soket gagal dibuat");
        }
        puts("socket berhasil dibuat");

        server.sin_addr.s_addr = inet_addr("127.0.0.1"); //ip address
        server.sin_family = AF_INET;
        server.sin_port = htons(7000); //port
        
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) //jika koneksi gagal
        {
            perror("koneksi gagal");
            return 1;
        }
        puts("terkoneksi");

        root = 0;
        commandTrue = 0;   
        while(true){
            //kirim command ke server
            root = 1;
            printf(">> ");
            fgets(request, sizeof(request), stdin);

            if (send(sock, request, strlen(request), 0) < 0) //jika tidak ada command
            {
                puts("Request failed");
                return 1;
            }
            
	    //untuk mengambil respons dari server ke client
            if (recv(sock, response, 1024, 0) < 0)
            {
                puts("Response failed");
                break;
            }

            puts("Server reply :");
            puts(response);
            memset(&response, '\0', sizeof(response));
        }
    }
    
    close(sock);
    return 0;
}
