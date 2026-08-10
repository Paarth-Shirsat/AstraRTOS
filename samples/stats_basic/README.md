# Task statistics sample code

This sample demonstrates AstraRTOS's real-time task monitoring
capabilities. A dedicated stats task periodically reads scheduler
metrics for each task, including run count and stack high water
mark, and prints them out over UART.
