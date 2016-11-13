 /* Modifications:
 * 3.5.2016 Changed output formatting. Added parse_command() and its dependent functions. Modified 
 *                   print utility that snapshot() calls.
 * 3.7.2016 Added printPIDs() and modified fillCPU() to check if Ready Queue is empty first.
 * 4.7.2016 Added private data members history_parameter_, initial_burst_estimate_ and num_cylinders_
 *                    and modified constructor accordingly.
 * 4.9.2016 Implemented Shortest Job First Scheduling for Ready Queue. Changed Ready_Queue to be a std::multiset and changed all insert
 *          and remove operatations accordingly. 
 * 5.6.2016 Added memory_unit_ and updated constructor, AddProcess() accordingly.
 * 5.7.2016 Added LoadProcess() and KillProcess(). Updated ParseCommand() to recognize K#. 
 * 5.8.2016 Updated DisplayQueues() and added DisplayPageTables(). 
 */

#include "Scheduler.h"
#include <iomanip>
#include <cmath>
#include <sstream> 
#include "Memory.h"
/********************Utility Functions********************/
namespace SchedulerNamespace {

//Utility Function that returns positive integer provided by user.
int GetPositiveIntFromUser() { 
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

} //end SchedulerNamespace

/********************Public Member Functions********************/
Scheduler::Scheduler() {
    Init(printer_, "printers");
    Init(disk_, "disks");
    Init(CD_RW_, "CD_RW devices");
    CPU_ = nullptr;
    process_counter_ = 0;
    avg_CPU_usage_ = 0.0;
    num_terminated_processes_ = 0.0;
    InitHisParam();
    InitBurst();
    InitNumCylinders();
    std::cout << "System Generation completed. Begin entering commands.\n";
}

void Scheduler::AddNewProcess()
{
    std::cout << "     What is the size of this process? ";
    size_t size_of_process = SchedulerNamespace::GetPositiveIntFromUser();
    if (size_of_process > memory_unit_.GetTotalMemorySize()) {
        std::cout << "       Size of Process cannot be larger than total memory. Rejected. \n";
        return;
    }
    PCB p(++process_counter_, initial_burst_estimate_, size_of_process); //Create a new process
    bool process_assigned_memory = memory_unit_.AssignMemory(p); //Place process in Job Pool or assign it memory depending on available memory
    if (process_assigned_memory) AddProcessToReadyQueue(p); //If process was assigned memory, give it to Ready Queue
}

void Scheduler::TerminateProcessInCPU() {
    if (CPU_ == nullptr) //Handle user error
        std::cout << "       There is no process to terminate in the CPU. Please enter another command.\n";
    else {
        std::cout << "     Process " << CPU_->getPID() << " has finished running in the CPU. \n";
        UpdateAccountingInfo_Syscall(*CPU_);
        TerminatingProcessAccounting(*CPU_);
        delete CPU_; //Delete the process in the CPU
        CPU_ = nullptr;
        FillCPU(); //Fill the CPU with the next process in the Ready Queue. 
        LoadProcesses(); //Now that there is more available memory, give memory to next process in Job Pool. 
    }
}

void Scheduler::Snapshot() {
    std::string user_input;
    std::cout << "  A Snapshot interrupt has been issued. \n"
              << "     The average CPU time of completed processes is: " << avg_CPU_usage_ << " ms.\n" 
              << "     Enter 'r', 'p', 'c','d', 'm' or 'j'" << std::endl;
    std::cin >> user_input;
    if (user_input == "r")
        DisplayReadyQueue();
    else if ( (user_input == "p") || (user_input == "c") || (user_input == "d") ) {
        DisplayHeader();
        if (user_input == "p")
            DisplayQueues(printer_, 'p');
        else if (user_input == "c")
            DisplayQueues(CD_RW_, 'c');
        else if (user_input == "d") 
            DisplayQueues(disk_, 'd');
    } else if (user_input == "m") {
            memory_unit_.DisplayFreeFrameList();
            memory_unit_.DisplayFrameList();
    } else if (user_input == "j")
            memory_unit_.DisplayJobPool(); 
    else
        std::cout << "       Bad input. Type 'S' and hit Enter to issue another Snapshot interrupt.\n";
}

void Scheduler::ParseCommand (std::string user_input) {
    char first_letter = user_input.front();
    std::string device_number = user_input.substr(1, user_input.size() -1);
    std::istringstream ss(device_number);
    int num_entered;
    if ( (ss >> num_entered) && !(ss >> user_input) ) {
        if ((first_letter == 'p') && ((unsigned)num_entered <= printer_.size()))
            ProcessSyscall(printer_[num_entered-1], "printer", num_entered);
        else if ((first_letter == 'P') && ((unsigned)num_entered <= printer_.size()))
            DeviceInterrupt(printer_[num_entered-1], "printer");
        else if ((first_letter == 'c') && ((unsigned)num_entered <= CD_RW_.size()))
            ProcessSyscall(CD_RW_[num_entered-1], "CD_RW", num_entered);
        else if ((first_letter == 'C') && ((unsigned)num_entered <= CD_RW_.size()))
            DeviceInterrupt(CD_RW_[num_entered-1], "CD_RW");
        else if ((first_letter == 'd') &&  ((unsigned)num_entered <= disk_.size()))
            ProcessSyscall(disk_[num_entered-1], "disk", num_entered);
        else if ((first_letter == 'D') && ((unsigned)num_entered <= disk_.size()))
            DeviceInterrupt(disk_[num_entered-1], "disk");
        else if ( (first_letter == 'K') && (num_entered > 0) && (num_entered <= process_counter_) )
            KillProcess(num_entered);  
        else //User entered correct format, but did not reference a device in system.
            std::cout << "       Invalid Commmand. This system has " << printer_.size() << " printers, "
            <<"     " << CD_RW_.size() << " CD/RW, and " << disk_.size() << " disks. Please try again.";
    } else //User did not use correct format
            std::cout << "       Invalid Commmand. Please enter one character specificying device, followed"
                      << "       by one number. \n";
}

/********************Private Member Functions********************/
void Scheduler::Init(std::vector<std::deque<PCB> >& device, std::string device_name) {
    std::cout << "   Enter the number of " << device_name << " that are in this system and press Enter." << std::endl;
    int num_device = SchedulerNamespace::GetPositiveIntFromUser();
    for (int i = 0; i < num_device; ++i)
        device.push_back(std::deque<PCB>()); //Create queue for each device to be able to hold its processes
}

void Scheduler::InitHisParam() {
    bool bad_input = true; 
    std::string user_input; 
    std::cout << "   Enter the historical parameter for this system. ";
    while (bad_input) {
        std::cin >> user_input;
        std::istringstream ss_his_param(user_input);
        if ((ss_his_param >> history_parameter_) && (history_parameter_ >= 0 ) && (history_parameter_ <= 1) 
                                                 && !(ss_his_param >> user_input))
            return;
        else     
            std::cout << "       Invalid entry. Please enter a number between 0 and 1 (inclusive). ";
    }
}

void Scheduler::InitBurst() {
    std::cout << "   Enter the ititial burst estimate in milliseconds for all new processes. ";
    initial_burst_estimate_ = SchedulerNamespace::GetPositiveIntFromUser();
}

void Scheduler::InitNumCylinders() {
    if ( disk_.size() == 0 ) return; 
    int num_cylinders;
    for ( uint i = 0; i < disk_.size() ; ++i ) {
        std::cout << "   Enter the number of cylinders for disk " << i+1 << ". "; 
        num_cylinders = SchedulerNamespace::GetPositiveIntFromUser();
        while (num_cylinders <= 0) {
            std::cout <<"       Invalid Entry. Cannot have negative number of cylinders. Try again.";
            num_cylinders = SchedulerNamespace::GetPositiveIntFromUser();
        }
        num_cylinders_per_disk_.push_back(num_cylinders); 
    }
}
void Scheduler::FillCPU() {
    if (CPU_ != nullptr) return; //CPU is not idle; cannot add process to CPU that in use.
    if (Ready_Queue_.empty()) { //CPU is empty, but there are no process waiting to be run. 
        CPU_ = nullptr;
        std::cout << "     No processes to run. The CPU is idle." << std::endl;
    } else {
        PCB process_to_run = *Ready_Queue_.begin(); //Pick next process from Ready Queue
        std::cout << "     Process " << process_to_run.getPID() << " has been added to the CPU." << std::endl;
        CPU_ = new PCB(process_to_run); //copy next process in Ready Queue over to CPU
        Ready_Queue_.erase(Ready_Queue_.begin()); //delete that process in the Ready Queue
    }
}

void Scheduler::DisplayQueues(std::vector<std::deque<PCB> >& device, char first_letter) const {
    for (size_t i = 0; i < device.size(); i++) { //For each device of a certain type
        std::cout << "   Device: " << first_letter << i+1 << std::endl; //output the device number
        if (device[i].empty()) {
            std::cout << "     This queue is empty.\n";
            continue;
        } else { //Output the processes waiting to be run by this device
            for (auto iter = begin(device[i]); iter != end(device[i]); ++iter) {
                iter->Print();
                int phys_add = memory_unit_.CalculatePhysicalAddress(iter->getLogicalStartAddress(), iter->getPID());
                std::cout << "   " << std::hex << phys_add << std::endl; //Output the physical address of this processes
            }
            DisplayPageTables(device, first_letter, i); //Display the page table of this process
        }
    }
}

void Scheduler::DisplayReadyQueue() const {
    if (Ready_Queue_.empty()) {
        std::cout << "     The Ready Queue is empty.\n";
    }
    else {
        std::cout << "PID|" << std::setw(9) << "CPU Use|" << std::setw(11) << "Avg Burst|" << std::setw(7) << "Size|" 
                  << std::setw(10) << "Log. Add|" << std::setw(10) << "Phys. Add" << std::endl; //Header
        for (auto iter = begin(Ready_Queue_); iter != end(Ready_Queue_); ++iter) {
            std::cout << iter->getPID() << std::setw(9) << iter->getCPU_Usage() << std::setw(11) << iter->getAvgBurst() << std::setw(7) 
                      << iter->getSizeOfProcess() << std::setw(10) << iter->getLogicalStartAddress(); 
            std::cout << "   " << memory_unit_.CalculatePhysicalAddress(iter->getLogicalStartAddress(), iter->getPID()) << std::endl;
        }
        DisplayPageTablesReadyQueue(); 
        std::cout << std::endl;
    }
}

void Scheduler::DisplayPageTables(std::vector<std::deque<PCB> >& device, char first_letter, int which_device) const {
    std::cout << "   Page Tables for  " << first_letter << which_device << ": \n";
    for (auto iter = begin(device[which_device]); iter != end(device[which_device]); ++iter) 
        memory_unit_.DisplayPageTable( iter->getPID() );
    std::cout << "*****************\n";
}

void Scheduler::DisplayPageTablesReadyQueue() const {
    std::cout <<"   Page Tables for Processes in Ready Queue: \n";
    for (auto iter = Ready_Queue_.begin(); iter != Ready_Queue_.end(); ++iter)
        memory_unit_.DisplayPageTable( iter->getPID());
    std::cout << "*****************\n";
}

void Scheduler::DisplayHeader() const {
    std::cout << "PID|" << std::setw(10) << "Filename|" << std::setw(7) << "Len.|" << std::setw(5) << "R/W|" << std::setw(9) << "CPU Use|" 
    << std::setw(11) << "Avg Burst|" << std::setw(10) << "Cylinder|" << std::setw(6) << "Size|" << std::setw(5) << "Log|" << std::setw(6) 
    << "Phys|" << std::endl;
}

void Scheduler::ProcessSyscall(std::deque<PCB>& device_queue, std::string device_name, int device_num) {
    if (CPU_ == nullptr) {
        std::cout << "       The CPU is idle. Please add a task to the Ready Queue before requesting I/O\n";
        return;
    }
    std::cout << "   The process in the CPU has requested " << device_name << " I/O.\n";
    UpdatePCB_InCPU(device_name, device_num);
    PCB pcb = *CPU_; //Make copy of process in CPU
    device_queue.push_back(pcb); //Enqueue the copy of the updated PCB to the Device Queue it requested.
    delete CPU_; //Remove the process from the CPU
    CPU_ = nullptr;
    std::cout << "  Process from CPU has been added to Device Queue.\n";
    FillCPU(); //Fill CPU with next process in Ready Queue
} 

void Scheduler::DeviceInterrupt(std::deque<PCB>& device_queue, std::string device_name) {
    if (device_queue.empty()) {
        std::cout << "     There are no processes in this queue. \n"
                  << "     Please enter another command." << std::endl;
        return;
    }
    if (device_name == "disk") device_queue.front().setCylinder(-1); //Reset cylinder number once disk I/O has completed. 
    PCB ready_process = device_queue.front(); //Make copy of the front of device queue
    AddProcessToReadyQueue(ready_process); //Add this copy to the Ready Queue
    device_queue.pop_front(); //Delete this process from the device queue
}

void Scheduler::KillProcess(int the_PID) {
    std::cout << "   Request to kill P" << the_PID << " received.\n";
    //Find Process. First check CPU:
    if ( CPU_->getPID() == the_PID ) {
        TerminateProcessInCPU(); //Process to kill is in CPU. 
    }
    else if ( memory_unit_.ProcessInJobPool(the_PID) ) { //Next check Job Pool
        memory_unit_.KillProcessInJobPool(the_PID); //Delete this process
        LoadProcesses(); //Give newly available memory to next process in Job Pool
    }
    else if ( FindPCBAndKill_CheckReadyQ(the_PID) || FindPCBAndKill_CheckPrinters(the_PID) ||  //Check device Queues and Ready Queue. If found, kill process.
                FindPCBAndKill_CheckDisks(the_PID) || FindPCBAndKill_CheckCD_RW(the_PID) ) {
        LoadProcesses(); //Give newly available memory to next process in Job Pool
    }
    else 
        std::cout << "       No process with this PID is still in the system to Kill. Enter another command.\n";
}

bool Scheduler::FindPCBAndKill_CheckReadyQ(int the_PID) {
    auto rq_iter = Ready_Queue_.begin();
    while ( rq_iter != Ready_Queue_.end() ) { //Iteratively search Ready Queue
        if (rq_iter->getPID() == the_PID ) {
            PCB PCB_to_be_killed = *rq_iter; //Make a copy of PCB to be killed because std::multiset doesn't allow elements to be modified.
            TerminatingProcessAccounting(*rq_iter); //Update the copy
            Ready_Queue_.erase(rq_iter); //Delete original process from Ready Queue
            std::cout << "     P" << rq_iter->getPID() << " (located in Ready Queue) has been killed.\n";
            return true;
        }
        ++rq_iter;
    }
    return false;
}

bool Scheduler::FindPCBAndKill_CheckPrinters(int the_PID) {
    for (size_t i = 0; i < printer_.size(); ++i)
        for (auto iter = begin(printer_[i]); iter != end(printer_[i]); ++iter)
            if (iter->getPID() == the_PID) {
                std::cout << "     P" << iter->getPID() << " (located in printer " << i << ") has been killed.\n";
                TerminatingProcessAccounting(*iter); 
                printer_[i].erase(iter);
                return true;
            }
    return false;
}
bool Scheduler::FindPCBAndKill_CheckCD_RW(int the_PID) {
    for (size_t i = 0; i < CD_RW_.size(); ++i) {
        for (auto iter = begin(CD_RW_[i]); iter != end(CD_RW_[i]); ++iter) {
            if (iter->getPID() == the_PID) {
                std::cout << "     P" << iter->getPID() << " (located in CD_RW " << i << ") has been killed.\n";
                TerminatingProcessAccounting(*iter); 
                CD_RW_[i].erase(iter);
                return true;
            }
        }
    }
    return false;
}
bool Scheduler::FindPCBAndKill_CheckDisks(int the_PID) {
    for (size_t i = 0; i < disk_.size(); ++i) {
        for (auto iter = begin(disk_[i]); iter != end(disk_[i]); ++iter) {
            if (iter->getPID() == the_PID) {
                std::cout << "     P" << iter->getPID() << " (located in disk " << i << ") has been killed.\n";
                TerminatingProcessAccounting(*iter);
                disk_[i].erase(iter);
                return true;
            }
        }
    }
    return false;
}

void Scheduler::TerminatingProcessAccounting(const PCB& process_to_kill) {
    std::cout << "     Total CPU Usage for this process: " << process_to_kill.getCPU_Usage() << std::endl
              << "     Average burst for this process: " << process_to_kill.getAvgBurst() << std::endl;
    ++num_terminated_processes_;
    avg_CPU_usage_ = avg_CPU_usage_ * ((num_terminated_processes_-1)/num_terminated_processes_) + (process_to_kill.getCPU_Usage()/num_terminated_processes_); 
        
    memory_unit_.FreeMemory( process_to_kill.getPID() ); //Free the memory that had been assigned to this process. 
}

void Scheduler::AddProcessToReadyQueue(PCB& a_process) {
    Ready_Queue_.insert(a_process); 
    if (CPU_ == nullptr) { // The ready queue is empty and this process will go directly into the CPU 
        FillCPU();
        return;
    }
    std::cout << "     Process arriving to Ready Queue. P" << CPU_->getPID() <<" leaves CPU so that\n"
              << "     Interrupt can be handled. \n";
    UpdateAccountingInfo_Interrupt();
    PCB preempted_process = *CPU_; //make a copy of process in CPU and put it back into the Ready Queue
    Ready_Queue_.insert(preempted_process);
    delete CPU_;
    CPU_ = nullptr;
    FillCPU();
} //int num_pages, int page_size

void Scheduler::UpdatePCB_InCPU(std::string device_name, int device_num){
    CPU_->setFileNameFromUser();
    int num_pages_for_process = ceil( CPU_->getSizeOfProcess() / memory_unit_.GetPageSize() );
    CPU_->setStartAddressFromUser(num_pages_for_process, memory_unit_.GetPageSize());
    memory_unit_.DisplayPhysicalAddress( CPU_->getLogicalStartAddress(), CPU_->getPID() ); 
    if (device_name != "printer")
        CPU_->setRead_WriteFromUser();
    else //Printer I/O requested; write only
        CPU_->setRead_Write('w');
    CPU_->setFileLenFromUser();
    if (device_name == "disk") {
        int cyl = WhichCylinder(device_num);
        CPU_->setCylinder(cyl); //update PCB to hold this cylinder value.
    }
    UpdateAccountingInfo_Syscall(*CPU_);
}

int Scheduler::WhichCylinder(int device_num) {
    std::cout << "     There are " << num_cylinders_per_disk_[device_num-1] << " cylinders on this disk. \n"
              << "     Which cylinder do you want to access? ";
    int cyl = SchedulerNamespace::GetPositiveIntFromUser();
    while ((cyl <= 0 ) || (cyl > num_cylinders_per_disk_[device_num-1])) { //Make sure cylinder provided is not over or under the number of cylinders this disk has. 
        std::cout << "       Attemp to access invalid cylinder. Try again. ";
        cyl = SchedulerNamespace::GetPositiveIntFromUser();
    }
    return cyl; 
}

void Scheduler::UpdateAccountingInfo_Syscall(PCB& a_pcb) {
    std::cout << "     How long did this process use the CPU (ms)? ";
    int usage_last_burst = SchedulerNamespace::GetPositiveIntFromUser();
    a_pcb.UpdatePCBAfterSyscall(usage_last_burst, history_parameter_);
}

void Scheduler::UpdateAccountingInfo_Interrupt() {
    std::cout << "     How long did this process use the CPU (ms)? ";
    int usage_last_burst = SchedulerNamespace::GetPositiveIntFromUser();
    CPU_->UpdatePCBAfterInterrupt(usage_last_burst);
}

void Scheduler::LoadProcesses() {
    while ( memory_unit_.MemoryForWaitingProcesses() ) {
        PCB a_ready_pcb = memory_unit_.AssignMemoryToProcessInJobPool();  //assign to frame, take out of free frame list, and take out of job pool
        AddProcessToReadyQueue(a_ready_pcb);  //add to ready queue. 
    }
}












