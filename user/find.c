#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p),0, DIRSIZ-strlen(p));//改为0，不填充！！
  return buf;
}

void
find(char *path,char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

if(strcmp(target,fmtname(path))==0)
printf("%s\n",path);
  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
     
char *a=fmtname(buf); 
   if(!((a[0]=='.'&&a[1]=='.'&&a[2]==0)||(a[0]=='.'&&a[1]==0)))
   find(buf,target);
   
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc ==1 ){
   printf("输入参数过少");
    exit(0);
  }
  if(argc == 2){
   find(".",argv[1]);
    exit(0);
  }
  if(argc == 3){
   find(argv[1],argv[2]);
    exit(0);
  }
 
  exit(0);
}
