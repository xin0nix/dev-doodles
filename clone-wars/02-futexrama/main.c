#define _GNU_SOURCE

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

// Types

#define STACK_SIZE (1024 * 1024) /* Stack size for cloned child */
#define UNLOCKED 0
#define LOCKED 1

typedef struct {
  _Atomic(uint32_t) futex;
} MyMutex;

// Globals

static char gStack[2][STACK_SIZE];
static MyMutex gMutex;
static char gSentence[] = "Hello World";

static long futex(MyMutex *obj, int op, int val) {
  return syscall(SYS_futex, &obj->futex, op, val, nullptr, nullptr, 0);
}

// "Methods"

void lock(MyMutex *obj) {
  for (;;) {
    uint32_t unlocked = UNLOCKED;
    uint32_t locked = LOCKED;
    if (atomic_compare_exchange_strong(&obj->futex, &unlocked, locked)) {
      break;
    }
    if (futex(obj, FUTEX_WAIT, 1) == -1 && errno != EAGAIN && errno != EINTR) {
      perror("futex");
      exit(EXIT_FAILURE);
    }
  }
}

void unlock(MyMutex *obj) {
  // NOTE: avoiding unnecessary FUTEX_WAKE syscalls requires additional
  // state (e.g. waiter tracking), which production mutexes implement.
  atomic_store(&obj->futex, 0);
  if (futex(obj, FUTEX_WAKE, 1) == -1) {
    perror("futex");
    exit(EXIT_FAILURE);
  }
}

// Functions

int worker(void *) {
  // NOTE: comment lock/unlock and get a data race
  for (auto i = 0; i < 1'000; ++i) {
    lock(&gMutex);
    auto len = sizeof(gSentence);
    for (auto j = 0U; j < len; ++j) {
      auto c = gSentence[j];
      if (isupper(c)) {
        gSentence[j] = (char)tolower(c);
      } else {
        gSentence[j] = (char)toupper(c);
      }
    }
    unlock(&gMutex);
  }
  puts("child exits");
  return 0;
}

void run(size_t i) {
  auto stackTop = gStack[i] + STACK_SIZE; /* Assume stack grows downward */
  auto pid = clone(
      worker, stackTop,
      CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_CHILD_CLEARTID, nullptr);
  if (pid == -1) {
    err(EXIT_FAILURE, "clone");
  }
  fflush(stdout);
}

// Entry point

int main() {
  for (auto i = 0U; i < 2; ++i) {
    run(i);
  }
  sleep(2);
  puts("main exits");
  puts(gSentence);
  fflush(stdout);
  return 0;
}
