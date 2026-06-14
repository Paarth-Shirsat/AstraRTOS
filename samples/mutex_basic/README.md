## Mutex sample code
Two tasks increment and print a shared counter, protected by a mutex.

Task 1 increments and prints the counter every 500 ms,task 2 every 700 ms.
The mutex (os_mutex_init, os_mutex_take, os_mutex_give) ensures only
one task accesses the counter at a time, preventing race conditions
and interleaved output.

Both tasks run at priority 2 with a stack size of 128.