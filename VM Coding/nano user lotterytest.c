#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/pstat.h"
#include "kernel/param.h"
#include "user/user.h"

int
main()
{
  int pid1, pid2, pid3;
  struct pstat ps;

  pid1 = fork();

  if(pid1 == 0){
    settickets(30);

    int count = 0;

    while(1){
      count++;

      if(count % 10000000 == 0){
        printf("PID1 running\n");
      }
    }
  }

  pid2 = fork();

  if(pid2 == 0){
    settickets(20);

    int count = 0;

    while(1){
      count++;

      if(count % 10000000 == 0){
        printf("PID2 running\n");
      }
    }
  }

  pid3 = fork();

  if(pid3 == 0){
    settickets(10);

    int count = 0;

    while(1){
      count++;

      if(count % 10000000 == 0){
        printf("PID3 running\n");
      }
    }
  }

  for(volatile int i = 0; i < 500000000; i++){
    ;
  }

  getpinfo(&ps);

  printf("PID\tTickets\tTicks\n");

  for(int i = 0; i < NPROC; i++){
    if(ps.inuse[i]){
      printf("%d\t%d\t%d\n",
        ps.pid[i],
        ps.tickets[i],
        ps.ticks[i]);
    }
  }

  kill(pid1);
  kill(pid2);
  kill(pid3);

  wait(0);
  wait(0);
  wait(0);

  exit(0);
}
