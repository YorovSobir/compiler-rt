// RUN: %clangxx_tsan -O0 %s -o %t && %run %t 2>&1 | FileCheck %s
#include "test.h"

int nax;
int x;

void* thread1(void* arg) {
  nax = 1;
  __atomic_store_n(&x, 1, __ATOMIC_RELEASE);
  return 0;
}

void* thread2(void* arg) {
  if (__atomic_load_n(&x, __ATOMIC_ACQUIRE) == 1) {
    __atomic_store_n(&x, 2, __ATOMIC_RELAXED);
  }
  return 0;
}

void* thread3(void* arg) {
  if (__atomic_load_n(&x, __ATOMIC_ACQUIRE) == 2) {
    int temp = nax;
    (void)temp;
  } else {
    fprintf(stderr, "DONE\n");
  }
  return 0;
}

int main() {
  pthread_t t1;
  pthread_t t2;
  pthread_t t3;
  pthread_create(&t1, nullptr, thread1, nullptr);
  pthread_create(&t2, nullptr, thread2, nullptr);
  pthread_create(&t3, nullptr, thread3, nullptr);

  pthread_join(t1, nullptr);
  pthread_join(t2, nullptr);
  pthread_join(t3, nullptr);
  return 0;
}

// CHECK: WARNING: ThreadSanitizer: data race
// CHECK-NOT-EMPTY: