#include <xinu.h>
#include <stream.h>
#include "tscdf.h"
#include <future.h>
int num_streams, work_queue_depth, time_window, output_time;
uint pcport;
future_t **farray;
void stream_consumer_futures(int32 id, future_t *f)
{
    int count = 0;

    struct tscdf *tc = tscdf_init(time_window);
    kprintf("stream_consumer_future id:%d (pid:%d)\n", id, getpid());
    while (TRUE)
    {
        de *out = (de *)getmem(sizeof(de));
        future_get(f, out);
        if (out->time == 0 && out->value == 0)
        {
            break;
        }
        //tscdf code
        tscdf_update(tc, out->time, out->value);
        if (count++ == output_time - 1)
        {
            char output[10];
            int32 *qarray = tscdf_quartiles(tc);
            if (qarray == NULL)
            {
                kprintf("tscdf_quartiles returned NULL\n");
                continue;
            }
            sprintf(output, "s%d: %d %d %d %d %d", id, qarray[0], qarray[1], qarray[2], qarray[3], qarray[4]);
            kprintf("%s\n", output);
            freemem((char *)qarray, (6 * sizeof(int32)));
            count = 0;
        }
    }
    ptsend(pcport, getpid());
    return;
}

int stream_proc_futures(int nargs, char *args[])
{
    int future_flags = FUTURE_QUEUE, i;
    // create the array of future pointers

    // Parse arguments
    char usage[] = "Usage: -s num_streams -w work_queue_depth -t time_window -o output_time\n";
    char *ch;
    char c;

    ulong secs, msecs, time;
    secs = clktime;
    msecs = clkticks;
    /* Parse arguments out of flags */
    /* if not even # args, print error and exit */
    if (!(nargs % 2))
    {
        printf("%s", usage);
        return (-1);
    }
    else
    {
        if ((pcport = ptcreate(num_streams)) == SYSERR)
        {
            printf("ptcreate failed:\n");
        }
        i = nargs - 1;
        while (i > 0)
        {
            ch = args[i - 1];
            c = *(++ch);

            switch (c)
            {
            case 's':
                num_streams = atoi(args[i]);
                break;

            case 'w':
                work_queue_depth = atoi(args[i]);
                break;

            case 't':
                time_window = atoi(args[i]);
                break;

            case 'o':
                output_time = atoi(args[i]);
                break;

            default:
                printf("%s", usage);
                return (-1);
            }

            i -= 2;
        }
    }
    uint size = sizeof(de) * work_queue_depth;
    //   printf("%d",size);
    // get futures array
    if ((farray = (future_t **)getmem(sizeof(future_t *) * (num_streams))) == (future_t **)SYSERR)
    {
        printf("getmem failed\n");
        return (SYSERR);
    }
    for (i = 0; i < num_streams; i++)
    {
        if ((farray[i] = future_alloc(future_flags, sizeof(de), work_queue_depth)) == (future_t *)SYSERR)
        {
            printf("future_alloc failed\n");
            return (SYSERR);
        }
    }
    int st, ts, v;
    char *a;
    int head;
    // Create consumer processes and initialize streams
    // Use `i` as the stream id.

    if ((pcport = ptcreate(num_streams)) == SYSERR)
    {
        printf("ptcreate failed\n");
        return (-1);
    }
    for (i = 0; i < num_streams; i++)
    {
        resume(create((void *)stream_consumer_futures, 4096, 20, "stream_consumer_futures", 2, i, farray[i]));
    }

    for (i = 0; i < n_input; i++)
    {
        a = (char *)stream_input[i];
        st = atoi(a);
        while (*a++ != '\t')
            ;
        ts = atoi(a);
        while (*a++ != '\t')
            ;
        v = atoi(a);
        de *temp = (de *)getmem(sizeof(de));
        temp->time = ts;
        temp->value = v;
        future_set(farray[st], temp);
    }

    // Parse input header file data and populate work queue
    for (i = 0; i < num_streams; i++)
    {
        uint32 pm;
        pm = ptrecv(pcport);
        kprintf("process %d exited\n", pm);
    }
    ptdelete(pcport, 0);
    time = (((clktime * 1000) + clkticks) - ((secs * 1000) + msecs));
    kprintf("time in ms: %u\n", time);
    for (i = 0; i < num_streams; i++)
    {
        future_free(farray[i]);
    }
    freemem((char *)farray, sizeof(future_t *) * (num_streams));
    return 0;
    return 0;
}