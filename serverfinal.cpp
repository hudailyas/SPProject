#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <cstring>
#include <time.h>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>

using namespace std;


class proc
{
  public:
  int pid;
  char name[400];
  time_t start;
  time_t finish;
  unsigned int elapsed;
  bool active;

  proc(int prpid, char n[400],time_t s){
    pid = prpid;
    strcpy(name,n);
    start = s;
    finish = NULL;
    int elapsed;
    active = true;


  }
  proc(){

  }
};
class clientInfo{
public:
char id[5];
char ip[500];
int port;
int csock;

clientInfo(char cid[5],char cip[500], int cport,int clientsock){
    strcpy(id,cid);
    strcpy(ip,cip);
    port = cport;
    csock = clientsock;

}
};
int addresult = 0;
int subresult = 0;
int multresult = 0;
int divresult = 0;
int i = 1;
int msgsock;

int add(char * str);
int sub(char * str);
int multiply(char * str);
int lengthResult(int k);

void *acceptThread(void* message);
void *commandThread(void* message);

vector <proc>activeProc;
vector <proc>allProc;
vector <clientInfo> connections;


static void handler(int signo)
{
    int status;
    if (signo == SIGCHLD){
    int pid = waitpid(0,&status,WNOHANG);
    if (pid > 0)
    {
        for (int ind = 0;ind<activeProc.size();ind++)
            {
                if (activeProc[ind].pid == pid)
                    {
                        activeProc.erase(activeProc.begin()+ind);
                        break;
                    }
            }

        for (int inde = 0;inde<allProc.size();inde++)
            {
                if (allProc[inde].pid == pid)
                    {
                        allProc[inde].finish = time(0);
                        allProc[inde].elapsed = difftime(time(0),allProc[inde].start);
                        allProc[inde].active = false;
                        break;
                    }
            }
        char * delsock;
        int fd = open("deleteClient",O_RDONLY,S_IRWXU);
        int by = lseek(fd,0,SEEK_END);
        char reading[by];
        lseek(fd,-by,SEEK_END);
        if (read(fd,reading,by) < 0)
            perror("reading ");
        char* str = strtok(reading," ");
        char tocheck[10];
        sprintf(tocheck,"%d",pid);
        while(str != NULL)
        {
            if (strcmp(str,tocheck) == 0)
            {
                delsock = strtok(NULL," ");
                break;

            }
            str = strtok(NULL," ");
        }
        sscanf(delsock,"%d",&pid);
        for (int index = 0;index < connections.size();index++)
        {
            if (connections[index].csock == pid)
            {
                connections.erase(connections.begin()+index);
                break;
            }
        }

    }

    }

}

int sock;
char s[10000];
socklen_t len;
struct sockaddr_in addr;
int main()
{

    int length;


    struct sockaddr_in server;



    signal(SIGCHLD,handler);


   sock = socket(AF_INET,SOCK_STREAM,0);
   if (sock < 0)
   {
    perror("socket failed");
    exit(1);
   }
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_family= AF_INET;
   server.sin_port = ntohs(1080);
   length = sizeof(server);

   if (bind(sock,(struct sockaddr*)&server,length) == -1)
   {
    perror("bind failed");
    exit(1);
   }

   if(getsockname(sock,(struct sockaddr*)&server,(socklen_t*)&length) == -1)
   {
    perror("socket name not found");
    exit(1);
   }
   char f[25];
   sprintf(f,"Socket has port #%d\n",ntohs(server.sin_port));
   write(STDOUT_FILENO,f,strlen(f));
   fflush(stdout);
   pthread_t athread, cthread;
   char* message;

   listen(sock,3);

        //if(msgsock = accept(sock,&clientadd,(socklen_t*)&len_add) == -1)
          //  perror("accept failed");
            //cout<<clientadd.sa_data;

            pthread_create(&athread,NULL,acceptThread,(void*)message);
            pthread_create(&cthread,NULL,commandThread,(void*)message);



            //write(STDOUT_FILENO,"hi",2);

            //write(STDOUT_FILENO,connections[0].ip,strlen(newclient.ip));

             pthread_join(athread,NULL);
            pthread_join(cthread,NULL);




close(msgsock);

}

