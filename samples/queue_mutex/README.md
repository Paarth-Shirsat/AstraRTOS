# Queue Mutex Sample

This sample demonstrates safe inter-task communication within the RTOS. It uses a **multi-producer, single-consumer** architecture to exchange data through an asynchronous queue while protecting the physical UART peripheral with a mutex.

The application consists of two producer tasks and one consumer task.

Each producer task increments its own counter and sends the updated value to the queue. The consumer task receives values from the queue.For verification, the producers print the value they enqueue, and the consumer prints the corresponding value it dequeues.

All tasks run with a priority of 2 and a stack size of 128.
