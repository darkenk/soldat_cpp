#include "Threads.hpp"
#include <pthread.h>

bool SetCurrentThreadName(const std::string_view name)
{
  return 0 == pthread_setname_np(pthread_self(), name.data());
}

bool SetThreadName(std::thread &thread, const std::string_view name)
{
  return 0 == pthread_setname_np(thread.native_handle(), name.data());
}
