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

//CLASSES
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
class clientInfo
{
    public:
    char id[5];
    char ip[500];
    int port;
    int csock;

    clientInfo(char cid[5],char cip[500], int cport,int clientsock)
    {
        strcpy(id,cid);
        strcpy(ip,cip);
        port = cport;
        csock = clientsock;

    }
};

//GLOBAL VARIABLES
int addresult = 0;
int subresult = 0;
int multresult = 0;
int divresult = 0;
int i = 1;
int msgsock;
int sock;
char s[10000];
char clientid[10];
int fdp;
socklen_t len;
struct sockaddr_in addr;
vector <proc>activeProc;
vector <proc>allProc;
vector <clientInfo> connections;

//METHODS
int add(char * str);
int sub(char * str);
int multiply(char * str);
int lengthResult(int k);
bool checkChar(char * str);
void *acceptThread(void* message);
void *commandThread(void* message);
void listingActive();

static void handler(int signo)
{
    int status;
    if (signo == SIGCHLD)
    {
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
            fdp = open(clientid,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
            listingActive();

        }
    }
    if (signo == SIGINT)
    {

        for (int index = 0;index < connections.size();index++)
        {
            remove(connections[index].id);

            if (write(connections[index].csock,"exit\n",6) < 0)
                perror("writing ");
        }
        exit(EXIT_SUCCESS);
    }
}
void handler2(int signo)
{
    if (signo == SIGCHLD)
    {
        int status;
        int pidw = waitpid(0,&status,WNOHANG);
        if (pidw > 0)
        {

            char * delsock;
            int so;
            int fd = open("deleteClient",O_RDONLY,S_IRWXU);
            int by = lseek(fd,0,SEEK_END);
            char reading[by];
            lseek(fd,-by,SEEK_END);
            if (read(fd,reading,by) < 0)
                perror("reading ");
            char* str = strtok(reading," ");
            char tocheck[10];
            sprintf(tocheck,"%d",pidw);
            while(str != NULL)
            {
                if (strcmp(str,tocheck) == 0)
                {
                    delsock = strtok(NULL," ");
                    break;

                }
                str = strtok(NULL," ");
            }
            sscanf(delsock,"%d",&so);
            for (int index = 0;index < connections.size();index++)
            {
                if (connections[index].csock == so)
                {
                    connections.erase(connections.begin()+index);
                    break;
                }
            }
        }
    }

}

int main()
{
   int length;
   struct sockaddr_in server;
   signal(SIGINT,handler);
   signal(SIGCHLD,handler2);
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

            if (pthread_create(&athread,NULL,acceptThread,(void*)message) >0)
                perror("thread creation ");
            if (pthread_create(&cthread,NULL,commandThread,(void*)message) > 0)
                perror("thread creation ");

            pthread_join(athread,NULL);
            pthread_join(cthread,NULL);




close(msgsock);

}

