
#include<xinu.h>
#define BUFFSIZE 5;

// declare globally shared array
extern int32 arr_q[5];


// declare globally shared semaphores
sid32 mutex,prodmut,conmut;
// declare globally shared read and write indices
int32 head,tail;

// function prototypes
void consumer_bb(int count);
void producer_bb(int count);