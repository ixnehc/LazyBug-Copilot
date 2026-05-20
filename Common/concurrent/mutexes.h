
#pragma once

#pragma warning(disable:4244)
#include "atomic.h"
#pragma warning(default:4244)

#include "mutex.h"
#include "queuing_mutex.h"
#include "queuing_rw_mutex.h"
#include "spin_mutex.h"
#include "spin_rw_mutex.h"


typedef tbb::mutex Mutex;

typedef tbb::queuing_mutex QueueMutex;
typedef tbb::queuing_rw_mutex QueueMutexRW;

typedef tbb::spin_mutex SpinMutex;
typedef tbb::spin_rw_mutex SpinMutexRW;