void *acceptThread(void* ptr)
{
    int ret;
    char id[45];
    char buff[1024];
    int fd = open("deleteClient",O_RDWR|O_TRUNC|O_CREAT,S_IRWXU);
    if (fd < 0)
    {
        perror("open ");
    }
    bool check;
    do
    {

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

        strcpy(clientid,newclient.id);
        fdp = open(clientid,O_RDWR|O_CREAT|O_TRUNC,S_IRWXU);
        sprintf(id, "Active Processes List for Client ID: %s\n\n",clientid);
        write(fdp,id,strlen(id));
        write(fdp,"No active processes yet\n\n",26);
        lseek(fdp,0,SEEK_SET);
            int pid = fork();
            if (pid == 0)
            {
                signal(SIGCHLD,handler);

                do
                {
                    //bzero(buff,sizeof(buff));
                    ret = read(msgsock,buff,sizeof(buff));
                    buff[ret] = '\0';
                    if (ret < 0)
                    {
                        perror("read error");
                    }
                    else if (ret == 0)
                    {

                        remove(clientid) == 0;

                        for (int index = 0;index<activeProc.size();index++)
                        {
                            kill(activeProc[index].pid,SIGTERM);
                        }
                        char forfile[10] = "";
                        char cpid[5];
                        char msgs[5];
                        sprintf(cpid,"%d ",getpid());
                        strcat(forfile,cpid);
                        sprintf(msgs,"%d ",msgsock);
                        strcat(forfile,msgs);
                        write(fd,forfile,strlen(forfile));
                        write(msgsock,"exit\n",6);
                        exit(EXIT_SUCCESS);

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
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else
                            {
                                int execfd[2];
                                pipe2(execfd,O_CLOEXEC);
                                int npid = fork();
                                if (npid == 0)
                                {

                                    close(execfd[0]);
                                    int execs = execlp(str,str,NULL);
                                    if (execs == -1)
                                    {
                                        if (write(execfd[1],"Failed\n\n",10) < 0)
                                            perror("writing ");
                                    }
                                }
                                else if (npid > 0)
                                {

                                    close(execfd[1]);
                                    time_t st = time(0);
                                    proc npro = proc(npid,str,st);
                                    char e[20];
                                    int byte = read(execfd[0],e,8);
                                    if (byte ==  0 ){
                                    write(msgsock,"running\n\n",9);
                                    activeProc.push_back(npro);
                                    allProc.push_back(npro);
                                    }
                                    else if (byte < 0){
                                        perror("reading ");
                                    }
                                    else if (byte > 0)
                                    {
                                      if (write(msgsock,"Failed\n\n",8) < 0)
                                        perror("writing ");
                                    }
                                }
                                else if (npid < 0){
                                    perror("fork ");
                                }
                            }
                        }
                        //LISTING ACTIVE PROCESSES
                        else if(strcmp(str,"list") == 0)
                        {
                        char listprint[3000] = "Active Processes:\n\n";
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
                                strcat(listprint,"\n\n");
                            }

                            if (write(msgsock,listprint,strlen(listprint)) < 0)
                                perror("writing ");


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
                                strcat(listprint,"\n\n");
                            }
                           if (write(msgsock,listprint,strlen(listprint)) < 0)
                                perror("writing");


                        }

                        //EXIT CLIENT
                        else if(strcmp(str,"exit")==0)
                        {
                            if (remove(clientid) != 0)
                                {
                                    perror("removing file");
                                }
                            for (int index = 0;index<activeProc.size();index++)
                            {
                                kill(activeProc[index].pid,SIGTERM);
                            }
                            char forfile[10] = "";
                            char cpid[5];
                            char msgs[5];
                            sprintf(cpid,"%d ",getpid());
                            strcat(forfile,cpid);
                            sprintf(msgs,"%d ",msgsock);
                            strcat(forfile,msgs);

                            //cout<<msgs;

                            //cout.flush();
                            if (write(fd,forfile,strlen(forfile)) < 0)
                                perror("writing ");
                            if (write(msgsock,"exit\n",6) < 0)
                                perror("writing ");
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
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else
                            {
                                if (str[0] >= '0' && str[0]<='9')
                                {
                                sscanf(str,"%d",&pidk);
                                kill(pidk,SIGTERM);
                                if (write(msgsock,"program killed\n\n",16) < 0)
                                    perror("writing ");
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
                                    if (write(msgsock,"Failed\n\n",9) < 0)
                                        perror("writing ");

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
                                        if (write(msgsock,"program killed\n\n",16) < 0)
                                            perror("writing ");
                                        break;
                                      }
                                    }
                                    if (found != true){
                                        if (write(msgsock,"Failed\n\n",9) < 0)
                                            perror("writing ");
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
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else
                            {
                                while(str!= NULL)
                                {
                                    check = checkChar(str);
                                    if (check)
                                        break;

                                    addresult = add(str);
                                    str = strtok(NULL," ");
                                }
                                if (!check)
                                {
                                int len = lengthResult(addresult);
                                char w[len];
                                sprintf(w,"Ans of addition = %d\n\n",addresult);
                                addresult = 0;
                                if (write(msgsock,w,len+20) < 0)
                                    perror("writing ");
                                }
                            }
                        }
                        //SUBTRACTION
                        else if (strcmp(str,"sub")==0)
                        {
                            str = strtok(NULL," ");
                            if (str == NULL)
                            {
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else if (!checkChar(str))
                            {
                                subresult = atoi(str);
                                str = strtok(NULL," ");
                                while(str!= NULL)
                                {
                                    check = checkChar(str);
                                    if (check)
                                        break;
                                    subresult = sub(str);
                                    str = strtok(NULL," ");

                                }
                                if (!check)
                                {
                                int len = lengthResult(subresult);
                                char w[len];
                                sprintf(w,"Ans of subtraction = %d\n\n",subresult);
                                if (write(msgsock,w,len+23) < 0)
                                    perror("writing ");
                                }
                            }
                        }
                        //MULTIPLICATION
                        else if (strcmp(str,"multiply")==0)
                        {
                            str = strtok(NULL," ");
                            if (str == NULL)
                            {
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else if (!checkChar(str))
                            {
                                multresult = atoi(str);
                                str = strtok(NULL," ");
                                while(str!= NULL)
                                {
                                    check = checkChar(str);
                                    if (check)
                                        break;
                                    multresult = multiply(str);
                                    str = strtok(NULL," ");

                                }
                                if (!check)
                                {
                                int len = lengthResult(multresult);
                                char w[len];
                                sprintf(w,"Ans of multiplication = %d\n\n",multresult);
                                if (write(msgsock,w,len + 26) < 0)
                                    perror("writing ");
                                }
                            }
                        }
                        //DIVISION
                        else if (strcmp(str,"divide")==0)
                        {
                            int st;
                            str = strtok(NULL," ");
                            if (str == NULL)
                            {
                                if (write(msgsock,"INCOMPLETE COMMAND\n\n",21) < 0)
                                    perror("writing ");
                            }
                            else if (!checkChar(str)){
                            divresult = atoi(str);
                            str = strtok(NULL," ");

                            while(str!= NULL){
                                check = checkChar(str);
                                if (check)
                                    break;
                                st = atoi(str);

                                if (st==0){
                                    //divresult = NULL;
                                    if (write(msgsock,"Division by zero not possible\n\n",32) < 0)
                                        perror("writing ");
                                    break;
                                    }

                                else
                                {
                                    divresult = divresult / st;
                                    str = strtok(NULL," ");
                                }

                                }

                            if (st != 0 && !check){
                            int len = lengthResult(divresult);
                            char w[len];
                            sprintf(w,"Ans of division = %d\n\n",divresult);
                            if (write(msgsock,w,len + 22) < 0)
                                perror("writing ");
                        }}
                        }
                        //INVALID COMMAND

                        else
                        {
                        if (write(msgsock,"INVALID COMMAND\n\n",18) < 0)
                            perror("writing ");
                        }

                    }

                    listingActive();
                }while(ret != 0);
            }
    }while(true);
}

