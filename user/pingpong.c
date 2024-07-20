#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"


#define MAX_MAX 16

int main(int argc, char *argv[])
{
   int fd[2];
   int fe[2];
   char buff[MAX_MAX];
   pipe(fd);
   pipe(fe);

    int pid=fork();
   if(pid>0)//父进程
   {
   int p_pid = getpid();

    write(fd[1],"ping",MAX_MAX);
   
    read(fe[0],buff,MAX_MAX);
    fprintf(1,"%d:收到了%s\n",p_pid,buff);
   }
else//子进程
{
int p_pid = getpid();
 
 read(fd[0],buff,MAX_MAX);
  fprintf(1,"%d:收到了%s\n",p_pid,buff);
 write(fe[1],"pong",MAX_MAX);
    
    

}return 0;
}/*
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int
main(int argc,char *argv[]){
    if(argc!=1){
        exit(1);
    }
    int p[2],c[2];
    if(pipe(p)==-1||pipe(c)==-1){
       exit(2);
    }
    int pid = fork();
    if(pid<0){
        exit(3);
    }
    else if(pid>0){
        int p_pid = getpid();
        char msg[1];
        write(p[1],"H",1);
        read(c[0],msg,1);
        fprintf(1,"%d: received pong\n",p_pid);
    }
    else{
        int c_pid = getpid();
        char msg[1];
        read(p[0],msg,1);
        fprintf(1,"%d: received ping\n",c_pid);
        write(c[1],"H",1);
    }
    close(p[0]);
    close(p[1]);
    close(c[0]);
    close(c[1]);
    exit(0);
}*/