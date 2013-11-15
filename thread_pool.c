#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;


struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
};

/**
 * @function void *threadpool_work(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *thread_do_work(void *threadpool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
threadpool_t *threadpool_create(int thread_count, int queue_size)
{
  int i;
  threadpool_t* pool = NULL;
  pool = malloc(sizeof(threadpool_t));
  pool->thread_count = thread_count;
  pool->task_queue_size_limit = queue_size;
  pool->threads = malloc(thread_count * sizeof(pthread_t));
  pool->queue = malloc(queue_size * sizeof(threadpool_task_t));
  printf("pool %p\n", pool);
  //pool->queue[0].argument = NULL;
  for(i=0;i<thread_count;i++)
    {
      pthread_create(&pool->threads[i], NULL, thread_do_work, (void*)pool);
    }
  return pool;
}


/*
 * Add a task to the threadpool
 *
 */
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument)
{
  int err = 0;
  /* Get the lock */
  /* Add task to queue */
  pool->queue[0].function = function;  
  pool->queue[0].argument = argument; 
  /* pthread_cond_broadcast and unlock */
  
  return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int threadpool_destroy(threadpool_t *pool)
{
    int err = 0;

    
    /* Wake up all worker threads */
    

    /* Join all worker thread */
        

    /* Only if everything went well do we deallocate the pool */
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{ 

    while(1) {
        /* Lock must be taken to wait on conditional variable */
        

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), do some task. */
        
        
        /* Grab our task from the queue */
      if(thread->queue[0].argument)
	{
	  printf("OK\n");
	  thread->queue[0].function(thread->queue[0].argument);
	}
      
        /* Unlock mutex for others */


        /* Start the task */

    }

    pthread_exit(NULL);
    return(NULL);
}
