MAN: the kernel picks a new address that may or may not depend on the hint
OBS:
UNK: why a buffer for the mapping is not provided by user (like in recv)?

MAN: After the mmap() call has returned, the file descriptor, fd, can be closed immediately without invalidating the mapping.
OBS:
UNK: I assume the reason is that the target shared memory object is managed by its name via shm_open and shm_unlink; What makes this API odd to me is that I would expect something like shm_link + open/close and shm_unlink; What is the reasoning behind the design choice here?

MAN: The region is automatically unmapped when the process is terminated.
OBS:
UNK:

MAN: SEE ALSO ftruncate(2), getpagesize(2), memfd_create(2), mincore(2), mlock(2),  mmap2(2),  mprotect(2),  mremap(2),  msync(2),  remap_file_pages(2),  setrlimit(2),  shmat(2),  userfaultfd(2)
OBS: I feel overwhelmed!
UNK: everything here

