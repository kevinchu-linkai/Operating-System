#include <stdio.h>
#include <limits.h>

// Structure to represent a process
typedef struct {
    int process_id;       // Process ID
    int arrival_time;     // Arrival time of the process
    int burst_time;       // Burst time (execution time) of the process
    int waiting_time;     // Waiting time of the process
    int turnaround_time;  // Turnaround time of the process
    int completion_time;  // Completion time of the process
    int remaining_time;
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

// Scans the processes array to identify the most urgent executable process based on SJF criteria.
int findNextProcess(Process processes[], int n, int currentTime) {
    int minTime = INT_MAX, index = -1;
    for (int i = 0; i < n; i++) {
        if (processes[i].arrival_time <= currentTime && processes[i].remaining_time < minTime && processes[i].remaining_time > 0) {
            minTime = processes[i].remaining_time;
            index = i;
        }
    }
    return index;
}

void calculateSJFNonPreemptiveTimes(Process processes[], int n) {
    int total_time = 0, completed = 0, currentTime = 0;
    // Initialize remaining times
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        total_time += processes[i].burst_time;
    }

    while (completed != n) {
        int index = findNextProcess(processes, n, currentTime);
        if (index != -1) { // Process found to execute
            currentTime += processes[index].burst_time;
            processes[index].completion_time = currentTime;
            processes[index].turnaround_time = processes[index].completion_time - processes[index].arrival_time;
            processes[index].waiting_time = processes[index].turnaround_time - processes[index].burst_time;
            processes[index].remaining_time = 0; // Mark as completed
            completed++;
        } else { // No process is ready to execute at the current time
            currentTime++;
        }
    }
}

int main() {
    // Define sample processes
    Process processes[4] = {
        {1, 0, 6}, // Process ID, Arrival Time, Burst Time
        {2, 2, 3},
        {3, 4, 2},
        {4, 6, 8}
    };

    // Sort processes by arrival time
    sortProcessesByArrival(processes, 4);
    // The SJF NonPreemptive scheduling times for each process are calculated
    calculateSJFNonPreemptiveTimes(processes, 4);
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
