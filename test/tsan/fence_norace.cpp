// RUN: %clangxx_tsan -O0 %s -o %t && %run %t 2>&1 | FileCheck %s
#include "test.h"
#include <atomic>

int nax;
std::atomic<int> x;

void* thread1(void* arg) {
  nax = 1;
  std::atomic_thread_fence(std::memory_order_release);
  x.store(1, std::memory_order_relaxed);
  return 0;
}

void* thread2(void* arg) {
  if (x.load(std::memory_order_relaxed) == 1) {
    std::atomic_thread_fence(std::memory_order_acquire);
    printf("%d\n", nax);
  }
  return 0;
}

int main() {
  pthread_t t1;
  pthread_t t2;
  pthread_create(&t1, nullptr, thread1, nullptr);
  pthread_create(&t2, nullptr, thread2, nullptr);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);
  return 0;
}

// CHECK-NOT: ThreadSanitizer: data race
// CHECK: 1
