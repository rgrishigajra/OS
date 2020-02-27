#include <xinu.h>
#include <future.h>

future_t *future_alloc(future_mode_t mode, uint size, uint nelems)
{
    future_t *fut_new;
    pid32 pid;
    if ((fut_new = (future_t *)getmem(sizeof(future_t) + size)) == (future_t *)SYSERR)
    {
        printf("getmem failed\n");
        return (SYSERR);
    }
    fut_new->data = sizeof(future_t) + (char *)fut_new;
    fut_new->mode = mode;
    fut_new->size = size;
    fut_new->state = FUTURE_EMPTY;
    fut_new->mode = mode;
    fut_new->get_queue = newqueue();
    fut_new->set_queue = newqueue();
    fut_new->pid = pid;
    return fut_new;
}

syscall future_free(future_t* f){
    intmask mask;
    mask=disable();
    if(freemem((char*)f,sizeof(f)+f->size)==SYSERR)
    {
        restore(mask);
        return(SYSERR);
    }else
    {
        restore(mask);
        return(OK);
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
    return SYSERR;
}