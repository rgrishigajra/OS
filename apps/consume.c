#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>
int n;

void consumer(int count) {
    int i= 0;
   for (i = 0; i < count; i++) {
    printf("\nconsumed : %d ", n);
}
}

void consumer_bb(int count) {
    // Iterate from 0 to count and for each iteration read the next available value from the global array `arr_q`
  // print consumer process name and read value as,
  // name : consumer_1, read : 8
    int32 i;
    int32 j;
    char *process_name =proctab[getpid()].prname;
   for (i = 0; i < count; i++) {
       wait(prodmut);
        wait(mutex);
        j= arr_q[tail];
        tail= (tail+1)%BUFFSIZE;
        printf("name: %s, read:%d\n",process_name,j);
        signal(mutex);
        signal(conmut);
}
}