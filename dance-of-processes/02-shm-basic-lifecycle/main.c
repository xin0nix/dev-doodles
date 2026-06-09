#include <fcntl.h> /* For O_* constants */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/wait.h>
#include <unistd.h>

#define defer _Defer

typedef struct {
  const char *name;
  const uint size;
  int fd;
} ShmObj;

bool deinit(ShmObj *obj);

bool init(ShmObj *obj) {
  puts("setup");
  auto fd = shm_open(obj->name, O_CREAT | O_RDWR, 0600);
  if (fd == -1) {
    perror("shm_open");
    return false;
  }
  obj->fd = fd;
  if (ftruncate(obj->fd, obj->size) == -1) {
    deinit(obj);
    perror("ftruncate");
    return false;
  }
  return true;
}

bool deinit(ShmObj *obj) {
  puts("tearDown");
  // could be closed earlier, right after mmap call
  if (close(obj->fd) == -1) {
    perror("close");
    return false;
  }
  if (shm_unlink(obj->name) == -1) {
    perror("shm_unlink");
    return false;
  }
  return true;
}

int parent(pid_t childPid, ShmObj *obj) {
  defer deinit(obj);
  printf("Parent. The child is %d\n", childPid);
  int wstatus;
  do {
    auto w = waitpid(childPid, &wstatus, 0);
    if (w == -1) {
      perror("waitpid");
      return EXIT_FAILURE;
    }
    if (WIFEXITED(wstatus)) {
      printf("Exited, status=%d\n", WEXITSTATUS(wstatus));
    }
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  return EXIT_SUCCESS;
}

int child(ShmObj *obj) {
  puts("child");
  if (close(obj->fd) == -1) {
    perror("close");
  }
  fflush(stdout);
  return EXIT_SUCCESS;
}

int main() {
  ShmObj shmObj = {
      .fd = -1,
      .name = "/02-shmemoization",
      .size = 8,
  };
  if (!init(&shmObj)) {
    return EXIT_FAILURE;
  }
  auto pid = fork();
  if (pid < 0) {
    perror("fork");
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    return child(&shmObj);
  }
  return parent(pid, &shmObj);
}
