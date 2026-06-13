MAN: POSIX semaphores allow processes and threads to synchronize their actions.
OBS: -
UNK: Is mutex implemented on top of a semaphore?

MAN: The  sem_open(3)  function  creates  a new named semaphore or opens an existing named semaphore...it can be removed from the system using sem_unlink(3).
OBS: I am starting to see a pattern here, shm_open/shm_unlink operates the same way (in at least one exception that semaphore needs a dedicated sem_close call, while shm_close does not exist).
UNK: Is the file API the same? I bet it is, and it's kind of a shame I don't know for sure, LOL.

MAN: I kind of unexpectedly learnt that "a System V shared memory segment" exists and is managed by a dedicated API - shmget(2)..
OBS: This is kind of a zoo of system calls
UNK: Why it exits, and is it implemented differently from POSIX shm? I would bet it is just a different facade of the same underlying mechanism, but need to investigate later (wild guess).

MAN: "child created by fork(2) inherits its parent's memory mappings"
OBS: I refactored the code and it did work, so there was no need to call mmap twice
UNK: -

MAN:
OBS: I feel (feel indeed) that semaphores act more of like condition variables rather than pure mutexes (maybe), need to rewind my understanding of this topic
UNK: Basically, what is the relation (under the hood of course) between semaphores on the one side and condition_variables/mutexes on the other? And the elephant in the room for me how to increment a semaphore?
