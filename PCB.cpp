#include "PCB.h"
#include <stdlib.h>

/********************Utility Functions********************/
namespace PCBNamespace {

//Returns positive integer provided by user.
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

} //end PCBNamespace

/********************Public Member Functions********************/

PCB::PCB(int pid, double init_burst_estimate, size_t the_size) {
    PID_ = pid;
    logical_start_address_ = 0;
    read_write_ = '-';
    file_length_ = 0;
    filename_ = "--";
    total_CPU_usage_ = 0.0;
    CPU_usage_this_burst_ = 0.0;
    avg_burst_time_ = 0.0;
    expected_next_burst_total_ = init_burst_estimate;
    expected_next_burst_remaining_ = expected_next_burst_total_; 
    num_bursts_ = 0.0;
    cylinder_ = -1;
    size_of_process_ = the_size; 
}
void PCB::setStartAddressFromUser(int num_pages, int page_size) {
    bool good_input = false;
    std::cout << "     What is the start address in memory? Enter a hexdecimal number. ";
    int address_in_hex;
    std::string address_str;

    std::cin >> address_str;
    while (good_input == false) {
        while ( address_str.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) { //make sure user input contains only hex values
            std::cout << "       Invalid input. Please enter a hexidecimal address. ";
            std::cin >> address_str;
        }
        std::stringstream address_ss(address_str);
        address_ss >> std::hex >> address_in_hex; 
        logical_start_address_ = address_in_hex;

        std::div_t divresult = std::div(logical_start_address_, page_size);
        int page_number = divresult.quot;
        if (page_number > num_pages) {
            std::cout << "       Invalid input. Please enter a hexidecimal address for a process with " << num_pages << " pages. ";
            std::cin >> address_str;    
        } else good_input = true;
    }   
     
}

void PCB::setFileNameFromUser() {
    std::string filename;
    std::cout << "     What is the filename? ";
    std::cin >> filename;
    filename_ = filename;
}

void PCB::setFileLenFromUser() {
    std::cout << "     What is the length of the file to write to? ";
    int length = PCBNamespace::GetPositiveIntFromUser();
    while ( length < 0) {
        std::cout << "       File length cannot be negative. Try Again. \n";
        length = PCBNamespace::GetPositiveIntFromUser();
    }
    file_length_ = length;
} 

void PCB::setRead_WriteFromUser() {
    char input;
    std::cout << "     Is this a Read or Write request? Type 'r' or 'w'. ";
    std::cin >> input;
    while ( (input != 'r') && (input != 'w') ) {
        std::cout << "       Read_Write of a PCB only accepts 'r' or 'w'. Try again. \n";
        std::cin >> input;
    }
    read_write_ = input; 
}

void PCB::setRead_Write(char rw) {
    while ( (rw != 'r') && (rw != 'w') ) {
        std::cout << "       Read_Write of a PCB only accepts 'r' or 'w'. Try again. \n";
        std::cin >> rw;
    }
    read_write_ = rw; 
}

void PCB::setFileLen(int length) {
    while ( length < 0) {
        std::cout << "       File Length cannot be negative. Try again. \n";
        length = PCBNamespace::GetPositiveIntFromUser();
    }
    file_length_ = length;
} 

void PCB::setCylinder(int a_cylinder) {
    while (a_cylinder < -1) {
        std::cout << "       Cylinder cannot be negative. Try again.\n" ;
        a_cylinder = PCBNamespace::GetPositiveIntFromUser();
    }
    cylinder_ = a_cylinder;
}
    
void PCB::Print() {
    std::cout << PID_ << std::setw(10) << filename_ << std::setw(7) << file_length_ << std::setw(5) << read_write_ 
      		  << std::setw(9) << total_CPU_usage_ << std::setw(11) << avg_burst_time_ << std::setw(10);
    if (cylinder_ == -1)
        std::cout << "N/A" << std::setw(6) << std::dec << size_of_process_ << std::setw(6) << std::hex << logical_start_address_ << "  ";
    else 
        std::cout << cylinder_ << std::setw(6) << std::dec << size_of_process_ << std::setw(6) << std::hex << logical_start_address_ << "  ";
}











