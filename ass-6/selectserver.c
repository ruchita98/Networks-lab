/*
Name : Vedic Partap
Roll No: 16CS10053
Assignment : 3

Server Side 

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

#define PORT 8182
#define MAX_CHAR 100
#define MAX_WORDS 100
#define MAX_LINE 1024
typedef char string[MAX_CHAR + 1];
/*
Funtion to receive the string from the client
=========

Input:
sockfd : socket id 
buffer : character buffer for storing the received output
cliaddr: The address of the client

=========

 */
int udpreceive(int sockfd, char buffer[], struct sockaddr_in *cliaddr)
{
    int n;
    socklen_t len;

    len = sizeof(*cliaddr);
    n = recvfrom(sockfd, (char *)buffer, MAX_LINE, MSG_DONTWAIT,
                 (struct sockaddr *)cliaddr, &len);
    if(n<0)
    {
         if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
            return -1;// use select() or epoll() to wait for the socket to be writable again
        }
        else
        {
            printf("Connection Failed\n");
            return 1;
        }
    }
    printf("UDP\n");
    buffer[n] = '\0';
    return 0;
}
//finding max of number
int max(int a,int b)
{
    if(a>b)return a;
    else return b;
}
// get IP from host
int hostname_to_ip(char *hostname, char *ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL)
    {
        // get the host info
        printf("Error in finding hostname\n");
        return 1;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcat(ip, inet_ntoa(*addr_list[i]));
        strcat(ip, ", ");
        // return 0;
    }

    return 1;
}
int main()
{
    int sockfd1, sockfd2, newsockfd;
    socklen_t clilen1,clilen2;
    struct sockaddr_in cli_add1, cli_add2, serv_addr;
    // struct timeval timeout;
    //creating socket 1
    if((sockfd1=socket(AF_INET, SOCK_STREAM,0))<0)
    {
        perror("[ERROR] Unable to create the TCP socket\n");
        exit(1);
    }

    // creating socket 2
    if((sockfd2=socket(AF_INET,SOCK_DGRAM, 0))<0)
    {
        perror("[ERROR] Unable to create the UDP socket\n");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&cli_add1, 0, sizeof(cli_add1));
    memset(&cli_add2, 0, sizeof(cli_add2));

    // setting server
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // bind the socket 1 
    if(bind(sockfd1,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to bind the TCP socket\n");
        exit(1);
    }
    // bind the socket 2
    if(bind(sockfd2,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        perror("[ERROR] Unable to bind the UDP socket\n");
        exit(1);
    }
    listen(sockfd1, 5);
    fcntl(sockfd1,F_SETFL,O_NONBLOCK);
    // int x=0,y=0;
    while(1)
    {
        // sleep(0.01);
        clilen1 = sizeof(cli_add1);
        clilen2 = sizeof(cli_add2);

        while(1)
        {
            if ((newsockfd = accept(sockfd1, (struct sockaddr *)&cli_add1, &clilen1)) < 0)
            {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                {
                    break;
                }
                else
                {
                    printf("Connection Failed\n");
                    continue;
                }
            }
            pid_t child;
            if((child=fork())==0)
            {
                printf("TCP\n");
                
                char filename[MAX_CHAR], temp_buf[MAX_CHAR];
                for (int i = 0; i < MAX_CHAR; i++)
                {
                    filename[i] = temp_buf[i] = '\0';
                }
                int sz = 0, total_sz = 0;
                total_sz = sz = recv(newsockfd, filename, MAX_CHAR, 0);
                // printf("Filename : %s\n", filename);
                while ((sz = recv(newsockfd, temp_buf, MAX_CHAR, MSG_DONTWAIT)) > -1)
                {
                    total_sz += sz;
                    strcat(filename, temp_buf);
                }
                FILE *fptr;
                fptr = fopen(filename, "r");
                char message[MAX_CHAR];
                if (fptr == NULL)
                {
                    perror("[ERROR] File not found\n");
                    message[0] = '\0';
                    send(newsockfd, message, sizeof(message), 0);
                    close(newsockfd);
                    continue;
                }
                else
                {
                    while (fgets(message, sizeof(message), fptr))
                    {
                        // printf("%s", message);
                        message[strlen(message)] = '\0';
                        send(newsockfd, message, strlen(message)+1, 0);
                    }
                    message[0] = '\0';
                    // printf("%ld\n", strlen(message));
                    send(newsockfd, message, strlen(message)+1, 0);
                    fclose(fptr);
                }
                close(newsockfd);
            }
        }
        while(1)
        {
            char hostname[MAX_LINE];    //Buffer to store the output
            int x = udpreceive(sockfd2, hostname, &cli_add2);
            if(x==-1)
                break;
            if(x==1)
                continue;
            char ip[1000];
            for(int i=0;i<1000;i++)
            ip[i]='\0';
            hostname_to_ip(hostname, ip);
            sendto(sockfd2,(const char*)ip,strlen(ip)+1,0,
                (const struct sockaddr*)&cli_add2,clilen2);
            printf("[SUCCESS] IP(s) Send\n");
        }
        
        
    }
    return 0;
}