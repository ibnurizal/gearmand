/* Gearman server and library
 * Copyright (C) 2008 Brian Aker, Eric Day
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

/**
 * @file
 * @brief libmemcached Queue Storage Definitions
 */

#include <config.h>
#include <libgearman-server/common.h>

#include <libgearman-server/plugins/queue/base.h>
#include <libgearman-server/plugins/queue/libmemcached/queue.h>
#include <libmemcached/memcached.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"

using namespace gearmand;

/**
 * @addtogroup gearmand::plugins::queue::Libmemcachedatic Static libmemcached Queue Storage Functions
 * @ingroup gearman_queue_libmemcached
 * @{
 */

/**
 * Default values.
 */
#define GEARMAN_QUEUE_LIBMEMCACHED_DEFAULT_PREFIX "gear_"

namespace gearmand { namespace plugins { namespace queue { class Libmemcached;  }}}

namespace gearmand {
namespace queue {

class LibmemcachedQueue : public gearmand::queue::Context 
{
public:
  LibmemcachedQueue(plugins::queue::Libmemcached* arg) :
    __queue(arg)
  { 
  }

  ~LibmemcachedQueue()
  {
  }

  gearmand_error_t add(gearman_server_st *server,
                       const char *unique, size_t unique_size,
                       const char *function_name, size_t function_name_size,
                       const void *data, size_t data_size,
                       gearman_job_priority_t priority,
                       int64_t when)
  {
  }

  gearmand_error_t flush(gearman_server_st *server);

  gearmand_error_t done(gearman_server_st *server,
                        const char *unique, size_t unique_size,
                        const char *function_name, size_t function_name_size);

  gearmand_error_t replay(gearman_server_st *server);

private:
  gearmand::plugins::queue::Libmemcached *__queue;
};

} // namespace queue
} // namespace gearmand

namespace gearmand {
namespace plugins {
namespace queue {

class Libmemcached : public gearmand::plugins::Queue {
public:
  Libmemcached ();
  ~Libmemcached ();

  gearmand_error_t initialize();

  memcached_st* memc;
  std::string server_list;
private:

};

Libmemcached::Libmemcached() :
  Queue("libmemcached")
{
  memc= memcached_create(NULL);

  command_line_options().add_options()
    ("libmemcached-servers", boost::program_options::value(&server_list), "List of Memcached servers to use.");
}

Libmemcached::~Libmemcached()
{
  memcached_free(memc);
  memc= NULL;
}

gearmand_error_t Libmemcached::initialize()
{
  gearmand_info("Initializing libmemcached module");

  memcached_server_st *servers= memcached_servers_parse(queue->server_list.c_str());
  if (servers == NULL)
  {
    gearmand_log_error(GEARMAN_DEFAULT_LOG_PARAM, "memcached_servers_parse");

    return GEARMAN_QUEUE_ERROR;
  }

  memcached_server_push(queue->memc, servers);
  memcached_server_list_free(servers);

  gearmand::queue::SqliteQueue* exec_queue= new gearmand::queue::SqliteQueue(this);
  gearman_server_set_queue(&Gearmand()->server, exec_queue);

  return GEARMAN_SUCCESS;
}

void initialize_libmemcached()
{
  static Libmemcached local_instance;
}

} // namespace queue
} // namespace plugins
} // namespace gearmand

/* Queue callback functions. */

