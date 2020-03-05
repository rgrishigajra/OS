#include <xinu.h>
#include <stream.h>
#include "tscdf_input.h"

int num_streams, work_queue_depth, time_window, output_time;

struct stream **inputstream;
uint pcport;
void stream_consumer(int32 id, struct stream *str)

{
    int tail;
    int count = 0;
    struct tscdf *tc = tscdf_init(time_window);
    kprintf("stream_consumer id:%d (pid:%d)\n", id, getpid());
    while (TRUE)
    {
        wait(str->spaces);
        wait(str->mutex);
        tail = str->tail;
        if (str->queue[str->tail].time == 0 && str->queue[str->tail].value == 0)
        {
            // ptsend(pcport,getpid());
            break;
        }
        tscdf_update(tc, str->queue[tail].time, str->queue[tail].value);
        if (count++ == output_time - 1)
        {
            char output[10];
            int32 qarray = tscdf_quartiles(tc);

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
        tail = (tail + 1) % work_queue_depth;
        str->tail = tail;
        signal(str->items);
        signal(str->mutex);
    }
    kprintf("stream_consumer exiting\n");
    ptsend(pcport, getpid());
    return;
}

int stream_proc(int nargs, char *args[])
{
    int i;
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
    // Create streams
    for (i = 0; i < num_streams; i++)
    {
        struct stream *newstream;

        if ((newstream = (struct stream *)getmem(sizeof(struct stream) + size)) == (struct stream *)SYSERR)
        {
            printf("getmem failed\n");
            return (SYSERR);
        }
        newstream->queue = sizeof(struct stream) + (char *)newstream;
        newstream->spaces = semcreate(0);
        newstream->items = semcreate(work_queue_depth);
        newstream->mutex = semcreate(1);
        newstream->head = 0;
        newstream->tail = 0;
        inputstream[i] = newstream;
        printf("Made a new stream in inputstream[%d]: and queue at:%d\n", i, newstream->queue);
    }
    int st, ts, v;
    char *a;
    int head;
    // Create consumer processes and initialize streams
    // Use `i` as the stream id.

    for (i = 0; i < num_streams; i++)
    {
        resume(create((void *)stream_consumer, 4096, 20, "stream_consumer", 2, i, inputstream[i]));
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
        wait(inputstream[st]->items);
        wait(inputstream[st]->mutex);
        head = inputstream[st]->head;
        inputstream[st]->queue[head].time = ts;
        inputstream[st]->queue[head].value = v;
        printf("Produced->%d %d\n", inputstream[st]->queue[head].time, inputstream[st]->queue[head].value);
        head = (head + 1) % work_queue_depth;
        inputstream[st]->head = head;
        signal(inputstream[st]->mutex);
        signal(inputstream[st]->spaces);
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
    printf("time in ms: %u\n", time);
    return 0;
}