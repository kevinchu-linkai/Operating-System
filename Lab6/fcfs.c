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

// Implementation of the bubble sort algorithm to sort the processes array by arrival time.
// This ensures the scheduler considers processes in the order they arrive.
void sortProcessesByArrival(Process processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                // Swap processes[j] and processes[j + 1]
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}
    
void calculateFCFSTimes(Process processes[], int n) {
    // Initial setup for the first process.
    // The first process's service time is its arrival time, it has no waiting time,
    // and its completion and turnaround times are calculated directly from its burst time.
    processes[0].service_time = processes[0].arrival_time;
    processes[0].waiting_time = 0;
    processes[0].completion_time = processes[0].arrival_time + processes[0].burst_time;
    processes[0].turnaround_time = processes[0].burst_time;

    for (int i = 1; i < n; i++) {
        processes[i].service_time = processes[i-1].completion_time;// For each subsequent process, calculate its service time based on the completion time of the previous process.
        processes[i].waiting_time = processes[i].service_time - processes[i].arrival_time;// The waiting time is the difference between its service time and its arrival time.
        processes[i].completion_time = processes[i].service_time + processes[i].burst_time;// The completion time is the sum of its service time and burst time.
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;// The turnaround time is the difference between its completion time and arrival time.
    }
}

int main() {
    // Define sample processes
    Process processes[4] = {
        {1, 0, 10}, // Process ID, Arrival Time, Burst Time
        {2, 2, 3},
        {3, 4, 7},
        {4, 6, 5}
    };

    // Sort processes by arrival time
    sortProcessesByArrival(processes, 4);
    // The FCFS scheduling times for each process are calculated
    calculateFCFSTimes(processes, 4);
    // Output the results
    printf("ProcessID\tArrival Time\tBurst Time\tWaiting Time\tTurnaround Time\tCompletion Time\n");
    for (int i = 0; i < 4; i++) {
        printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
            processes[i].process_id,
            processes[i].arrival_time,
            processes[i].burst_time,
            processes[i].waiting_time,
            processes[i].turnaround_time,
            processes[i].completion_time);
    }
    return 0;
}
