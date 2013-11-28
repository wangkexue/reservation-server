#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

//#define PRIORITY_QUEUE

#ifdef PRIORITY_QUEUE
/**
 * @struct param_t
 * @brief  Param       which will be passed to handle_conn.
 * @var    cornnfd_ptr Addr for connfd.
 * @var    buf         Buffer read from connfd.
 */
typedef struct {
  int* connfd_ptr;
  char* buf;
  int priority;
} param_t; 
#endif

typedef struct threadpool_t threadpool_t;

/**
 * @function threadpool_create
 * @brief Creates a threadpool_t object.
 * @param thread_count Number of worker threads.
 * @param queue_size   Size of the queue.
 * @return a newly created thread pool or NULL
 */
threadpool_t *threadpool_create(unsigned char thread_count,unsigned char queue_size,  int (*routine)(void*));

/**
 * @function threadpool_add
 * @brief add a new task in the queue of a thread pool
 * @param pool  Threadpool to use.
 * @param function Pointer to the function that will perform the task.
 * @param argument Argument to be passed to the function.
 * @return 0 if all goes well, negative values in case of error
 */
#ifdef PRIORITY_QUEUE
void threadpool_add_task(threadpool_t *pool, void *arg, int priority);
#else
void threadpool_add_task(threadpool_t *pool, int *arg);
#endif
/**
 * @function threadpool_destroy
 * @brief Stops and destroys a thread pool.
 * @param pool  Thread pool to destroy.
 *
 * Known values for flags are 0 (default) and threadpool_graceful in
 * which case the thread pool doesn't accept any new tasks but
 * processes all pending tasks before shutdown.
 */
void threadpool_destroy(threadpool_t *pool);

#endif
