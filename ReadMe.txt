Compile with: "make"
Clean with: "make clean"
Executable: run.me

How to Use:

Type "A" to add a process to the Ready Queue. Process are run on a preemptive history-based SJF appoximation algorithm in the CPU and FCFS in the device queues. 

Type "t" to terminate the process in the CPU. 

Type "S" followed by "r", "p", "c", "d" to see all processes in the Ready Queues of the CPU, printer, CD/RW, or disk, respectively.
Type "S" followed by "m" to see all free frames and all allocated frames in the Memory Manager.
Type "S" followed by "j" to see all process in the Job Pool that are awaiting frame allocation in the Memory Manager. 

Type "p", "c", or "d" followed by the number of the device (without a space) to issue a system call for the process in the CPU for I/O to this device. 

Type "P", "C", or "D" followed by the number of the device (without a space) to issue an interrupt for the process currently receiving I/O in this device to be put back into the CPU Ready Queue. 

Press control+c to quit. 
