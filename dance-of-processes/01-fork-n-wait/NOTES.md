- waitpid does not necessarily mean that the child has exited; it might return some other status when the child receives a signal or even continued (e.g. woken up from a sleep state);
- "In  the  Linux  kernel,  a  kernel-scheduled thread is not a distinct construct from a process." What? I know that processes have isolated memory regions (each with its own virtual memory space); but threads share the memory and that might lead to race conditions / data races. I won't dig in, rather postpone for later investigation.
  - How threads and processes are represented in the Linux kernel?
- A  child  that terminates, but has not been waited for becomes a "zombie". Which is odd at the first glance, but actually this design choice lets the parent to later get some useful information about the terminated child (its return code etc). So the zombie is killed either by calling wait or by killing its parent. Zombie is not a real process, it's just some left-overs;
  - Does the kernel process table have a fixed size? Could it be dynamic (interesting mini-project on itself, modify the kernel a bit to alter this behavior);

