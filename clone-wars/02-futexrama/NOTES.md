LINUX: Futexes are very basic  and  lend  themselves well  for  building higher-level locking abstractions such as mutexes, condition variables, read-write locks, barriers, and semaphores.
OBS: Comparing mutexes with atomic algorithms is not entirely fair. Modern mutexes are often implemented as atomic synchronization with a blocking fallback. In the uncontended case, a mutex is usually just a few atomic instructions; the kernel participates only under contention.
- A mutex is often "atomics first, sleeping second".
