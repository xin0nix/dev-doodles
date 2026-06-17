MAN:
glibc provides no wrapper for clone3(); it must be invoked via syscall(2).

OBS:
Unlike glibc clone(), clone3() does not accept a thread entry function. The caller is responsible for low-level thread setup.

Attempting to create a thread directly with clone3() quickly exposed additional concerns such as stack setup and thread-local state.

A successful clone3() call created a thread, but the thread immediately crashed (RIP = 0x0), suggesting that glibc's clone() performs non-trivial setup beyond issuing a kernel syscall.

UNK:
- How is glibc clone() implemented internally?
- Does it eventually invoke clone() or clone3()?
- Which thread initialization steps are performed in user space?

