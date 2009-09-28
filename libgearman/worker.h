/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief Worker declarations
 */

#ifndef __GEARMAN_WORKER_H__
#define __GEARMAN_WORKER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup gearman_worker Worker Interface
 * This is the interface gearman workers should use.
 *
 * @ref main_page_worker "See Main Page for full details."
 * @{
 */

/**
 * Initialize a worker structure. Always check the return value even if passing
 * in a pre-allocated structure. Some other initialization may have failed.
 *
 * @param[in] worker Caller allocated structure, or NULL to allocate one.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
GEARMAN_API
gearman_worker_st *gearman_worker_create(gearman_worker_st *worker);

/**
 * Clone a worker structure.
 *
 * @param[in] worker Caller allocated structure, or NULL to allocate one.
 * @param[in] from Structure to use as a source to clone from.
 * @return Same return as gearman_worker_create().
 */
GEARMAN_API
gearman_worker_st *gearman_worker_clone(gearman_worker_st *worker,
                                        const gearman_worker_st *from);

/**
 * Free resources used by a worker structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 */
GEARMAN_API
void gearman_worker_free(gearman_worker_st *worker);

/**
 * See gearman_error() for details.
 */
GEARMAN_API
const char *gearman_worker_error(gearman_worker_st *worker);

/**
 * See gearman_errno() for details.
 */
GEARMAN_API
int gearman_worker_errno(gearman_worker_st *worker);

/**
 * Get options for a worker structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @return Options set for the worker structure.
 */
GEARMAN_API
gearman_worker_options_t gearman_worker_options(const gearman_worker_st *worker);

/**
 * Set options for a worker structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param options Available options for worker structures.
 */
GEARMAN_API
void gearman_worker_set_options(gearman_worker_st *worker,
                                gearman_worker_options_t options);

/**
 * Add options for a worker structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param options Available options for worker structures.
 */
GEARMAN_API
void gearman_worker_add_options(gearman_worker_st *worker,
                                gearman_worker_options_t options);

/**
 * Remove options for a worker structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param options Available options for worker structures.
 */
GEARMAN_API
void gearman_worker_remove_options(gearman_worker_st *worker,
                                   gearman_worker_options_t options);

/**
 * See gearman_timeout() for details.
 */
GEARMAN_API
int gearman_worker_timeout(gearman_worker_st *worker);

/**
 * See gearman_set_timeout() for details.
 */
GEARMAN_API
void gearman_worker_set_timeout(gearman_worker_st *worker, int timeout);

/**
 * Get the application context for a worker.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @return Application context that was previously set, or NULL.
 */
GEARMAN_API
void *gearman_worker_context(const gearman_worker_st *worker);

/**
 * Set the application context for a worker.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] context Application context to set.
 */
GEARMAN_API
void gearman_worker_set_context(gearman_worker_st *worker, const void *context);

/**
 * See gearman_set_log_fn() for details.
 */
GEARMAN_API
void gearman_worker_set_log_fn(gearman_worker_st *worker,
                               gearman_log_fn *function, const void *context,
                               gearman_verbose_t verbose);

/**
 * See gearman_set_event_watch_fn() for details.
 */
GEARMAN_API
void gearman_worker_set_event_watch_fn(gearman_worker_st *worker,
                                       gearman_event_watch_fn *function,
                                       const void *context);

/**
 * See gearman_set_workload_malloc_fn() for details.
 */
GEARMAN_API
void gearman_worker_set_workload_malloc_fn(gearman_worker_st *worker,
                                           gearman_malloc_fn *function,
                                           const void *context);

/**
 * See gearman_set_workload_free_fn() for details.
 */
GEARMAN_API
void gearman_worker_set_workload_free_fn(gearman_worker_st *worker,
                                         gearman_free_fn *function,
                                         const void *context);

/**
 * Add a job server to a worker. This goes into a list of servers that can be
 * used to run tasks. No socket I/O happens here, it is just added to a list.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] host Hostname or IP address (IPv4 or IPv6) of the server to add.
 * @param[in] port Port of the server to add.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_add_server(gearman_worker_st *worker,
                                           const char *host, in_port_t port);

/**
 * Add a list of job servers to a worker. The format for the server list is:
 * SERVER[:PORT][,SERVER[:PORT]]...
 * Some examples are:
 * 10.0.0.1,10.0.0.2,10.0.0.3
 * localhost:1234,jobserver2.domain.com:7003,10.0.0.3
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] servers Server list described above.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_add_servers(gearman_worker_st *worker,
                                            const char *servers);

/**
 * Remove all servers currently associated with the worker.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 */
GEARMAN_API
void gearman_worker_remove_servers(gearman_worker_st *worker);

/**
 * When in non-blocking I/O mode, wait for activity from one of the servers.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_wait(gearman_worker_st *worker);

/**
 * Register function with job servers with an optional timeout. The timeout
 * specifies how many seconds the server will wait before marking a job as
 * failed. If timeout is zero, there is no timeout.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] function_name Function name to register.
 * @param[in] timeout Optional timeout (in seconds) that specifies the maximum
 *  time a job should. This is enforced on the job server. A value of 0 means
 *  an infinite time.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_register(gearman_worker_st *worker,
                                         const char *function_name,
                                         uint32_t timeout);

/**
 * Unregister function with job servers.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] function_name Function name to unregister.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_unregister(gearman_worker_st *worker,
                                           const char *function_name);

/**
 * Unregister all functions with job servers.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_unregister_all(gearman_worker_st *worker);

/**
 * Get a job from one of the job servers. This does not used the callback
 * interface below, which means results must be sent back to the job server
 * manually. It is also the responsibility of the caller to free the job once
 * it has been completed.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] job Caller allocated structure, or NULL to allocate one.
 * @param[out] ret_ptr Standard gearman return value.
 * @return On success, a pointer to the (possibly allocated) structure. On
 *  failure this will be NULL.
 */
GEARMAN_API
gearman_job_st *gearman_worker_grab_job(gearman_worker_st *worker,
                                        gearman_job_st *job,
                                        gearman_return_t *ret_ptr);

/**
 * Free a job structure.
 *
 * @param[in] job Structure previously initialized with
 *  gearman_worker_grab_job().
 */
GEARMAN_API
void gearman_job_free(gearman_job_st *job);

/**
 * Free all jobs for a gearman structure.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 */
GEARMAN_API
void gearman_job_free_all(gearman_worker_st *worker);

/**
 * Register and add callback function for worker. To remove functions that have
 * been added, call gearman_worker_unregister() or
 * gearman_worker_unregister_all().
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] function_name Function name to register.
 * @param[in] timeout Optional timeout (in seconds) that specifies the maximum
 *  time a job should. This is enforced on the job server. A value of 0 means
 *  an infinite time.
 * @param[in] function Function to run when there is a job ready.
 * @param[in] context Argument to pass into the callback function.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_add_function(gearman_worker_st *worker,
                                             const char *function_name,
                                             uint32_t timeout,
                                             gearman_worker_fn *function,
                                             const void *context);

/**
 * Wait for a job and call the appropriate callback function when it gets one.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_work(gearman_worker_st *worker);

/**
 * Send data to all job servers to see if they echo it back. This is a test
 * function to see if job servers are responding properly.
 *
 * @param[in] worker Structure previously initialized with
 *  gearman_worker_create() or gearman_worker_clone().
 * @param[in] workload The workload to ask the server to echo back.
 * @param[in] workload_size Size of the workload.
 * @return Standard gearman return value.
 */
GEARMAN_API
gearman_return_t gearman_worker_echo(gearman_worker_st *worker,
                                     const void *workload,
                                     size_t workload_size);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __GEARMAN_WORKER_H__ */
