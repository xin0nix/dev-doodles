#include <assert.h>
#include <ctype.h>
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stddefer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/wait.h>
#include <unistd.h>

#define MESSAGE_CAP 8

// NOTE: Since a child created by fork(2) inherits its parent's memory mappings,
// it  can  also access the semaphore.
typedef struct {
  // child waits for the parent to finish data initialization
  sem_t parentInitBarrier;
  // parent waits for the child to finish data transformation (and print the
  // result)
  sem_t childTransformBarrier;
  // it might be tricky to properly interpret this array; as this buffer is
  // stored in a shm region, the data is a properly sized string
  char message[MESSAGE_CAP];
} BufferLayout;

typedef struct {
  void *data;
} MemMap;

typedef struct {
  const char *name;
  int fd;
  MemMap mappedRegion;
} ShmObj;

BufferLayout *getBufferLayout(MemMap *obj) { return obj->data; }
size_t getBufferSize() { return sizeof(BufferLayout); }

bool init_MemMap(MemMap *obj, int fd) {
  puts(__FUNCTION__);
  auto buf =
      mmap(nullptr, getBufferSize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (buf == MAP_FAILED) {
    perror("mmap");
    return false;
  }
  bool shouldUnmap = true;
  defer {
    if (shouldUnmap) {
      munmap(buf, getBufferSize());
    }
  }
  obj->data = buf;
  // sanity check
  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    return false;
  }
  assert(sb.st_size == (int)getBufferSize());
  printf("initialized shm associated with fd %d, size %d, addr=%lu\n", fd,
         (int)getBufferSize(), (uint64_t)obj->data);
  auto layout = getBufferLayout(obj);
  bool shouldDestroyParentInitBarrier = true;
  defer {
    if (shouldDestroyParentInitBarrier) {
      sem_destroy(&layout->parentInitBarrier);
    }
  }
  const auto pshared = 1;
  const auto initVal = 0;
  if (sem_init(&layout->parentInitBarrier, pshared, initVal) == -1) {
    perror("sem_init 1");
    return false;
  }
  if (sem_init(&layout->childTransformBarrier, pshared, initVal) == -1) {
    perror("sem_init 2");
    return false;
  }
  shouldDestroyParentInitBarrier = false;
  shouldUnmap = false;
  return true;
}

void deinit_MemMap(MemMap *obj) {
  puts(__FUNCTION__);
  // NOTE: The region is automatically unmapped when the process is terminated.
  // But I will unmap it anyway for clarity (and also catch any bugs)
  if (munmap(obj->data, getBufferSize()) == -1) {
    perror("munmap");
  };
}

void deinit_ShmObj(ShmObj *obj);

bool init_ShmObj(ShmObj *obj) {
  puts(__FUNCTION__);
  auto fd = shm_open(obj->name, O_CREAT | O_RDWR, 0600);
  if (fd == -1) {
    perror("shm_open");
    return false;
  }
  obj->fd = fd;
  if (ftruncate(obj->fd, (int)getBufferSize()) == -1) {
    deinit_ShmObj(obj);
    perror("ftruncate");
    return false;
  }
  printf("initialized shmem obj %s, fd %d, expected size %d\n", obj->name,
         obj->fd, (int)getBufferSize());
  return true;
}

void deinit_ShmObj(ShmObj *obj) {
  puts(__FUNCTION__);
  // NOTE: could be closed earlier, right after mmap call
  if (close(obj->fd) == -1) {
    perror("close");
  }
  if (shm_unlink(obj->name) == -1) {
    perror("shm_unlink");
  }
  auto layout = getBufferLayout(&obj->mappedRegion);
  if (sem_destroy(&layout->parentInitBarrier) == -1) {
    perror("sem_destroy 1");
  }
  if (sem_destroy(&layout->childTransformBarrier) == -1) {
    perror("sem_destroy 2");
  }
}

int parent(pid_t childPid, ShmObj *obj) {
  puts(__FUNCTION__);
  printf("child's pid is %d\n", childPid);
  defer deinit_ShmObj(obj);
  auto mapping = &obj->mappedRegion;
  defer deinit_MemMap(mapping);
  auto layout = getBufferLayout(mapping);
  const char *message = "hello";
  strncpy(layout->message, message, MESSAGE_CAP);
  assert(strlen(message) < MESSAGE_CAP);
  // let child continue
  sem_post(&layout->parentInitBarrier);
  // and wait for it to finish transformation
  sem_wait(&layout->childTransformBarrier);
  // NOTE: we don't need to call waitpid anymore the child will remain a zombie
  // for some milliseconds (actually it will be waited on after parent exits)
  assert(strnlen(layout->message, MESSAGE_CAP) < MESSAGE_CAP);
  printf("MESSAGE: %s\n", layout->message);
  deinit_MemMap(mapping);
  return EXIT_SUCCESS;
}

int child(ShmObj *obj) {
  puts(__FUNCTION__);
  defer close(obj->fd);
  defer fflush(stdout);
  auto mapping = &obj->mappedRegion;
  defer deinit_MemMap(mapping);
  auto layout = getBufferLayout(mapping);
  // wait for parent to intialize the message
  sem_wait(&layout->parentInitBarrier);
  // perform the "transformation" (idea from the man pages)
  for (auto i = 0; i < MESSAGE_CAP; ++i) {
    layout->message[i] = (char)toupper(layout->message[i]);
  }
  // let it continue
  sem_post(&layout->childTransformBarrier);
  return EXIT_SUCCESS;
}

int main() {
  puts(__FUNCTION__);
  ShmObj shmObj = {
      .name = "/02-shmemoization",
      .fd = -1,
  };
  if (!init_ShmObj(&shmObj)) {
    return EXIT_FAILURE;
  }
  // the child inherits parent's memory mapping (and both semaphores)
  if (!init_MemMap(&shmObj.mappedRegion, shmObj.fd)) {
    return EXIT_FAILURE;
  }
  puts("fork");
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
