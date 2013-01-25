/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#if HAVE_CONFIG_H
  #include "config.h"
#endif

#include <rtems/libcsupport.h>

#include <string.h>

#include <rtems/libio_.h>
#include <rtems/malloc.h>
#include <rtems/score/wkspace.h>
#include <rtems/score/protectedheap.h>

#ifdef RTEMS_POSIX_API
  #include <rtems/posix/barrier.h>
  #include <rtems/posix/cond.h>
  #include <rtems/posix/mqueue.h>
  #include <rtems/posix/mutex.h>
  #include <rtems/posix/key.h>
  #include <rtems/posix/psignal.h>
  #include <rtems/posix/pthread.h>
  #include <rtems/posix/rwlock.h>
  #include <rtems/posix/semaphore.h>
  #include <rtems/posix/spinlock.h>
  #include <rtems/posix/timer.h>
#endif

static const Objects_Information *objects_info_table[] = {
  &_Barrier_Information,
  &_Extension_Information,
  &_Message_queue_Information,
  &_Partition_Information,
  &_Rate_monotonic_Information,
  &_Dual_ported_memory_Information,
  &_Region_Information,
  &_Semaphore_Information,
  &_RTEMS_tasks_Information,
  &_Timer_Information
  #ifdef RTEMS_POSIX_API
    ,
    &_POSIX_Barrier_Information,
    &_POSIX_Condition_variables_Information,
    &_POSIX_Keys_Information,
    &_POSIX_Message_queue_Information,
    &_POSIX_Message_queue_Information_fds,
    &_POSIX_Mutex_Information,
    &_POSIX_RWLock_Information,
    &_POSIX_Semaphore_Information,
    &_POSIX_Spinlock_Information,
    &_POSIX_Threads_Information,
    &_POSIX_Timer_Information
  #endif
};

static int open_files(void)
{
  int free_count = 0;
  rtems_libio_t *iop;

  rtems_libio_lock();

  iop = rtems_libio_iop_freelist;
  while (iop != NULL) {
    ++free_count;

    iop = iop->data1;
  }

  rtems_libio_unlock();

  return (int) rtems_libio_number_iops - free_count;
}

void rtems_resource_snapshot_take(rtems_resource_snapshot *snapshot)
{
  uint32_t *active = &snapshot->rtems_api.active_barriers;
  size_t i;

  _Protected_heap_Get_information(RTEMS_Malloc_Heap, &snapshot->heap_info);

  _Thread_Disable_dispatch();

  _Heap_Get_information(&_Workspace_Area, &snapshot->workspace_info);

  for (i = 0; i < RTEMS_ARRAY_SIZE(objects_info_table); ++i) {
    active [i] = _Objects_Active_count(objects_info_table[i]);
  }

  _Thread_Enable_dispatch();

  #ifndef RTEMS_POSIX_API
    memset(&snapshot->posix_api, 0, sizeof(snapshot->posix_api));
  #endif

  snapshot->open_files = open_files();
}

bool rtems_resource_snapshot_equal(
  const rtems_resource_snapshot *a,
  const rtems_resource_snapshot *b
)
{
  return memcmp(a, b, sizeof(*a)) == 0;
}

bool rtems_resource_snapshot_check(const rtems_resource_snapshot *snapshot)
{
  rtems_resource_snapshot now;

  rtems_resource_snapshot_take(&now);

  return rtems_resource_snapshot_equal(&now, snapshot);
}