
/* xsh_run.c - xsh_run */
#include <xinu.h>
#include <string.h>
#include <stdio.h>
#include <prodcons_bb.h>
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * xsh_run - //
 *------------------------------------------------------------------------
 */

// definition of array, semaphores and indices
sid32 mutex,prodmut,conmut;
int head=0;
int tail=0;
int arr_q[5];
void prodcons_bb(int nargs, char *args[])
{   mutex=semcreate(1);
prodmut=semcreate(0);
conmut=semcreate(5);
   int i=0;
   char proc_name[10];
  int producer_count=atoi(args[1]);
  int consumer_count=atoi(args[2]);
  int producer_iterations=atoi(args[3]);
  int consumer_iterations=atoi(args[4]);
  if(producer_count * producer_iterations != consumer_iterations*consumer_count){
      return;
      } 
else{
  for (i = 0; i < producer_count; i++)
  { sprintf(proc_name,"producer_%d",i);
    resume(create((void *)producer_bb, 4096, 20,proc_name, 1, producer_iterations));
  }
  for (i = 0; i < consumer_count; i++)
  {sprintf(proc_name,"consumer_%d",i);
    resume(create((void *)consumer_bb, 4096, 20,proc_name, 1, consumer_iterations));
  }
  //create and initialize semaphores to necessary values

  //initialize read and write indices for the queue

  //create producer and consumer processes and put them in ready queue

  return ;
}}
shellcmd xsh_run(int nargs, char *args[])
{  
  /* Output info for '--help' argument */
  if ((nargs == 1) || (strncmp(args[1], "list", 5) == 0))
  {
    printf("producer_bb\n");
    printf("consumer_bb\n");
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
}