int add(char * str)
{
    int st = atoi(str);
    addresult = addresult + st;
    return addresult;
}

int sub(char * str)
{
    int st;
    sscanf(str,"%d",&st);
    subresult = subresult-st;
    return subresult;
}

int multiply(char * str)
{
    int st = atoi(str);
    multresult = multresult * st;
    return multresult;
}

int lengthResult(int k)
{
  int len = 1;
  int cresult = k;
  if (cresult > 0)
  {
    for (len = 0;cresult >0;len++)
    {
        cresult = cresult/10;
    }
  }
if (k<0)
    return len + 1;
else
    return len;
}

void *acceptThread(void* ptr)
{
        int ret;
        char buff[1024];
        int fd = open("deleteClient",O_RDWR|O_TRUNC,S_IRWXU);
        do{
        msgsock = accept(sock,0,0);
         if (msgsock < 0)
            {
                perror("not accepted");
                exit(1);
            }

            len = sizeof(addr);
            if (getpeername(msgsock, (struct sockaddr*)&addr, &len) < 0)
                perror("client addr");
            char cid[5];
            sprintf(cid,"C%d",i);
            i = i + 1;
            clientInfo newclient = clientInfo(cid,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),msgsock);
            connections.push_back(newclient);



            int pid = fork();
            if (pid > 0)
            {


            }
            if (pid == 0){
            do
            {
                bzero(buff,sizeof(buff));
                ret = read(msgsock,buff,sizeof(buff));
                if (ret < 0)
                {
                    perror("read error");
                }
                else if (ret == 0)
                {
                    perror("ending connection");
                }
                else
                {
                    buff[ret] = '\0';
                    char * str = strtok(buff," ");

                    //RUNNING A PROCESS
                    if(strcmp(str,"run")==0)
                    {
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        int execfd[2];
                        pipe2(execfd,O_CLOEXEC);
                        int npid = fork();
                        if (npid == 0)
                        {

                            close(execfd[0]);
                            int execs = execlp(str,str,NULL);
                            if (execs == -1)
                            {
                                write(execfd[1],"Failed\n",8);
                            }
                        }
                        else if (npid > 0)
                        {

                            close(execfd[1]);
                            time_t st = time(0);
                            proc npro = proc(npid,str,st);
                            char e[20];
                            int byte = read(execfd[0],e,7);
                            if (byte ==  0 ){
                            write(msgsock,"running\n",9);
                            activeProc.push_back(npro);
                            allProc.push_back(npro);
                            }
                            else if (byte > 0)
                            {
                              write(msgsock,e,7);
                            }
                        }
                    }
                    }
                    //LISTING ACTIVE PROCESSES
                    else if(strcmp(str,"list") == 0)
                    {
                    char listprint[3000] = "Active Processes:\n";
                    char processname[25];
                    char starttime[25];
                    char endtime[25];
                    char processid[35];
                    char active[20];
                    char timeelapsed[30];

                        for (int a = 0;a<activeProc.size();a++)
                        {

                            int pn = sprintf(processname,"Process Name: %s\n",activeProc[a].name);
                            strcat(listprint,processname);
                            int pi = sprintf(processid,"Process ID: %d\n",activeProc[a].pid);
                            strcat(listprint,processid);
                            int st = sprintf(starttime,"Start Time: %s",ctime(&activeProc[a].start));
                            strcat(listprint,starttime);
                            int elap = difftime(time(0),activeProc[a].start);
                            int te = sprintf(timeelapsed,"Elapsed Time: %d s\n",elap);
                            strcat(listprint,timeelapsed);
                            strcat(listprint,"\n");
                        }

                        write(msgsock,listprint,strlen(listprint));


                    }
                    //LISTING ALL PROCESSES
                    else if(strcmp(str,"listall") == 0)
                    {
                        char listprint[3000] = "All Processes:\n\n";
                        char processname[25];
                        char starttime[25];
                        char endtime[25];
                        char processid[35];
                        char active[20];
                        char timeelapsed[30];


                        for (int a = 0;a<allProc.size();a++)
                        {

                            int pn = sprintf(processname,"Process Name: %s\n",allProc[a].name);
                            strcat(listprint,processname);
                            int pi = sprintf(processid,"Process ID: %d\n",allProc[a].pid);
                            strcat(listprint,processid);
                            int st = sprintf(starttime,"Start Time: %s",ctime(&allProc[a].start));
                            strcat(listprint,starttime);
                            if (allProc[a].active)
                            {
                                int et = sprintf(endtime,"End Time: null\n");
                                strcat(listprint,endtime);
                                int elap = difftime(time(0),allProc[a].start);
                                int te = sprintf(timeelapsed,"Elapsed Time: %d s\n",elap);
                                strcat(listprint,timeelapsed);
                                sprintf(active,"Active: true\n");
                                strcat(listprint,active);

                            }
                            else
                            {
                                int et = sprintf(endtime,"End Time: %s",ctime(&allProc[a].finish));
                                strcat(listprint,endtime);
                                int te = sprintf(timeelapsed,"Elapsed Time: %d s\n",allProc[a].elapsed);
                                strcat(listprint,timeelapsed);
                                sprintf(active,"Active: false\n");
                                strcat(listprint,active);
                            }
                            strcat(listprint,"\n");
                        }
                       write(msgsock,listprint,strlen(listprint));


                    }
                    //EXIT CLIENT
                    else if(strcmp(str,"exit")==0)
                    {
                        char forfile[10];
                        char cpid[5];
                        char msgs[5];
                        sprintf(cpid,"%d ",getpid());
                        strcat(forfile,cpid);
                        sprintf(msgs,"%d ",msgsock);
                        strcat(forfile,msgs);
                        write(fd,forfile,strlen(forfile));
                        write(msgsock,"exit",4);
                        exit(EXIT_SUCCESS);
                    }
                    //KILL PROCESS
                    else if(strcmp(str,"kill")==0)
                    {
                        bool found = NULL;
                        int pidk;
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        if (str[0] >= '0' && str[0]<='9'){
                            sscanf(str,"%d",&pidk);
                            kill(pidk,SIGTERM);
                            write(msgsock,"program killed\n",16);
                            for (int ind = 0;ind<activeProc.size();ind++)
                            {
                                if (activeProc[ind].pid == pidk)
                                {
                                    found = true;
                                    activeProc.erase(activeProc.begin()+ind);
                                    break;
                                }
                            }
                            if (found != true){
                                write(msgsock,"failed\n",8);

                            }
                            for (int inde = 0;inde<allProc.size();inde++)
                            {
                                if (allProc[inde].pid == pidk)
                                {
                                    allProc[inde].finish = time(0);
                                    allProc[inde].elapsed = difftime((0),allProc[inde].start);
                                    allProc[inde].active = false;
                                    break;
                                }
                            }


                        }
                        else
                        {
                            for(int ind = 0;ind<activeProc.size();ind++)
                            {
                              if(strcmp(activeProc[ind].name,str) == 0)
                              {
                                found = true;
                                kill(activeProc[ind].pid,SIGTERM);
                                activeProc.erase(activeProc.begin()+ind);
                                write(msgsock,"program killed\n",16);
                                break;
                              }
                            }
                            if (found != true){
                                write(msgsock,"failed\n",8);
                            }
                            for (int inde = 0;inde<allProc.size();inde++)
                            {
                                if (strcmp(allProc[inde].name,str)==0)
                                {
                                    allProc[inde].finish = time(0);
                                    allProc[inde].elapsed = difftime(time(0),allProc[inde].start);
                                    allProc[inde].active = false;
                                    break;
                                }
                            }

                        }
                    }
                    }
                    //ADDITION
                    else if(strcmp(str,"add")==0)
                    {
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        while(str!= NULL)
                        {
                            addresult = add(str);
                            str = strtok(NULL," ");
                        }

                        int len = lengthResult(addresult);
                        char w[len];
                        sprintf(w,"Ans of addition = %d\n",addresult);
                        addresult = 0;
                        write(msgsock,w,len+20);

                    }
                    }
                    //SUBTRACTION
                    else if (strcmp(str,"sub")==0)
                    {
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        subresult = atoi(str);
                        str = strtok(NULL," ");


                        while(str!= NULL)
                        {
                            subresult = sub(str);
                            str = strtok(NULL," ");

                        }

                        int len = lengthResult(subresult);
                        char w[len];
                        sprintf(w,"Ans of subtraction = %d\n",subresult);
                        write(msgsock,w,len+23);

                    }
                    }
                    //MULTIPLICATION
                    else if (strcmp(str,"multiply")==0)
                    {
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        multresult = atoi(str);
                        str = strtok(NULL," ");
                        while(str!= NULL)
                        {
                            multresult = multiply(str);
                            str = strtok(NULL," ");

                        }

                        int len = lengthResult(multresult);
                        char w[len];
                        sprintf(w,"Ans of multiplication = %d\n",multresult);
                        write(msgsock,w,len + 26);

                    }
                    }
                    //DIVISION
                    else if (strcmp(str,"divide")==0)
                    {
                        str = strtok(NULL," ");
                        if (str == NULL)
                        {
                            write(msgsock,"INCOMPLETE COMMAND\n",20);
                        }
                        else{
                        divresult = atoi(str);
                        str = strtok(NULL," ");

                        while(str!= NULL){

                            int st = atoi(str);

                            if (st==0){
                                divresult = NULL;
                                write(msgsock,"Division  by zero not possible\n",32);
                                }

                            else
                            {
                                divresult = divresult / st;
                                str = strtok(NULL," ");
                            }

                            }

                        if (divresult != NULL){
                        int len = lengthResult(divresult);
                        char w[len];
                        sprintf(w,"Ans of division = %d\n",divresult);
                        cout<<"hello";
                        cout.flush();
                        write(msgsock,w,len + 21);
                    }}
                    }
                    //INVALID COMMAND
                    else
                    {
                    write(msgsock,"INVALID COMMAND\n",16);
                    }

                }
                }while(ret != 0);



}
}while(true);
}