namespace gearmand {
namespace queue {

gearmand_error_t LibmemcachedQueue::flush(gearman_server_st *)
{
  gearmand_debug("libmemcached flush");

  return GEARMAN_SUCCESS;
}

gearmand_error_t LibmemcachedQueue::done(gearman_server_st*,
                                         const char *unique, size_t unique_size,
                                         const char *function_name, size_t function_name_size)
{
  char key[MEMCACHED_MAX_KEY];

  gearmand_log_debug(GEARMAN_DEFAULT_LOG_PARAM, "libmemcached done: %.*s", (uint32_t)unique_size, (char *)unique);

  size_t key_length= (size_t)snprintf(key, MEMCACHED_MAX_KEY, "%s%.*s-%.*s",
                                      GEARMAN_QUEUE_LIBMEMCACHED_DEFAULT_PREFIX,
                                      (int)function_name_size,
                                      (const char *)function_name, (int)unique_size,
                                      (const char *)unique);

  /* For the moment we will assume it happened */
  memcached_return rc= memcached_delete(__queue->memc, (const char *)key, key_length, 0);
  if (rc != MEMCACHED_SUCCESS)
  {
    return GEARMAN_QUEUE_ERROR;
  }

  return GEARMAN_SUCCESS;
}

struct replay_context
{
  memcached_st clone;
  gearman_server_st *server;
  gearman_queue_add_fn *add_fn;
  void *add_context;
};

static memcached_return callback_loader(const memcached_st*,
                                        memcached_result_st*,
                                        void *context)
{
  struct replay_context *container= (struct replay_context *)context;
  const char *key;
  const char *unique;
  const char *function;
  size_t function_len;

  key= memcached_result_key_value(result);
  if (strncmp(key, GEARMAN_QUEUE_LIBMEMCACHED_DEFAULT_PREFIX, strlen(GEARMAN_QUEUE_LIBMEMCACHED_DEFAULT_PREFIX)))
    return MEMCACHED_SUCCESS;

  function= key + strlen(GEARMAN_QUEUE_LIBMEMCACHED_DEFAULT_PREFIX);

  unique= index(function, '-');

  if (! unique)
    return MEMCACHED_SUCCESS;

  function_len = (size_t) (unique-function);
  unique++;

  assert(unique);
  assert(strlen(unique));
  assert(function);
  assert(function_len);

  std::vector<char> data;
  /* need to make a copy here ... gearman_server_job_free will free it later */
  try {
    data.resize(memcached_result_length(result));
  }
  catch(...)
  {
    gearmand_perror("malloc");
    return MEMCACHED_MEMORY_ALLOCATION_FAILURE;
  } 
  memcpy(data, memcached_result_value(result), data_size);

  /* Currently not looking at failure cases */
  (void)add(container->server,
            unique, strlen(unique),
            function, function_len,
            data, data.size(),
            static_cast<gearmand_job_priority_t>(memcached_result_flags(result)), 0);


  return MEMCACHED_SUCCESS;
}

/* Grab the object and load it into the loader */
static memcached_return callback_for_key(const memcached_st*,
                                         const char *key, size_t key_length,
                                         void *context)
{
  struct replay_context *container= (struct replay_context *)context;
  memcached_execute_function callbacks[1];
  char *passable[1];

  callbacks[0]= (memcached_execute_fn)&callback_loader;

  passable[0]= (char *)key;
  memcached_return_t rc= memcached_mget(&container->clone, passable, &key_length, 1);
  (void)rc;

  /* Just void errors for the moment, since other treads might have picked up the object. */
  (void)memcached_fetch_execute(&container->clone, callbacks, context, 1);

  return MEMCACHED_SUCCESS;
}

/*
  If we have any failures for loading values back into replay we just ignore them.
*/
gearmand_error_t LibmemcachedQueue::replay(gearman_server_st *server)
{
  struct replay_context container;
  memcached_st *check_for_failure;
  memcached_dump_func callbacks[1];

  callbacks[0]= (memcached_dump_fn)&callback_for_key;

  gearmand_info("libmemcached replay start");

  memset(&container, 0, sizeof(struct replay_context));
  check_for_failure= memcached_clone(&container.clone, __queue->memc);
  container.server= server;
  container.add_fn= add_fn;
  container.add_context= add_context;

  assert(check_for_failure);

  (void)memcached_dump(__queue->memc, callbacks, (void *)&container, 1);

  memcached_free(&container.clone);

  return GEARMAN_SUCCESS;
}

} // queue
} // gearmand

