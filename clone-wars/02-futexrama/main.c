#define _GNU_SOURCE

#include <err.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024) /* Stack size for cloned child */

static int childFunc(void *) {
  puts("child");
  return 0;
}

int main() {
  static char stack[STACK_SIZE];
  auto stackTop = stack + STACK_SIZE; /* Assume stack grows downward */

  auto pid = clone(childFunc, stackTop, CLONE_THREAD | CLONE_VM | CLONE_SIGHAND,
                   nullptr);
  if (pid == -1) {
    err(EXIT_FAILURE, "clone");
  }

  printf("clone() returned %jd\n", (intmax_t)pid);
  sleep(1);
  return 0;
}
