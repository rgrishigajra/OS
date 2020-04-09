#include <xinu.h>
#include <future.h>

future_t *future_alloc(future_mode_t mode, uint size, uint nelems)
{
    future_t *fut_new;
    pid32 pid;
    if (mode == FUTURE_QUEUE)
    {
        if ((fut_new = (future_t *)getmem(sizeof(future_t) + size * nelems)) == (future_t *)SYSERR)
        {
            printf("getmem failed\n");
            return (SYSERR);
        }
    }
    else
    {
        if ((fut_new = (future_t *)getmem(sizeof(future_t) + size)) == (future_t *)SYSERR)
        {
            printf("getmem failed\n");
            return (SYSERR);
        }
    }

    fut_new->data = sizeof(future_t) + (char *)fut_new;
    fut_new->mode = mode;
    fut_new->size = size;
    fut_new->state = FUTURE_EMPTY;
    fut_new->mode = mode;
    fut_new->get_queue = newqueue();
    fut_new->set_queue = newqueue();
    fut_new->pid = pid;
    fut_new->max_elems = nelems;
    fut_new->count = 0;
    fut_new->head = 0;
    fut_new->tail = 0;
    return fut_new;
}

syscall future_free(future_t *f)
{
    intmask mask;
    mask = disable();
    if (freemem((char *)f, sizeof(f) + f->size) == SYSERR)
    {
        restore(mask);
        return (SYSERR);
    }
    else
    {
        restore(mask);
        return (OK);
    }
}

syscall future_get(future_t *f, char *out)
{
    if (f->mode == FUTURE_EXCLUSIVE)
    {

        if (f->state == FUTURE_EMPTY)
        {
            f->state = FUTURE_WAITING;
            f->pid = getpid();
            suspend(f->pid);
            *out = *f->data;
            return OK;
        }
        if (f->state == FUTURE_READY)
        {
            *out = *f->data;
            return OK;
        }
    }
    if (f->mode == FUTURE_SHARED)
    {
        if (f->state == FUTURE_EMPTY)
        {
            f->state = FUTURE_WAITING;
            enqueue(getpid(), f->get_queue);
            suspend(getpid());
            memcpy(out, f->data, sizeof(f->data));
            return OK;
        }
        if (f->state == FUTURE_READY)
        {
            memcpy(out, f->data, sizeof(f->data));
            return OK;
        }
        if (f->state == FUTURE_WAITING)
        {
            enqueue(getpid(), f->get_queue);
            suspend(getpid());
            memcpy(out, f->data, sizeof(f->data));
            return OK;
        }
    }
    if (f->mode == FUTURE_QUEUE)
    {
        // printf("inside future get consumer");
        if (f->state == FUTURE_EMPTY)
        {
            f->state = FUTURE_WAITING;
            enqueue(getpid(), f->get_queue);
            suspend(getpid());
            // printf("consumer resumed");
            char *headelemptr = f->data + (f->head * f->size);

            memcpy(out, headelemptr, f->size);
            f->head++;
            f->count--;
            if (f->head == f->max_elems)
            {
                f->head = 0;
            }

            return OK;
        }
        if (f->state == FUTURE_READY)
        {
            if (f->count == 0)
            {
                enqueue(getpid(), f->get_queue);
                suspend(getpid());
            }
            // printf("consumer resumed");
            char *headelemptr = f->data + (f->head * f->size);

            memcpy(out, headelemptr, f->size);
            f->head++;
            f->count--;
            if (f->head == f->max_elems)
            {
                f->head = 0;
            }
            pid32 proc;
            proc = dequeue(f->set_queue);
            resume(proc);
            return OK;
        }
        if (f->state == FUTURE_WAITING)
        {
            enqueue(getpid(), f->get_queue);
            suspend(getpid());
            // printf("consumer resumed");
            char *headelemptr = f->data + (f->head * f->size);

            memcpy(out, headelemptr, f->size);
            f->head++;
            f->count--;
            if (f->head == f->max_elems)
            {
                f->head = 0;
            }
            return OK;
        }
    }
    return SYSERR;
}

syscall future_set(future_t *f, char *in)
{
    if (f->mode == FUTURE_EXCLUSIVE)
    {

        if (f->state == FUTURE_EMPTY)
        {
            *f->data = *in;
            f->state = FUTURE_READY;
            return OK;
        }
        if (f->state == FUTURE_WAITING)
        {
            *f->data = *in;
            f->state = FUTURE_READY;
            resume(f->pid);
            return OK;
        }
    }
    if (f->mode == FUTURE_SHARED)
    {
        if (f->state == FUTURE_EMPTY)
        {
            memcpy(f->data, in, sizeof(in));
            f->state = FUTURE_READY;
            return OK;
        }
        if (f->mode == FUTURE_WAITING)
        {
            memcpy(f->data, in, sizeof(in));
            f->state = FUTURE_READY;
            pid32 proc;
            while ((proc = dequeue(f->get_queue)) != EMPTY)
            {
                resume(proc);
            }
        }
    }
    if (f->mode == FUTURE_QUEUE)
    {
        if (f->count == f->max_elems - 1)
        {
            // kprintf("producer slept\n");
            enqueue(getpid(), f->set_queue);
            suspend(getpid());
        }
        // kprintf("producer resumed");
        if (f->state == FUTURE_EMPTY)
        {
            char *tailelemptr = f->data + (f->tail * f->size);

            memcpy(tailelemptr, in, f->size);
            // printf("oye2");
            f->tail++;
            f->count++;
            f->state = FUTURE_READY;
            pid32 proc;
            proc = dequeue(f->get_queue);
            // kprintf("%d", proc);
            // printf("resume a consumer if ");
            resume(proc);

            return OK;
        }
        if (f->state == FUTURE_WAITING)
        {  
            char *tailelemptr = f->data + (f->tail * f->size);
            memcpy(tailelemptr, in, f->size);
            // printf("oye1");
            f->tail++;
            f->count++;
            if (f->tail == f->max_elems)
            {
                f->tail = 0;
            }
            // printf("oye2");
            f->state = FUTURE_READY;
            pid32 proc;
            proc = dequeue(f->get_queue);
            // kprintf("%d", proc);
            // printf("resume a consumer if ");
            resume(proc);
            return OK;
        }
        if (f->state == FUTURE_READY)
        {
            char *tailelemptr = f->data + (f->tail * f->size);

            memcpy(tailelemptr, in, f->size);
            // printf("oye1");
            f->tail++;
            f->count++;
            if (f->tail == f->max_elems)
            {
                f->tail = 0;
            }
            // printf("oye2");
            pid32 proc;
            proc = dequeue(f->get_queue);
            // kprintf("%d", proc);
            // printf("resume a consumer if ");
            resume(proc);
            return OK;
        }
    }
    return SYSERR;
}