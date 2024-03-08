#include <stdio.h>

// Structure to represent a process
typedef struct {
    int process_id;       // Process ID
    int arrival_time;     // Arrival time of the process
    int burst_time;       // Burst time (execution time) of the process
    int waiting_time;     // Waiting time of the process
    int turnaround_time;  // Turnaround time of the process
    int completion_time;  // Completion time of the process
    int service_time;     // Service time of the process
} Process;

// Function to calculate waiting time, turnaround time, completion time, and service time for each process
void calculateTimes(Process processes[], int n) {
    // To Do
}


int main() {
    // Define sample processes
    Process processes[4] = {
        {1, 0, 6}, // Process ID, Arrival Time, Burst Time
        {2, 2, 4},
        {3, 4, 2},
        {4, 6, 5}
    };

    // Calculate waiting time, turnaround time, completion time, and service time
    calculateTimes(processes, 4);

    return 0;
}
