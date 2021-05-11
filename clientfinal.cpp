#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>


using namespace std;

int sock;
struct sockaddr_in server;
struct hostent *hp;
char buf[1024];
char s[10000];
char ret[10000];

void *writeToServer(void *ptr);
void *readFromServer(void *ptr);

int main(int argc,char *argv[])
{
    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
    {
        perror("socket failed\n");
        exit(0);
    }
    hp = gethostbyname(argv[1]);
    if (hp == 0)
    {
        fprintf(stderr,"%s host failed\n",argv[1]);
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    bcopy(hp->h_addr,&server.sin_addr,sizeof(hp->h_addr));

    int con = connect(sock,(struct sockaddr*)&server,sizeof(server));
    if (con < 0)
    {
        perror(("connection failed\n"));
        exit(1);
    }


            pthread_t thread1, thread2;
            char* message = "hello";

            pthread_create(&thread1,NULL,writeToServer,(void*) message);
            pthread_create(&thread2,NULL,readFromServer,(void*) message);

            pthread_join(thread1,NULL);
            pthread_join(thread2,NULL);



}
void *writeToServer(void *ptr)
{
    while(true){
    if (write(STDOUT_FILENO,"Please enter a command\n",24) < 0)
        perror("prompt error: ");
    int bytes = read(STDIN_FILENO,s,sizeof(s));
    if (bytes == 0){
    perror("reading: ");
    }
 //   char by[4];
   // sscanf(by,"%d",bytes);
    //write(STDOUT_FILENO,by,4);
    if (bytes < 0)
        perror("reading: ");
    //writing to server
    //for empty line
    if (strcmp(s,"\n") == 0)
        write(sock,"shdg",4);
    write(sock,s,bytes-1);

}
}

void *readFromServer(void *ptr)
{
    //reading
while(true){
    fflush(stdout);
    int b = read(sock, ret,10000);
    ret[b] = '\0';
    if (strcmp(ret,"exit")==0)
        exit(EXIT_SUCCESS);
    write(STDOUT_FILENO,ret,b);
}
}
