#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define defer _Defer

int main() {
  // TODO: if a kernel-scheduled thread is not distinct from a process how both
  // of them are represented in the Kernel?
  auto pid = fork();
  if (pid < 0) {
    perror("fork");
    _exit(EXIT_FAILURE);
  }

  if (pid == 0) {
    puts("child");
    fflush(stdout);
    _exit(EXIT_SUCCESS);
  } else {
    printf("Child is %d\n", pid);
    int wstatus;
    do {
      // TODO: if waitpid is not called, the child zombifies; the reason is the
      // Kernel process table overflow; Is it of a fixed size? Could it be made
      // dynamic?
      auto w = waitpid(pid, &wstatus, 0);
      if (w == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
      if (WIFEXITED(wstatus)) {
        printf("Exited, status=%d\n", WEXITSTATUS(wstatus));
      }
    } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  }
  _exit(EXIT_SUCCESS);
}
