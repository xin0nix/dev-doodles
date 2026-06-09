- a shared memory object's name might look like a file or url, but it is not; it simply starts with a slash.
- After a call to  mmap  the  file  descriptor  may  be closed without affecting the memory mapping; Interesting, if I closed a file descriptor and tried to write/read to/from it I bet I would get an error.
- given that C lacks builtin error-handling mechanics (like exceptions in c++ or unions in zig), even a small program becomes unreadable; it does not mean I need to switch to these higher-level languages, but simply make a dedicated set of doodles.
- "The child inherits copies of the parent's set of open file descriptors"; So I assume that they both should call close of that fd's (*no errors, seems ok*).
- The lifecycle of the shared memory object's name and the lifecycle of file descriptors appear to be independent. After fork(), both processes inherit a file descriptor and should close their own copy; However, the shared memory name exists only once, so a second shm_unlink() fails with ENOENT.