void *commandThread(void* message)
{
 while(true){
     if (write(STDOUT_FILENO,"Please enter your command\n",27) < 0)
                    perror("prompt error: ");
                int bytes = read(STDIN_FILENO,s,sizeof(s));
                if (bytes == 0)
                    perror("reading: ");
               // write(STDOUT_FILENO,s,bytes);


               s[bytes-1] = '\0';

                if (strcmp("CONN LIST",s) == 0)
                {
                    char clientid[20];
                    char clientip[20];
                    char clientsocket[20];
                    char clientport[20];
                    char connectionList[3000] = "Connected clients are\n\n";
                    for (int index = 0;index<connections.size();index++)
                    {
                        sprintf(clientid,"Client ID is: %s\n",connections[index].id);
                        strcat(connectionList,clientid);
                        sprintf(clientip,"Client IP is: %s\n",connections[index].ip);
                        strcat(connectionList,clientip);
                        sprintf(clientport,"Client Port is: %d\n",connections[index].port);
                        strcat(connectionList,clientport);
                        sprintf(clientsocket,"Client Socket fd is: %d\n",connections[index].csock);
                        strcat(connectionList,clientsocket);
                        strcat(connectionList,"\n");



                    }
                    write(STDOUT_FILENO,connectionList,strlen(connectionList));
                }

}
}

