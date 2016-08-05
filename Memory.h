/*
 * Author: Emma Kimlin
 * Title: Memory.h
 * Date Created: 5.6.2016
 * Description: Memory stimulates the memory unit that implements paging.
 * Modifications:
*/

#ifndef Memory_h
#define Memory_h

#include <iostream>
#include <queue>
#include <utility>
#include <set>
#include <map>
#include "PCB.h"

struct GreaterThanBasedOnProcessSize { //Function Object to implement comparator based on process size for the Job Pool (std::set job_pool_)
    bool operator() ( const PCB& lhs, const PCB& rhs) const { return lhs.getSizeOfProcess() >= rhs.getSizeOfProcess(); }
};

class Memory {
public:
	Memory(); //Constructor depends on user input. 

	/* Initialize the maxmimum size of a process. Will query user until they enter not larger than total_memory_size_. */
	void InitMaxProcessSize() ;

	/* Initialize Page size. Queries user until they enter an integer greater than zero, that is a power of two 
	 * and divides evenly total_memory_size. 
	*/
	void InitPageSize();

	/*
	 * Post-Condition: the PCB associated with this PID is currently assigned memory in frame_list_. It no longer needs this memory.
	 * Post-Condition: The frames assigned to this PCB are freed.
	*/
	void FreeMemory(int the_PID); 
	/* 
	 * Pre-Condition: Process with PID the_PID is in Job Pool. 
	 * Process waiting to be assigned memory in Job Pool is terminated. 
	*/
	void KillProcessInJobPool(int the_PID); 
	
	/* 
	 * Returns true if process was assigned memory. Returns false if process was put into Job Pool to wait until
	 * enough memory is available. 
	*/
	bool AssignMemory(const PCB& a_pcb); 

	void DisplayFrameList() const; 

	void DisplayFreeFrameList() const; 

	void DisplayJobPool() const; 

	void DisplayPageTable(int PID) const; 

	size_t GetTotalMemorySize() const { return total_memory_size_; }
	size_t GetPageSize() const { return page_size_; }

	/* Return true if there are enough frames in free_frame_list_ for a process waiting in the Job Pool. */
	bool MemoryForWaitingProcesses() const; 

	/* Return true if process the_PID is in Job Pool. */
	bool ProcessInJobPool(int the_PID) const;

	/* 
	 * Pre-Condition: There are enough free frames for a process in the Job Pool to be assigned frames.
	 * Post-Condition: The largest job that will fit has been assigned memory. These frames have been taken 
	 * 				out of free frame list and the job has been taken out of job pool. 
	 * Returns the job that has been assigned memory. 
	*/
	PCB AssignMemoryToProcessInJobPool(); 

	/**
	  * Calculates the physical address from logical address of a given process. 
	  * Returns the decimal value of the physical address. 
	 */
	int CalculatePhysicalAddress(const int& logical_address, const int& PID) const;
	/**
      * Prints the physical address derived from the logical address provided. Ouput is in hex. Assumes input is in decimal. 
     */
    void DisplayPhysicalAddress(int logical_address, int PID) const ; 
    /**
      * Returns the frame number associated with this page number of this process. 
      * Pre-Condition: This process has been allocated memory. If not, a call to this process will exit the program.  
     */
    int GetFrameNumber(const int& page_number, const int& PID) const; 

private:
	size_t total_memory_size_; 
    size_t page_size_; 
    size_t max_size_process_;
    size_t num_pages_; 
    size_t max_pages_per_process_;
    std::deque<size_t> free_frame_list_; 
    std::vector< std::pair<int, int> > frame_list_; //Index is frame number. First int is PID, second int is page number. 
    std::set<PCB, GreaterThanBasedOnProcessSize > job_pool_; //Stores the processes waiting to be assigned memory. Initially empty. 
};

#endif