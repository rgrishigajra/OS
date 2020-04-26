
/* xsh_run.c - xsh_run */
#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <prodcons_bb.h>
#include <string.h>
#include <stdlib.h>
#include <future.h>
#include <future_fib.h>
#include <future_prodcons.h>
#include <stream.h>
/*------------------------------------------------------------------------
 * xsh_run - //
 *------------------------------------------------------------------------
 */

// definition of array, semaphores and indices
sid32 mutex, prodmut, conmut;
int head = 0;
int tail = 0;
int arr_q[5];
int one = 1;
int zero = 0;
int two = 2;
future_t **fibfut;

void waitFor(unsigned int wait_msecs)
{
  ulong secs, msecs;
  secs = clktime;
  msecs = clkticks;
  while (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs) < wait_msecs)
    ;
}
void futureq_test1(int nargs, char *args[])
{
  int three = 3, four = 4, five = 5, six = 6;
  future_t *f_queue;
  f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

  resume(create(future_cons, 1024, 20, "fcons6", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons7", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons8", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons9", 1, f_queue));
  resume(create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char *)&three));
  resume(create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char *)&four));
  resume(create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char *)&five));
  resume(create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char *)&six));
  waitFor(100);
}

void futureq_test2(int nargs, char *args[])
{
  int seven = 7, eight = 8, nine = 9, ten = 10, eleven = 11;
  future_t *f_queue;
  f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

  resume(create(future_prod, 1024, 20, "fprod10", 2, f_queue, (char *)&seven));
  resume(create(future_prod, 1024, 20, "fprod11", 2, f_queue, (char *)&eight));
  resume(create(future_prod, 1024, 20, "fprod12", 2, f_queue, (char *)&nine));
  resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&ten));
  resume(create(future_prod, 1024, 20, "fprod13", 2, f_queue, (char *)&eleven));

  resume(create(future_cons, 1024, 20, "fcons14", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons15", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons16", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons17", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons18", 1, f_queue));
  waitFor(100);
}

