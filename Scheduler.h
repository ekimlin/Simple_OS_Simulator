/*
 * Author: Emma Kimlin
 * Title: Scheduler.h
 * Date Created: 3.2.2016
 * Description: Scheduler is a tool for simulating some basic devices in an operating system that manage processes.
*/

#ifndef Scheduler_h
#define Scheduler_h
#include "PCB.h"
#include "Memory.h"
#include <deque>
#include <vector>
#include <string>
#include <iostream>
#include <set>


class Scheduler
{
public:
    /**
     * The constructer will interact with the user for the system generation. It will ask the user
     * how many devices and of what kind are in the system.
     */
    Scheduler();
    
    /**
     * When a process arrives, add_process is called to handle this. The process is given a PCB and entered
     * into the Ready Queue. If the CPU is empty, the next process in line is brought into the CPU to be
     * run.
     * Post-Condition: The CPU is executing a process.
     */
    void AddNewProcess();
    
    /**
     * When a process is finished running in the CPU, terminate_process() is called. The PCB is recycled
     * and if another process is waiting to be run, it is brought into the CPU to be run.
     * NOTE: The PID is NOT recycled.
     */
    void TerminateProcessInCPU();
    
    /**
     * snapshot() is an interrupt. It simulates a Big Button on the Sys-op's console. It will interact with
     * the user to see what device's queues they want to print.
     */
    void Snapshot();
    
    /**
     * Parses user command. This decide what device the user is referencing and whether they are issuing a
     * system call or interrupt. It will verify that the command they are attempting to generate is
     * compatible with this system (in other words, the device they are referencing is present).
     * Pre-Condition: User input is in form "[first letter of device name][device number]"
     */
    void ParseCommand (std::string user_input);
    
private:
    std::vector<std::deque<PCB> > printer_;
    std::vector<std::deque<PCB> > disk_;
    std::vector<std::deque<PCB> > CD_RW_;
    std::multiset<PCB> Ready_Queue_; //Ready_Queue holds all processes currently waiting to enter CPU
    std::vector<int> num_cylinders_per_disk_; //where disk[i] has num_cylinders_per_disk[i] cylinders
    PCB* CPU_; //The CPU holds one process at a time that it will run.
    double history_parameter_; 
    double initial_burst_estimate_; // in milliseconds 
    int process_counter_; //Keeps track of how many processes have been in the system -- not the current number of processes. 
    double avg_CPU_usage_; // Rolling average of total CPU time of completed processes in milliseconds
    double num_terminated_processes_; //Number of termininated processes. Needed to compute avg_CPU_usage
    Memory memory_unit_; 
    /**
     * init() is the system generator. It asks the user how many devices are on their system, as well
     * as the hostoriaal paramete, initial burst estimate and number of cylinders each disk has.
     * The user can only have printers, disks, and CD_RW devices on their system. 
     *0 <= historical parameter <= 1.
     */
    void Init(std::vector<std::deque<PCB> >& device, std::string device_name);

    /**
     * Initialize history_parameter_ by quering user. Queries user repeatedly until they enter
     * proper value (a number <= 0 and >= 1).
    */
    void InitHisParam();

    /**
     * Initialize initial_burst_estimate by quering user. Queries user repeatedly until they 
     * enter a number.  
    */
    void InitBurst();

    /**
      * Initialize the number of cylinders per disk. Queries user repeatedly until they enter 
      * an integer.
     */
    void InitNumCylinders();

    /**
     * If CPU is empty, fill_CPU will give CPU a process to run.
     * Pre-Condition: CPU and Ready Queue may or may not be empty.
     * Post-Condtion: If there is a process in Ready Queue, it is added to the CPU.
     */
    void FillCPU();

    /**
     * Prints all of the queues in the vector to the screen. Each element in seperated
     * by a space, and each queue is seperated by a new line. If a queue is empty, message is displayed.
     * Prints page table for element of each device. 
     */
    void DisplayQueues(std::vector<std::deque<PCB> >& device, char first_letter) const;

    /**
      * Print the page table for every process in device passed as argument.
     */
    void DisplayPageTables(std::vector<std::deque<PCB> >& device, char first_letter, int which_device) const;
    /**
      * Print the page table for every process in Ready Queue.
     */
    void DisplayPageTablesReadyQueue() const;
    /**
     * Prints the name of the private member variables of a PCB object. The output is formatted. This is
     * followed by a new line.
     */
    void DisplayHeader() const;

    /**
      * Prints the PIDs of all PCBs in Ready_Queue, seperated by a space and followed by a new line.
     */
    void DisplayReadyQueue() const;
    
    /**
      * A process in the CPU requests I/O from a device. If the device exists, the process is added to the
      * device queue and a process from the Ready Queue is placed in the CPU.
     */
    void ProcessSyscall(std::deque<PCB>& device_queue, std::string device_name, int device_num);
    
    /**
      * An interrupt is generated by the device when a task in the device's queue is completed. The PCB for
      * this task is returned to the Ready Queue.
     */
    void DeviceInterrupt(std::deque<PCB>& device_queue, std::string device_name);
    
    /**
      * Pre-Condition: There is a process in the system with PID the_PID. 
      * Post-Condition: This process is terminated and its memory is recycled. 
     */
    void KillProcess(int the_PID); 

    bool FindPCBAndKill_CheckDisks(int the_PID);
    bool FindPCBAndKill_CheckCD_RW(int the_PID);
    bool FindPCBAndKill_CheckPrinters(int the_PID);
    bool FindPCBAndKill_CheckReadyQ(int the_PID);

    /*
     * Collects all accounting information from terminated process. 
    */
    void TerminatingProcessAccounting(const PCB& process_to_kill); 

    /**
      * Internal method for when I/O is requested by a process in the CPU and therefore the PCB of this
      * process needs to be updated.
     */
    void UpdatePCB_InCPU(std::string device_name, int device_num);

    /**
      * Add a process to the Ready_Queue, using SJF Pre-emptive CPU Scheduling. 
    */
    void AddProcessToReadyQueue(PCB& a_process);
    
    /*
     * Pre-Condition: Process has requested disk I/O. 
     * Queries user to find what cylinder the process is requesting access to. Queries until user
     * provides valid cylinder. 
     * Returns cylinder number. 
    */
    int WhichCylinder(int device_num);

    /*
     * Called anytime process a_pcb issues a system call.  
    */
    void UpdateAccountingInfo_Syscall(PCB& a_pcb);

    /*
     * Will update the PCB in the CPU. 
    */
    void UpdateAccountingInfo_Interrupt();

    /*
     * Checks to see if any processes in Job Pool can be assigned memory and if so, assigns memory and 
     *   adds process to Ready Queue. 
    */
    void LoadProcesses(); 
};

#endif /* Scheduler_h */