void *commandThread(void* message)
{

     while(true)
     {
        if (write(STDOUT_FILENO,"Please enter your command\n",27) < 0)
            perror("prompt error: ");
        int bytes = read(STDIN_FILENO,s,sizeof(s));
        if (bytes == 0)
            perror("reading: ");
        if (bytes < 0)
            perror("reading ");
        if (strcmp(s,"\n") == 0)
        {
            if (write(STDOUT_FILENO,"INVALID COMMAND\n\n",18) < 0)
                perror("writing ");
        }
        else
        {
            s[bytes-1] = '\0';
            char* str = strtok(s," ");

            if (strcmp("conn",str) == 0 )
            {
                str = strtok(NULL," ");
                if(str == NULL)
                {
                    if (write(STDOUT_FILENO,"INCOMPLETE COMMAND\n\n",21) < 0)
                        perror("writing ");
                }
                else if(strcmp(str,"list") == 0)
                {
                    char clientid[20];
                    char clientip[20];
                    char clientsocket[20];
                    char clientport[20];
                    char connectionList[3000] = "Connected clients are:\n\n";
                    if (connections.size() == 0)
                    {
                        if (write(STDOUT_FILENO,"NO ACTIVE CONNECTIONS\n\n",23) < 0)
                            perror("writing ");
                    }
                    else
                    {
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
                            strcat(connectionList,"\n\n");
                        }
                            if (write(STDOUT_FILENO,connectionList,strlen(connectionList)) < 0)
                                perror("writing ");
                    }
                }
            }
                else if (strcmp("print",str) == 0)
                {
                    bool idfound = NULL;
                    str = strtok(NULL," ");
                    if (str == NULL)
                    {
                        if (write(STDOUT_FILENO,"INCOMPLETE COMMAND\n\n",21) < 0)
                            perror("writing ");
                    }
                        else
                        {
                            if (connections.size() == 0)
                            {
                                if (write(STDOUT_FILENO,"NO ACTIVE CONNECTIONS\n\n",23) < 0)
                                    perror("writing ");
                            }
                            else {
                            char toprint[1000] = "";
                            while (str != NULL)
                            {
                                for (int index = 0;index < connections.size();index++)
                                {
                                    if (strcmp(str,connections[index].id) == 0)
                                    {
                                        idfound = true;
                                        strcat(toprint,"\n\n");
                                        write(connections[index].csock,toprint,strlen(toprint));

                                        break;
                                    }
                                }
                                if (idfound != true)
                                    {
                                        strcat(toprint,str);
                                        strcat(toprint," ");
                                    }


                                    str = strtok(NULL," ");

                                }
                                if (idfound != true)
                                {
                                strcat(toprint,"\n\n");
                                  for (int index = 0;index < connections.size();index++)
                                        {
                                            write(connections[index].csock,toprint,strlen(toprint));

                                        }
                                }
                    }
                    }
                }
                    else if(strcmp("list",str) == 0)
                    {

                        if (connections.size() == 0){
                           if (write(STDOUT_FILENO,"NO ACTIVE CONNECTIONS\n\n",23) < 0)
                                perror("writing ");
                        }
                        else
                        {
                        for (int index = 0; index < connections.size();index++)
                        {

                            int rfd = open(connections[index].id,O_RDONLY,S_IRWXU);
                            int by = lseek(rfd,0,SEEK_END);
                            char reading[by];
                            lseek(rfd,-by,SEEK_END);
                            if (read(rfd,reading,by) < 0)
                                perror("reading ");
                            else
                                write(STDOUT_FILENO,reading,by);

                        }
                    }
                    }

                    else
                    {
                        if (write(STDOUT_FILENO,"INVALID COMMAND\n\n",18) < 0)
                            perror("writing ");
                    }
        }
    }
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

