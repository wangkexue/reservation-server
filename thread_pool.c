#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//#include <stdio.h>

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

typedef struct task_t{
    int (*function)(void *);
    int *argument;
    struct task_t* next;
} threadpool_task_t;

typedef struct {
  threadpool_task_t *head;
  threadpool_task_t *tail;
} queue_t;

struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  queue_t queue;
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
pthread_mutex_t thread_lock;
pthread_mutex_t task_lock;
pthread_cond_t task_on;
*/
//int over;
//pthread_attr_t attr;

/*
 * Create a threadpool, initialize variables, etc
 *
 */
threadpool_t *threadpool_create(int thread_count, int queue_size)
{
  int i;
  //over = 0;
  threadpool_t* pool = malloc(sizeof(threadpool_t));
  pool->thread_count = thread_count;
  pool->task_queue_size_limit = queue_size;
  pool->threads = malloc(thread_count * sizeof(pthread_t));
  
  pool->queue.head = NULL;
  pool->queue.tail = NULL;

  //pthread_attr_init(&attr);
  //pthread_attr_setdetachstate(&attr, PTHREAD_CREAT_JOINABLE);
  for(i=0;i<thread_count;i++)
    {
      pthread_create(&pool->threads[i], NULL, thread_do_work, (void*)pool);
    }
  //pthread_attr_destroy(&attr);
  pthread_mutex_init(&pool->lock, NULL);
  //pthread_mutex_init(&task_lock, NULL);
  pthread_cond_init(&pool->notify, NULL);
  return pool;
}


/*
 * Add a task to the threadpool
 *
 */
int threadpool_add_task(threadpool_t *pool, int (*function)(void *), int *argument)
{
  int err = 0;
  /* Get the lock */
  pthread_mutex_lock(&pool->lock);
  /* Add task to queue */
  // add task to the queue's tail
  threadpool_task_t* task = malloc(sizeof(threadpool_task_t));
  task->function = function;
  task->argument = argument;
  task->next = NULL;
  if(pool->queue.head == NULL)
    pool->queue.head = task;
  if(pool->queue.tail == NULL)
    {
      pool->queue.tail = task;
    }
  else
    {
      pool->queue.tail->next = task;
      pool->queue.tail = task;
    }
  /* pthread_cond_broadcast and unlock */
  pthread_cond_signal(&pool->notify);
  pthread_mutex_unlock(&pool->lock);
  return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int threadpool_destroy(threadpool_t *pool)
{
    int err = 0;
    int i;
    
    /* Wake up all worker threads */
    pthread_mutex_lock(&pool->lock);
    //int count = pool->thread_count;
    //over = 1;
    for(i=0;i<pool->thread_count;i++)
      pthread_cond_signal(&pool->notify);
    pthread_mutex_unlock(&pool->lock);
    /* Join all worker thread */
    for(i=0;i<pool->thread_count;i++)        
      pthread_join(pool->threads[i], NULL);
    pthread_exit(NULL);
    /* Only if everything went well do we deallocate the pool */
    for(i=0;i<pool->task_queue_size_limit;i++)
      {
	threadpool_task_t* task = pool->queue.head;
	pool->queue.head = pool->queue.head->next;
	free(task->argument);
	free(task);
      }
    free(pool->threads);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->notify);
    free(pool);
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{ 
  threadpool_t* thread= (threadpool_t*)threadpool;
  //printf("%d\n", *(thread->queue[0].argument));
  while(1) {
    //if(over)
    //break;
    /* Lock must be taken to wait on conditional variable */
    pthread_mutex_lock(&thread->lock);
    /* Wait on condition variable, check for spurious wakeups.
       When returning from pthread_cond_wait(), do some task. */
    //thread->thread_count--;
    pthread_cond_wait(&thread->notify, &thread->lock);
    /* Grab our task from the queue */
    threadpool_task_t* task = thread->queue.head;
    thread->queue.head = thread->queue.head->next;
    if(thread->queue.head == NULL)
      thread->queue.tail = NULL;
    /* Unlock mutex for others */
    pthread_mutex_unlock(&thread->lock);
    
    /* Start the task */
    task->function(task->argument);
    //thread->thread_count++;
  }
  //printf("OK\n");
  pthread_exit(NULL);
  return(NULL);
}