void futureq_test3(int nargs, char *args[])
{
  int three = 3, four = 4, five = 5, six = 6;
  future_t *f_queue;
  f_queue = future_alloc(FUTURE_QUEUE, sizeof(int), 3);

  resume(create(future_cons, 1024, 20, "fcons6", 1, f_queue));
  resume(create(future_prod, 1024, 20, "fprod3", 2, f_queue, (char *)&three));
  resume(create(future_prod, 1024, 20, "fprod4", 2, f_queue, (char *)&four));
  resume(create(future_prod, 1024, 20, "fprod5", 2, f_queue, (char *)&five));
  resume(create(future_prod, 1024, 20, "fprod6", 2, f_queue, (char *)&six));
  resume(create(future_cons, 1024, 20, "fcons7", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons8", 1, f_queue));
  resume(create(future_cons, 1024, 20, "fcons9", 1, f_queue));
  waitFor(100);
}
void prodcons_bb(int nargs, char *args[])
{
  mutex = semcreate(1);
  prodmut = semcreate(0);
  conmut = semcreate(5);
  int i = 0;
  char proc_name[10];
  int producer_count = atoi(args[1]);
  int consumer_count = atoi(args[2]);
  int producer_iterations = atoi(args[3]);
  int consumer_iterations = atoi(args[4]);
  if (producer_count * producer_iterations != consumer_iterations * consumer_count)
  {
    return;
  }
  else
  {
    for (i = 0; i < producer_count; i++)
    {
      sprintf(proc_name, "producer_%d", i);
      resume(create((void *)producer_bb, 4096, 20, proc_name, 1, producer_iterations));
    }
    for (i = 0; i < consumer_count; i++)
    {
      sprintf(proc_name, "consumer_%d", i);
      resume(create((void *)consumer_bb, 4096, 20, proc_name, 1, consumer_iterations));
    }
    //create and initialize semaphores to necessary values

    //initialize read and write indices for the queue

    //create producer and consumer processes and put them in ready queue

    return;
  }
}

shellcmd xsh_run(int nargs, char *args[])
{

  /* Output info for '--help' argument */
  if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
  {
    printf("prodcons_bb\n");
    printf("futures_test\n");
    printf("tscdf\n");
    return OK;
  }
  /* This will go past "run" and pass the function/process name and its
    * arguments.
    */
  args++;
  nargs--;

  if (strncmp(args[0], "prodcons_bb", 11) == 0)
  {
    /* create a process with the function as an entry point. */
    resume(create((void *)prodcons_bb, 4096, 20, "prodcons_bb", 2, nargs, args));
  }
  if ((strncmp(args[0], "futures_test", 12) == 0) && (strncmp(args[1], "-pc", 3) == 0))
  {
    /* create a process with the function as an entry point. */
    // printf("pc");
    future_t *f_exclusive, *f_shared;
    f_exclusive = future_alloc(FUTURE_EXCLUSIVE, sizeof(int), 1);
    f_shared = future_alloc(FUTURE_SHARED, sizeof(int), 1);

    // Test FUTURE_EXCLUSIVE
    resume(create(future_cons, 1024, 20, "fcons1", 1, f_exclusive));
    resume(create(future_prod, 1024, 20, "fprod1", 2, f_exclusive, (char *)&one));

    // Test FUTURE_SHARED
    resume(create(future_cons, 1024, 20, "fcons2", 1, f_shared));
    resume(create(future_cons, 1024, 20, "fcons3", 1, f_shared));
    resume(create(future_cons, 1024, 20, "fcons4", 1, f_shared));
    resume(create(future_cons, 1024, 20, "fcons5", 1, f_shared));
    resume(create(future_prod, 1024, 20, "fprod2", 2, f_shared, (char *)&two));
  }
  if ((strncmp(args[0], "futures_test", 12) == 0) && (strncmp(args[1], "-fq", 3) == 0))
  {
    if ((strncmp(args[1], "-fq1", 4) == 0))
    {
      resume(create(futureq_test1, 1024, 20, "futureq_test1", 2, nargs, args));
      return 0;
    }
    if ((strncmp(args[1], "-fq2", 4) == 0))
    {
      resume(create(futureq_test2, 1024, 20, "futureq_test2", 2, nargs, args));
      return 0;
    }
    if ((strncmp(args[1], "-fq3", 4) == 0))
    {
      resume(create(futureq_test3, 1024, 20, "futureq_test3", 2, nargs, args));
      return 0;
    }
  }
  else if ((strncmp(args[0], "futures_test", 12) == 0) && (strncmp(args[1], "-f", 2) == 0))
  {

    int fib = -1, i;

    fib = atoi(args[2]);
    if (fib > -1)
    {
      int final_fib;
      int future_flags = FUTURE_SHARED; // TODO - add appropriate future mode here

      // create the array of future pointers
      if ((fibfut = (future_t **)getmem(sizeof(future_t *) * (fib + 1))) == (future_t **)SYSERR)
      {
        printf("getmem failed\n");
        return (SYSERR);
      }

      // get futures for the future array
      for (i = 0; i <= fib; i++)
      {
        if ((fibfut[i] = future_alloc(future_flags, sizeof(int), 1)) == (future_t *)SYSERR)
        {
          printf("future_alloc failed\n");
          return (SYSERR);
        }
      }

      // spawn fib threads and get final value
      // TODO - you need to add your code here
      for (i = 0; i <= fib; i++)
      {
        resume(create(ffib, 1024, 20, "ffib", 1, i));
      }

      future_get(fibfut[fib], (char *)&final_fib);

      for (i = 0; i <= fib; i++)
      {
        future_free(fibfut[i]);
      }

      freemem((char *)fibfut, sizeof(future_t *) * (fib + 1));
      printf("Nth Fibonacci value for N=%d is %d\n", fib, final_fib);
      return (OK);
    }
  }
  if (strncmp(args[0], "tscdf_fq", 7) == 0)
  {
    resume(create((void *)stream_proc_futures, 4096, 20, "stream_proc_futures", 2, nargs, args));
  }
  else if (strncmp(args[0], "tscdf", 5) == 0)
  {
    resume(create((void *)stream_proc, 4096, 20, "stream_proc", 2, nargs, args));
  }
}
