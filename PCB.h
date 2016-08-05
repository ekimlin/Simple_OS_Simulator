/*
Author: Emma Kimlin
Title: PCB.h
Date Created: 3.2.2016
Description: PCB (Process Control Block) symbolizes a task that needs to be processed by the CPU. 
Modifications:
    3.5.2016 Added private member variables needed for I/O request. 
                       Added setters. Added print()
    3.7.2016 Added getPID()
    4.8.2016 Added total_CPU_usage and avg_burst_time and functions to update them. Modified print() to include these new variables.
    4.9.2016 Added expected_next_burst and num_burst and functions to update them. Added setters that query user.
    4.13.2016 Added cylinder_ 
    5.6.2016 Added num_words_
    5.9.2016 Modified setStartAddressFromUser() to only accept hexidecimal input. 
*/

#ifndef PCB_h
#define PCB_h
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

class PCB {
public:
    PCB(int pid, double next_burst, size_t the_size);
    /* Setters that query the user. */
    void setStartAddressFromUser(int num_pages, int page_size);
    void setFileNameFromUser();
    void setRead_WriteFromUser();
    void setFileLenFromUser();
    /* Setters that do not query user */
    void setRead_Write (char rw);
    void setFileLen (int len);
    void setCylinder(int a_cylinder);
    /* Getters */
    int getPID() const { return PID_; }
    int getLogicalStartAddress() const { return logical_start_address_; }
    int getRead_Write() const { return read_write_; }
    int getFileLen() const { return file_length_; }
    std::string getFileName() const { return filename_; }
    double getCPU_Usage() const { return total_CPU_usage_; }
    double getAvgBurst() const { return avg_burst_time_; }
    double getExpectedNextBurstTotal() const { return expected_next_burst_total_; }
    double getExpectedNextBurstRemaining() const { return expected_next_burst_remaining_;}
    double getSizeOfProcess() const { return size_of_process_; }
    /* Print all data members of a PCB. Output spacing is formatted and ends in a new line. */
    void Print();

    friend bool operator<(const PCB& lhs, const PCB& rhs) {
        return lhs.expected_next_burst_remaining_ < rhs.expected_next_burst_remaining_ ? true : false;
    }
    friend bool operator>(const PCB& lhs, const PCB& rhs) {
        return lhs.expected_next_burst_remaining_ > rhs.expected_next_burst_remaining_ ? true : false;
    }
    friend bool operator==(const PCB& lhs, const PCB& rhs) {
        return lhs.expected_next_burst_remaining_ == rhs.expected_next_burst_remaining_ ? true : false;
    }

    void UpdatePCBAfterSyscall(double CPU_usage, double his_param)   {
        num_bursts_++;
        CPU_usage_this_burst_ += CPU_usage; 
        total_CPU_usage_ += CPU_usage_this_burst_;
        avg_burst_time_ = total_CPU_usage_/num_bursts_; 
        expected_next_burst_total_ = ((1 - his_param) * expected_next_burst_total_) + (his_param * CPU_usage_this_burst_);
        //Re-set for when the PCB is done with I/O burst and needs to go to Ready Queue:
        expected_next_burst_remaining_ = expected_next_burst_total_;
        CPU_usage_this_burst_ = 0; 
}

    void UpdatePCBAfterInterrupt(double usage_last_burst) {
        CPU_usage_this_burst_ += usage_last_burst;
        expected_next_burst_remaining_ -= usage_last_burst;
    }

    void UpdateBeforeKill() {
        num_bursts_++;
        total_CPU_usage_ += CPU_usage_this_burst_;
        avg_burst_time_ = total_CPU_usage_/num_bursts_;
    }

private:
    int PID_;
    int logical_start_address_;
    char read_write_;
    int file_length_;
    std::string filename_;
    double total_CPU_usage_; //total time process has used CPU
    double CPU_usage_this_burst_; //time process has used CPU during current burst; must be reset to 0 when burst finishes 
    double avg_burst_time_;
    double expected_next_burst_total_; 
    double expected_next_burst_remaining_; //Used by the Ready Queue Scheduler to implement SJF Scheduling 
    double num_bursts_;
    int cylinder_; //will be set to -1 if process is not requesting access to disk I/O
    size_t size_of_process_; 
};

#endif
