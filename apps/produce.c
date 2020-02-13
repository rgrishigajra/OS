#include <xinu.h>
#include <prodcons.h>
#include <prodcons_bb.h>

void producer(int count) {
    int i;
    for (i = 0; i < count; i++) {
    n =i;    
    printf("\nproduced : %d ", n);
}
}
void producer_bb(int count) {
     // Iterate from 0 to count and for each iteration add iteration value to the global array `arr_q`, 
  // print producer process name and written value as,
  // name : producer_1, write : 8
    int i;
    char *process_name =proctab[getpid()].prname;
    for (i = 0; i < count; i++) {
        wait(conmut);
        wait(mutex);
        arr_q[head] = i;
        head= (head+1)%BUFFSIZE;
        printf("name: %s, write :%d\n",process_name,i);
        signal(mutex);
        signal(prodmut);
    }


 

    
}
  