#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/param.h"

#define MASSAGESIZE 256

int
main(int argc, char *argv[])
{
 char buf[MASSAGESIZE];
 read(0,buf,MASSAGESIZE);//获得标准化输入

int xargc=0;
char *xargv[MAXARG];
for(int i=1;i<argc;i++)
xargv[xargc++]=argv[i];
 char *p=buf;
 for(int i=0;i<MASSAGESIZE;i++)
 if(buf[i]=='\n'){
    int pid=fork();
    if (pid>0)//父
    {
p=&buf[i+1];
wait(0);
    }
    else//子
{

    buf[i]=0;
    xargv[xargc]=p;
    xargc++;
    xargv[xargc]=0;
    xargc++;
    exec(xargv[0],xargv);
    wait(0);
}
 }
exit(0);
}
