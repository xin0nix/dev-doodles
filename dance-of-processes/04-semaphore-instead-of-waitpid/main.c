#include <assert.h>
#include <fcntl.h> /* For O_* constants */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/wait.h>
#include <unistd.h>

#define defer _Defer

typedef struct {
  int fd;
  char *data;
  const uint size;
} MemMap;

bool init_MemMap(MemMap *obj);
bool deinit_MemMap(MemMap *obj);

typedef struct {
  const char *name;
  const uint size;
  int fd;
} ShmObj;

bool deinit_ShmObj(ShmObj *obj);
bool init_ShmObj(ShmObj *obj);

#define init(x) _Generic((x), MemMap *: init_MemMap, ShmObj *: init_ShmObj)(x)
#define deinit(x)                                                              \
  _Generic((x), MemMap *: deinit_MemMap, ShmObj *: deinit_ShmObj)(x)

bool init_MemMap(MemMap *obj) {
  puts("init memory map");
  auto buf =
      mmap(nullptr, obj->size, PROT_READ | PROT_WRITE, MAP_SHARED, obj->fd, 0);
  if (buf == MAP_FAILED) {
    perror("mmap");
    return false;
  }
  obj->data = buf;
  // sanity check
  struct stat sb;
  if (fstat(obj->fd, &sb) == -1) {
    perror("fstat");
    return false;
  }
  assert(sb.st_size == obj->size);
  printf("initialized fd %d of size %d\n", obj->fd, obj->size);
  return true;
}

bool deinit_MemMap(MemMap *obj) {
  puts("de-init memory map");
  // The region is automatically unmapped when the process is terminated.
  // But I will unmap it anyway for clarity (and also catch any bugs)
  if (munmap(obj->data, obj->size) == -1) {
    perror("munmap");
    return false;
  };
  return true;
}

bool init_ShmObj(ShmObj *obj) {
  puts("init shared memory object");
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
  printf("initialized shmem obj %s, fd %d, expected size %d\n", obj->name,
         obj->fd, obj->size);
  return true;
}

bool deinit_ShmObj(ShmObj *obj) {
  puts("de-init shared memory object");
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

int parent(pid_t childPid, ShmObj *obj, MemMap *map) {
  printf("Parent. The child is %d\n", childPid);
  defer deinit(obj);
  int wstatus;
  do {
    // even if we do not wait for the child, it won't become zombie for long,
    // because a zombie record in the process table is removed after the parent
    // terminates (just a note)
    auto w = waitpid(childPid, &wstatus, 0);
    if (w == -1) {
      perror("waitpid");
      return EXIT_FAILURE;
    }
    if (WIFEXITED(wstatus)) {
      printf("Exited, status=%d\n", WEXITSTATUS(wstatus));
    }
  } while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));
  // no need for mapping until the child has not finished, because the
  // underlying shared memory object has a name, it will still be available.
  if (!init(map)) {
    return EXIT_FAILURE;
  }
  const char *message = map->data;
  assert(strnlen(message, map->size) < map->size);
  printf("MESSAGE: %s\n", message);
  defer deinit(map);
  return EXIT_SUCCESS;
}

int child(ShmObj *obj, MemMap *map) {
  puts("child");
  defer close(obj->fd);
  defer fflush(stdout);
  if (!init(map)) {
    return EXIT_FAILURE;
  }
  const char *message = "1234567";
  assert(strlen(message) < map->size);
  memcpy(map->data, message, map->size);
  defer deinit(map);
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
  // The  child  process and the parent process run in separate memory spaces.
  // At the time of fork() both memory spaces have the same  content.   Memory
  // writes,  file  mappings (mmap(2)),  and unmappings (munmap(2)) performed by
  // one of the processes do not affect the other.
  auto pid = fork();
  if (pid < 0) {
    perror("fork");
    return EXIT_FAILURE;
  }
  MemMap memMap = {
      .fd = shmObj.fd,
      .data = nullptr,
      .size = 8,
  };
  if (pid == 0) {
    return child(&shmObj, &memMap);
  }
  return parent(pid, &shmObj, &memMap);
}
