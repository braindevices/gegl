/* This file is part of GEGL.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2018 Ell
 */

#include "config.h"

#include <glib.h>

#include "gegl.h"
#include "gegl-config.h"
#include "gegl-parallel.h"
#include "gegl-parallel-private.h"


#define GEGL_PARALLEL_DISTRIBUTE_MAX_THREADS GEGL_MAX_THREADS


typedef struct
{
  GeglParallelDistributeFunc func;
  gint                       n;
  gpointer                   user_data;
} GeglParallelDistributeTask;

typedef struct
{
  GThread                    *thread;
  GMutex                      mutex;
  GCond                       cond;

  gboolean                    quit;

  GeglParallelDistributeTask *volatile task;
  volatile gint               i;
} GeglParallelDistributeThread;


/*  local function prototypes  */

static void                       gegl_parallel_notify_threads           (GeglConfig                   *config);

static void                       gegl_parallel_set_n_threads            (gint                          n_threads,
                                                                          gboolean                      finish_tasks);

static void                       gegl_parallel_distribute_set_n_threads (gint                          n_threads);
static gpointer                   gegl_parallel_distribute_thread_func   (GeglParallelDistributeThread *thread);


/*  local variables  */

static gint                         gegl_parallel_distribute_n_threads = 1;
static GeglParallelDistributeThread gegl_parallel_distribute_threads[GEGL_PARALLEL_DISTRIBUTE_MAX_THREADS - 1];

static GMutex                       gegl_parallel_distribute_completion_mutex;
static GCond                        gegl_parallel_distribute_completion_cond;
static volatile gint                gegl_parallel_distribute_completion_counter;
static volatile gint                gegl_parallel_distribute_busy;


/*  public functions  */


void
gegl_parallel_init (void)
{
  g_signal_connect (gegl_config (), "notify::threads",
                    G_CALLBACK (gegl_parallel_notify_threads),
                    NULL);

  gegl_parallel_notify_threads (gegl_config ());
}

void
gegl_parallel_cleanup (void)
{
  g_signal_handlers_disconnect_by_func (gegl_config (),
                                        gegl_parallel_notify_threads,
                                        NULL);

  /* stop all threads */
  gegl_parallel_set_n_threads (0, /* finish_tasks = */ FALSE);
}

void
gegl_parallel_distribute (gint                       max_n,
                          GeglParallelDistributeFunc func,
                          gpointer                   user_data)
{
  GeglParallelDistributeTask task;
  gint                       i;

  g_return_if_fail (func != NULL);

  if (max_n == 0)
    return;

  if (max_n < 0)
    max_n = gegl_parallel_distribute_n_threads;
  else
    max_n = MIN (max_n, gegl_parallel_distribute_n_threads);

  if (max_n == 1 ||
      ! g_atomic_int_compare_and_exchange (&gegl_parallel_distribute_busy,
                                           0, 1))
    {
      func (0, 1, user_data);

      return;
    }

  task.n         = max_n;
  task.func      = func;
  task.user_data = user_data;

  g_atomic_int_set (&gegl_parallel_distribute_completion_counter, task.n - 1);

  for (i = 0; i < task.n - 1; i++)
    {
      GeglParallelDistributeThread *thread =
        &gegl_parallel_distribute_threads[i];

      g_mutex_lock (&thread->mutex);

      thread->task = &task;
      thread->i    = i;

      g_cond_signal (&thread->cond);

      g_mutex_unlock (&thread->mutex);
    }

  func (i, task.n, user_data);

  if (g_atomic_int_get (&gegl_parallel_distribute_completion_counter))
    {
      g_mutex_lock (&gegl_parallel_distribute_completion_mutex);

      while (g_atomic_int_get (&gegl_parallel_distribute_completion_counter))
        {
          g_cond_wait (&gegl_parallel_distribute_completion_cond,
                       &gegl_parallel_distribute_completion_mutex);
        }

      g_mutex_unlock (&gegl_parallel_distribute_completion_mutex);
    }

  g_atomic_int_set (&gegl_parallel_distribute_busy, 0);
}

typedef struct
{
  gsize                           size;
  GeglParallelDistributeRangeFunc func;
  gpointer                        user_data;
} GeglParallelDistributeRangeData;

static void
gegl_parallel_distribute_range_func (gint                             i,
                                     gint                             n,
                                     GeglParallelDistributeRangeData *data)
{
  gsize offset;
  gsize sub_size;

  offset   = (2 * i       * data->size + n) / (2 * n);
  sub_size = (2 * (i + 1) * data->size + n) / (2 * n) - offset;

  data->func (offset, sub_size, data->user_data);
}

void
gegl_parallel_distribute_range (gsize                           size,
                                gsize                           min_sub_size,
                                GeglParallelDistributeRangeFunc func,
                                gpointer                        user_data)
{
  GeglParallelDistributeRangeData data;
  gsize                           n = size;

  g_return_if_fail (func != NULL);

  if (size == 0)
    return;

  if (min_sub_size > 1)
    n /= min_sub_size;

  n = CLAMP (n, 1, gegl_parallel_distribute_n_threads);

  data.size      = size;
  data.func      = func;
  data.user_data = user_data;

  gegl_parallel_distribute (
    n,
    (GeglParallelDistributeFunc) gegl_parallel_distribute_range_func,
    &data);
}

typedef struct
{
  const GeglRectangle            *area;
  GeglSplitStrategy               split_strategy;
  GeglParallelDistributeAreaFunc  func;
  gpointer                        user_data;
} GeglParallelDistributeAreaData;

