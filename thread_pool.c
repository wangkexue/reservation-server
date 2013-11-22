#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

//#include <stdio.h>

#include "thread_pool.h"

#define PRIORITY_QUEUE
/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

/**
 *  @struct task_t
 *  @brief task which wait in queue
 *  since every task use the same function
 *  we store the function in threadpool rather than here
 *  @var argument Be passed to function
 *  @var next Pointer to next task 
 */
typedef struct task_t{
    int *argument;
    struct task_t* next;
} threadpool_task_t;
/** 
 *  @struct queue_t
 *  @brief singly-linked list as worker queue
 *  @var head Where thread fetch job
 *  @var tail Where new task added
 */
typedef struct {
  threadpool_task_t *head;
  threadpool_task_t *tail;
} queue_t;
/**
 *  @struct threadpool_t
 *  @var over Flag for server-down
 */
#ifdef PRIORITY_QUEUE
struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  int (*function)(void *);
  queue_t q1;
  queue_t q2;
  queue_t q3;
  int thread_count;
  int task_queue_size_limit;
  unsigned char over;
};
#else
struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  int (*function)(void *);
  queue_t queue;
  unsigned char thread_count;
  //int task_queue_size_limit;
  unsigned char over;
};
#endif


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
threadpool_t *threadpool_create(unsigned char thread_count, int (*function)(void*))
{
  int i;

  threadpool_t* pool = (threadpool_t*)malloc(sizeof(threadpool_t));
  pool->thread_count = thread_count;
  //pool->task_queue_size_limit = queue_size;
  pool->threads = malloc(thread_count * sizeof(pthread_t));

  pool->function = function;
  
  #ifdef PRIORITY_QUEUE
  pool->q1.head = NULL;
  pool->q1.tail = NULL;
  pool->q2.head = NULL;
  pool->q2.tail = NULL;
  pool->q3.head = NULL;
  pool->q3.tail = NULL;  
  #else
  pool->queue.head = NULL;
  pool->queue.tail = NULL;
  #endif
  //pthread_attr_init(&attr);
  //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for(i=0;i<thread_count;i++)
    {
      pthread_create(&pool->threads[i], NULL, thread_do_work, (void*)pool);
    }
  //pthread_attr_destroy(&attr);
  pthread_mutex_init(&pool->lock, NULL);

  pthread_cond_init(&pool->notify, NULL);
  return pool;
}


/*
 * Add a task to the threadpool
 *
 */
int threadpool_add_task(threadpool_t *pool, void *argument, int priority)
{
  int err = 0;
  /* Get the lock */
  pthread_mutex_lock(&pool->lock);
  /* Add task to queue */
  /* add task to the queue's tail */
  threadpool_task_t* task = (threadpool_task_t*)malloc(sizeof(threadpool_task_t));
  //task->function = function;
  task->argument = argument;
  task->next = NULL;
#ifdef PRIORITY_QUEUE
  if(priority == 1)
    {
      if(pool->q1.head == NULL)
	pool->q1.head = task;
      if(pool->q1.tail == NULL)
	{
	  pool->q1.tail = task;
	}
      else
	{
	  pool->q1.tail->next = task;
	  pool->q1.tail = task;
	}
    }
  else if(priority == 2)
    {
      if(pool->q2.head == NULL)
	pool->q2.head = task;
      if(pool->q2.tail == NULL)
	{
	  pool->q2.tail = task;
	}
      else
	{
	  pool->q2.tail->next = task;
	  pool->q2.tail = task;
	}
    }
  else
    {
      if(pool->q3.head == NULL)
	pool->q3.head = task;
      if(pool->q3.tail == NULL)
	{
	  pool->q3.tail = task;
	}
      else
	{
	  pool->q3.tail->next = task;
	  pool->q3.tail = task;
	}
    }
#else
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
#endif
  pool->over = 0;
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
    pool->over = 1;
    for(i=0;i<pool->thread_count;i++)
      { 
	pthread_cond_signal(&pool->notify);
      }
    /* Join all worker thread */
    for(i=0;i<pool->thread_count;i++)        
      pthread_join(pool->threads[i], NULL);
 
   
    /* Only if everything went well do we deallocate the pool */
    #ifdef PRIORITY_QUEUE
    while(pool->q1.head != NULL)
      {
	threadpool_task_t* task = pool->q1.head;
	pool->q1.head = pool->q1.head->next;
	free(task->argument);
	free(task);
      }
    while(pool->q2.head != NULL)
      {
	threadpool_task_t* task = pool->q2.head;
	pool->q2.head = pool->q2.head->next;
	free(task->argument);
	free(task);
      }
    while(pool->q3.head != NULL)
      {
	threadpool_task_t* task = pool->q3.head;
	pool->q3.head = pool->q3.head->next;
	free(task->argument);
	free(task);
      }
    #else
    while(pool->queue.head != NULL)
      {
	threadpool_task_t* task = pool->queue.head;
	pool->queue.head = pool->queue.head->next;
	free(task->argument);
	free(task);
      }
    #endif

    //for(i=0;i<pool->thread_count;i++)
    //  free(&pool->threads[i]);
    free(pool->threads);

    free(pool);

    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{ 
  threadpool_t* thread = (threadpool_t*)threadpool;
  //printf("%d\n", *(thread->queue[0].argument));
  while(1) {
    /* Lock must be taken to wait on conditional variable */
    pthread_mutex_lock(&thread->lock);
    /* Wait on condition variable, check for spurious wakeups.
       When returning from pthread_cond_wait(), do some task. */
    pthread_cond_wait(&thread->notify, &thread->lock);
    if(thread->over==1)
      break;
    /* Grab our task from the queue */
    #ifdef PRIORITY_QUEUE
    threadpool_task_t* task = NULL;
    if(thread->q1.head)
      {
	task = thread->q1.head;
	thread->q1.head = thread->q1.head->next;
	if(thread->q1.head == NULL)
	  thread->q1.tail = NULL;
      }
    else if(thread->q2.head)
      {
	task = thread->q2.head;
	thread->q2.head = thread->q2.head->next;
	if(thread->q2.head == NULL)
	  thread->q2.tail = NULL;
      }
    else
      {
	task = thread->q3.head;
	thread->q3.head = thread->q3.head->next;
	if(thread->q3.head == NULL)
	  thread->q3.tail = NULL;
      }
    #else
    threadpool_task_t* task = thread->queue.head;
    thread->queue.head = thread->queue.head->next;
    if(thread->queue.head == NULL)
      thread->queue.tail = NULL;
    #endif
    /* Unlock mutex for others */
    pthread_mutex_unlock(&thread->lock);
    
    /* Start the task */
    thread->function(task->argument);
    free(task->argument);
    free(task);
  }

  pthread_exit(NULL);
  return(NULL);
}
