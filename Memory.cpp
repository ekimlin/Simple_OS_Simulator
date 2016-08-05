#include "Memory.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdlib.h>

/********************Utility Functions********************/
namespace MemoryNamespace {

//Utility Function that returns positive integer provided by user.
int GetPositiveIntFromUser()
{
    std::string user_input;
    int integer_value;
    std::stringstream ss;
    bool bad_input = true;
    while (bad_input) {
        std::cin >> user_input;
        ss << user_input;
        if ( (ss >> integer_value) && !(ss >> user_input) && (integer_value >= 0) ) //user entered a positive integer that is not trailed by a string
            bad_input = false;
        else {
            std::cout << "       Invalid Input. Enter a number.";
            ss.str("");
            ss.clear();
        }
    }
    return integer_value;
}
} //end MemoryNamespace

/********************Public Member Functions********************/

Memory::Memory() {
    std::cout << "   Enter the total size of memory. "; 
    total_memory_size_ = MemoryNamespace::GetPositiveIntFromUser();
    InitMaxProcessSize();
    InitPageSize();
    num_pages_ = total_memory_size_ / page_size_; 
    max_pages_per_process_ = max_size_process_ / page_size_;
    for (size_t i = 0; i < num_pages_; ++i) {
    	free_frame_list_.push_back(i); 
    	frame_list_.push_back( std::make_pair(-1, -1) );
    }
}

void Memory::InitMaxProcessSize() {
	std::cout << "   Enter the maximum size of a process for this system. ";
	max_size_process_ = MemoryNamespace::GetPositiveIntFromUser();
    while (max_size_process_ > total_memory_size_) {
        std::cout << "       Invalid Entry. Maximum Process Size cannot be greater than total memory.\n" 
                  << "       Try again.";
        max_size_process_ = MemoryNamespace::GetPositiveIntFromUser();
    }
}

void Memory::InitPageSize() {
    std::cout << "   Enter the size of a page for this system. ";
    page_size_ = MemoryNamespace::GetPositiveIntFromUser();
    while ( ((page_size_ & (page_size_ - 1)) != 0 ) || (page_size_ == 0) || (total_memory_size_ % page_size_) ) {
        std::cout << "       Invalid Entry. Please enter page size that is a power of two,\n"
                  << "       greater than zero, and that divides " << total_memory_size_ << " evenly.";
        page_size_ = MemoryNamespace::GetPositiveIntFromUser();
    }
}

void Memory::FreeMemory(int the_PID) {
    std::cout << "   Freeing memory that P" << the_PID << " was using. \n";
    //find the frames a_pcb has used by iteratng through entire frame_list until end. 
    for (size_t i = 0; i < frame_list_.size(); ++i) {
        if ( frame_list_[i].first == the_PID ) { //a_pcb was assigned this frame. 
            frame_list_[i] = std::make_pair(-1, -1); //default value signifies that frame is unused. 
            free_frame_list_.push_back(i); //Add newly freed frames to free_frame_list_. 
        }
    }
}

void Memory::KillProcessInJobPool(int the_PID) {
    auto iter = job_pool_.begin(); 
    while ( iter != job_pool_.end() ) {
        if (iter->getPID() == the_PID) {
            job_pool_.erase(iter); 
            std::cout << "     P" << the_PID << " (located in Job Pool) has been killed.\n";
            return;
        }
        ++iter;
    }
}

bool Memory::AssignMemory(const PCB& a_pcb) {
	size_t num_pages_needed = ceil( a_pcb.getSizeOfProcess() / page_size_) ; // Check to see if there are enough frames available for this process.
    int the_PID = a_pcb.getPID();
	if (free_frame_list_.size() < num_pages_needed) { //Not enough frames available--put into Job Pool until there are. 
		job_pool_.insert(a_pcb); 
        std::cout << "     There is not enough memory for this Job. Inserted into Job Pool.\n";
		return false;
	}
	for (size_t i = 0; i < num_pages_needed; ++i) { //Enough free frames--Assign a free frame to every page the process requires.
		int frame = free_frame_list_.front(); 
		free_frame_list_.pop_front(); 
		frame_list_[frame] = std::pair<int, int>(the_PID, i); 
	}
    return true;
}

