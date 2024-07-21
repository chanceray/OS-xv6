#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{  uint64 pp;
  int n;
    int bitmask;

  argaddr(0, &pp);
  // lab pgtbl: your code here.

  argint(1, &n);
  if(n<0||n>100)
  return -2;
  //It's okay to set an upper limit on the number of pages that can be scanned.
  argint(2,&bitmask);
int res=0;
struct proc *p = myproc();
for(int i=0;i<n;i++)
{
uint64 va=pp+i*PGSIZE;//获得虚拟地址

 pte_t *pte;
  if(va >= MAXVA)
    return 0;

  pte = walk(p->pagetable, va, 0);
  if(pte == 0)
    return 0;
  if((*pte & PTE_A)){
    *pte=*pte&(~PTE_A);
    res=res|1<<i;
  }

}

  
if(copyout(p->pagetable, bitmask, (char *)&res, sizeof(res)) < 0)
      return -1;


  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
