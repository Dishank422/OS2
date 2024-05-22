#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_mydate(void)
{
    struct rtcdate * r;
    if(argptr(0, (void *)&r, 2*sizeof(r[0]))<0) return -1;
    cmostime(r);
    return 0;
}

int
sys_pgtprint(void)
{
    uint i, j;
    pde_t * pde, * pgdir;
    pgdir = myproc()->pgdir;

    for(i = 0; i<NPDENTRIES; i += 1){
        if(!(pgdir[i] & PTE_P)) continue;
        pde = (pde_t *)P2V(PTE_ADDR(pgdir[i]));
        for(j = 0; j<NPTENTRIES; j++){
            if(!((pde[j] & PTE_P) && (pde[j] & PTE_U))) continue;
            cprintf("Page number: %d, Virtual address: %p, Physical address: %p, W-bit: %d\n", i*NPTENTRIES+j, (i*PGSIZE*NPTENTRIES+j*PGSIZE),
                    PTE_ADDR(pde[j]), (pde[j]&PTE_W)>>1);
        }
    }
    return 0;
}