PCB Memory::AssignMemoryToProcessInJobPool() {  //assign to frames, take out of free frame list, and take out of job pool
    auto iter = job_pool_.begin(); //start with largest job in job_pool_
    while ( iter != job_pool_.end() ) {
        size_t num_pages_needed = ceil( iter->getSizeOfProcess() / page_size_);
        if ( num_pages_needed <= free_frame_list_.size() ) { //Found biggest job in Job Pool that can be assigned frames.
            std::cout <<"   Assigning P" << iter->getPID() << " (from Job Pool) memory. \n";
            for (size_t i = 0; i < num_pages_needed; ++i) { //Assign a free frame to every page the process requires.
                int frame = free_frame_list_.front(); 
                free_frame_list_.pop_front(); //Remove frames from free_frame_list_ now that they are assigned. 
                frame_list_[frame] = std::make_pair(iter->getPID(), i); //Assign frames
            }
            PCB a_ready_pcb = *iter; //take the job that was just assigned memory out of the job pool 
            job_pool_.erase(iter); //delete this job out of Job Pool now that it is assigned.
            return a_ready_pcb;
        }   
        ++iter; 
    }
    std::cout << "Error. Cannot assign memory to jobs in Job Pool if no memory available. \n";
    exit(1);
}

void Memory::DisplayFreeFrameList() const {
	std::cout << "   Free Frames: ";
    if ( free_frame_list_.empty() ) {
        std::cout << "   Empty.\n";
        return;
    }
	auto iter = free_frame_list_.begin(); 
	while (iter != free_frame_list_.end()) {
		std::cout << *iter << " "; 
		++iter; 
	}
	std::cout << std::endl;
}

void Memory::DisplayFrameList() const {
    std::cout << "PID" << std::setw(14) << "Page Number" << std::endl;
    for (auto iter = frame_list_.begin(); iter != frame_list_.end(); ++iter) {
        if (iter->first != -1)
            std::cout << iter->first << std::setw(14) << iter->second << std::endl;
        else 
            std::cout << "Unused" << std::endl;
    }
}

void Memory::DisplayJobPool() const {
    std::cout << "   Job Pool: " << std::endl;
    if ( job_pool_.empty() ) {
        std::cout << "   Empty" << std::endl;
        return;
    }
    auto iter = job_pool_.begin(); 
    while ( iter != job_pool_.end() ) {
        std::cout << iter->getPID() << std::setw(7) << iter->getSizeOfProcess() << std::endl;
        ++iter;
    }
}
void Memory::DisplayPageTable(int PID) const {
    std::cout << "P" << PID << ": ";
    //Search frametable:
    for (size_t i = 0; i < frame_list_.size(); ++i) { //go through Frame Table until you have displayed all of the frames this process uses 
        if (frame_list_[i].first == PID) 
            std::cout << i << " ";
    }
    std::cout << std::endl;
}

bool Memory::MemoryForWaitingProcesses() const {
    if ( job_pool_.empty() ) {
        std::cout << "     The Job Pool is empty. No Processes to assign memory. \n";
        return false;
    }
    auto iter = job_pool_.begin(); //start with largest job in job_pool_
    while ( iter != job_pool_.end() ) {
        size_t num_pages_needed = ceil( iter->getSizeOfProcess() / page_size_);
        if ( num_pages_needed <= free_frame_list_.size() ) return true;
        ++iter; 
    }
    return false;
} 

bool Memory::ProcessInJobPool(int the_PID) const {
    auto iter = job_pool_.begin();
    while ( iter != job_pool_.end() ) {
        if ( iter->getPID() == the_PID ) return true;
        ++iter;
    }
    return false;
}

int Memory::CalculatePhysicalAddress(const int& logical_address, const int& PID) const {
    if (logical_address == 0) return 0;
    std::div_t divresult = std::div(logical_address, page_size_);
    int offset = divresult.rem; 
    int page_number = divresult.quot; 
    int frame_number = GetFrameNumber(page_number, PID);
    std::string frame_num_str = std::to_string(frame_number);
    std::string offset_str = std::to_string(offset);
    std::string physical_address_str = frame_num_str + offset_str;

    std::stringstream ss(physical_address_str); 
    int physical_address_int; 
    ss >> physical_address_int; 
    return physical_address_int;
}

void Memory::DisplayPhysicalAddress(int logical_address, int PID) const {
    if (logical_address == 0) {
        std::cout << "     Physical Address is 0." << std::endl;
        return;
    }
    int p_add = CalculatePhysicalAddress(logical_address, PID);
    std::cout << "     Physical Address is " << std::hex << p_add << std::endl;
} 

int Memory::GetFrameNumber(const int& page_number, const int& PID) const {
    for (size_t i = 0; i < frame_list_.size(); ++i) {
        if ( (frame_list_[i].first == PID) && (frame_list_[i].second == page_number) ) 
            return i; 
    }
    std::cout << "       Error: Process " << PID << " not allocated memory. Enter another command.\n";
    exit(1);
}




















    