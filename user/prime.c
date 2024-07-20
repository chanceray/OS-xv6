#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"


#define MAX_MAX 36
#define ONE '1'
#define ZERO '0' 

void prime(int _read,int _write){

   char buff[MAX_MAX];
   int index=0;
   read(_read,buff,MAX_MAX);
   for(int i=0;i<MAX_MAX;i++)
   {
if(buff[i]==ONE)
{
index=i;
    break;
}}
if(index==0)
exit(0);
fprintf(1,"prime: %d\n",index);

   for(int i=0;i<MAX_MAX;i++)
   {
if(i%index==0)
buff[i]=ZERO;
}
  int pid=fork();  
if(pid==0)
 write(_write,buff,MAX_MAX);
     else{
     prime(_read,_write);

   }
}

int main(int argc, char *argv[])
{
   int fd[2];
 
   char buff[MAX_MAX];
   pipe(fd);
    int pid=fork();  
    //初始化
    for(int i=0;i<MAX_MAX;i++)
    {
    buff[i]=ONE;
    
    }
    buff[0]=ZERO;
    buff[1]=ZERO;
  
   if(pid>0)//父进程
   {
   
    write(fd[1],buff,MAX_MAX);

   }
   else{
    prime(fd[0],fd[1]);

   }

return 0;
}