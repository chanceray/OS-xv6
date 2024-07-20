#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if(argc <= 1){
    printf("缺少参数\n");
    exit(0);
}
else{
char *p=argv[1];
while(*p!='\0'){
if (*p>'9'||*p<'0'){
printf("无效参数\n");
exit(1);
}
p++;
}
if(argc>2){
printf("参数过多\n");
}
sleep(atoi(argv[1]));
}
exit(0);
}