#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "pstat.h"

extern struct proc proc[NPROC];

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
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

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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

uint64
sys_getprocs(void)
{
int count = 0;
struct proc *p;

for(p = proc; p< &proc[NPROC]; p++){
if(p->state != UNUSED)
count++;
}
return count;
}

#include "qlock.h"

struct qlock testlock;
int qlock_initialized = 0;

uint64
sys_qtest(void)
{
 if(qlock_initialized == 0){
  initqlock(&testlock, "testlock");
  qlock_initialized = 1;
}

printf("Trying to acuire Qlock. . .\n");

for(int i = 0; i < 10000000; i++){

}

printf("Releasing QLock . . .\n");
releaseq(&testlock);

printf("QLock released. Resource is unlocked.\n");

return 0;

}

#include "semaphore.h"

struct semaphore sem;
int shared = 0;

uint64
sys_semtest(void)
{
  sem_init(&sem, 1);

  printf("Process attempting to enter critical section\n");

  sem_wait(&sem);

  printf("Inside critical section\n");

  shared++;

  printf("Shared value = %d\n", shared);

  printf("Leaving critical section\n");

  sem_signal(&sem);

  return 0;
}

uint64
sys_settickets(void)
{
  int n;

  argint(0, &n);

  if(n < 1)
    return -1;

  struct proc *p = myproc();
  p->tickets = n;

  return 0;
}

uint64
sys_getpinfo(void)
{
  uint64 addr;
  struct pstat ps;
  struct proc *p = myproc();

  argaddr(0, &addr);

  for(int i = 0; i < NPROC; i++) {
   ps.inuse[i] = 0;
   ps.tickets[i] = 0;
   ps.pid[i] = 0;
   ps.ticks[i] = 0;
  }

 for(int i = 0; i < NPROC; i++) {
  struct proc *pp = &proc[i];

  acquire(&pp->lock);

  if(pp->state != UNUSED) {
    ps.inuse[i] = 1;
    ps.tickets[i] = pp->tickets;
    ps.pid[i] = pp->pid;
    ps.ticks[i] = pp->ticks;
}

  release(&pp->lock);
}

if(copyout(p->pagetable, addr, (char *)&ps, sizeof(ps)) < 0)
   return -1;

return 0;
}