static void
gegl_parallel_distribute_area_func (gint                            i,
                                    gint                            n,
                                    GeglParallelDistributeAreaData *data)
{
  GeglRectangle sub_area;

  switch (data->split_strategy)
    {
    case GEGL_SPLIT_STRATEGY_HORIZONTAL:
      sub_area.x       = data->area->x;
      sub_area.width   = data->area->width;

      sub_area.y       = (2 * i       * data->area->height + n) / (2 * n);
      sub_area.height  = (2 * (i + 1) * data->area->height + n) / (2 * n);

      sub_area.height -= sub_area.y;
      sub_area.y      += data->area->y;

      break;

    case GEGL_SPLIT_STRATEGY_VERTICAL:
      sub_area.y       = data->area->y;
      sub_area.height  = data->area->height;

      sub_area.x       = (2 * i       * data->area->width + n) / (2 * n);
      sub_area.width   = (2 * (i + 1) * data->area->width + n) / (2 * n);

      sub_area.width  -= sub_area.x;
      sub_area.x      += data->area->x;

      break;

    default:
      g_return_if_reached ();
    }

  data->func (&sub_area, data->user_data);
}

void
gegl_parallel_distribute_area (const GeglRectangle            *area,
                               gsize                           min_sub_area,
                               GeglSplitStrategy               split_strategy,
                               GeglParallelDistributeAreaFunc  func,
                               gpointer                        user_data)
{
  GeglParallelDistributeAreaData data;
  gsize                          n;

  g_return_if_fail (area != NULL);
  g_return_if_fail (func != NULL);

  if (area->width <= 0 || area->height <= 0)
    return;

  if (split_strategy == GEGL_SPLIT_STRATEGY_AUTO)
    {
      if (area->width > area->height)
        split_strategy = GEGL_SPLIT_STRATEGY_VERTICAL;
      else
        split_strategy = GEGL_SPLIT_STRATEGY_HORIZONTAL;
    }

  n = (gsize) area->width * (gsize) area->height;

  if (min_sub_area > 1)
    n /= min_sub_area;

  n = CLAMP (n, 1, gegl_parallel_distribute_n_threads);

  data.area           = area;
  data.split_strategy = split_strategy;
  data.func           = func;
  data.user_data      = user_data;

  gegl_parallel_distribute (
    n,
    (GeglParallelDistributeFunc) gegl_parallel_distribute_area_func,
    &data);
}


/*  private functions  */


static void
gegl_parallel_notify_threads (GeglConfig *config)
{
  gint n_threads;

  g_object_get (config,
                "threads", &n_threads,
                NULL);

  gegl_parallel_set_n_threads (n_threads,
                               /* finish_tasks = */ TRUE);
}

static void
gegl_parallel_set_n_threads (gint     n_threads,
                             gboolean finish_tasks)
{
  gegl_parallel_distribute_set_n_threads (n_threads);
}

static void
gegl_parallel_distribute_set_n_threads (gint n_threads)
{
  gint i;

  while (! g_atomic_int_compare_and_exchange (&gegl_parallel_distribute_busy,
                                              0, 1));

  n_threads = CLAMP (n_threads, 1, GEGL_PARALLEL_DISTRIBUTE_MAX_THREADS);

  if (n_threads > gegl_parallel_distribute_n_threads) /* need more threads */
    {
      for (i = gegl_parallel_distribute_n_threads - 1; i < n_threads - 1; i++)
        {
          GeglParallelDistributeThread *thread =
            &gegl_parallel_distribute_threads[i];

          thread->quit = FALSE;
          thread->task = NULL;

          thread->thread = g_thread_new (
            "worker",
            (GThreadFunc) gegl_parallel_distribute_thread_func,
            thread);
        }
    }
  else if (n_threads < gegl_parallel_distribute_n_threads) /* need less threads */
    {
      for (i = n_threads - 1; i < gegl_parallel_distribute_n_threads - 1; i++)
        {
          GeglParallelDistributeThread *thread =
            &gegl_parallel_distribute_threads[i];

          g_mutex_lock (&thread->mutex);

          thread->quit = TRUE;
          g_cond_signal (&thread->cond);

          g_mutex_unlock (&thread->mutex);
        }

      for (i = n_threads - 1; i < gegl_parallel_distribute_n_threads - 1; i++)
        {
          GeglParallelDistributeThread *thread =
            &gegl_parallel_distribute_threads[i];

          g_thread_join (thread->thread);
        }
    }

  gegl_parallel_distribute_n_threads = n_threads;

  g_atomic_int_set (&gegl_parallel_distribute_busy, 0);
}

static gpointer
gegl_parallel_distribute_thread_func (GeglParallelDistributeThread *thread)
{
  g_mutex_lock (&thread->mutex);

  while (TRUE)
    {
      if (thread->quit)
        {
          break;
        }
      else if (thread->task)
        {
          thread->task->func (thread->i, thread->task->n,
                              thread->task->user_data);

          if (g_atomic_int_dec_and_test (
                &gegl_parallel_distribute_completion_counter))
            {
              g_mutex_lock (&gegl_parallel_distribute_completion_mutex);

              g_cond_signal (&gegl_parallel_distribute_completion_cond);

              g_mutex_unlock (&gegl_parallel_distribute_completion_mutex);
            }

          thread->task = NULL;
        }

      g_cond_wait (&thread->cond, &thread->mutex);
    }

  g_mutex_unlock (&thread->mutex);

  return NULL;
}