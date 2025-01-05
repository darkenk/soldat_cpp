#include "Threads.hpp"
#include <pthread.h>

#if __EMSCRIPTEN__

extern "C" {
  void __stack_chk_fail(void)
  {
  }

  unsigned long __stack_chk_guard = 0;

  int pthread_setschedparam (pthread_t __target_thread, int __policy,
                            const struct sched_param *__param) { return 0;}

  //int sched_get_priority_min (int __algorithm) { return 0; }
  //int sched_get_priority_max (int __algorithm) { return 0; }
}

bool SetCurrentThreadName(const std::string_view name)
{
  return true;
}

bool SetThreadName(std::thread &thread, const std::string_view name)
{
  return true;
}

#else

auto SetCurrentThreadName(const std::string_view name) -> bool
{
  return 0 == pthread_setname_np(pthread_self(), name.data());
}

auto SetThreadName(std::thread &thread, const std::string_view name) -> bool
{
  return 0 == pthread_setname_np(thread.native_handle(), name.data());
}
#endif