bool checkChar(char * str)
{
    if (!isdigit(*str))
    {
       if (write(msgsock,"Please insert only numbers for addition\n\n",41) < 0)
           perror("writing ");
       return true;
    }
    for (int i = 0;i<strlen(str);i++)
    {
        if (!isdigit(str[i]) && str[i]!= '\n' )
        {
            if (write(msgsock,"Please insert only numbers for addition\n\n",41) < 0)
                perror("writing ");
            return true;
        }
    }
return false;

}

void listingActive()
{

    char client[3000] = "Active Processes List for Client ID: ";
    char csockchar[5];
    strcat(client,clientid);
    strcat(client, "\n\n");
    char processname[25];
    char starttime[25];
    char endtime[25];
    char processid[35];
    char active[20];
    char timeelapsed[30];
    if (activeProc.size() == 0)
    {
        strcat(client,"No active processes yet\n\n");
    }
    for (int a = 0;a<activeProc.size();a++)
    {
        int pn = sprintf(processname,"Process Name: %s\n",activeProc[a].name);
        strcat(client,processname);
        int pi = sprintf(processid,"Process ID: %d\n",activeProc[a].pid);
        strcat(client,processid);
        int st = sprintf(starttime,"Start Time: %s",ctime(&activeProc[a].start));
        strcat(client,starttime);
        strcat(client,"\n\n");
    }

    if (write(fdp,client,strlen(client)) < 0)
        perror("writing ");
    lseek(fdp,0,SEEK_SET);
}

