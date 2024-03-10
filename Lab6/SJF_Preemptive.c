#include <stdio.h>
#include <limits.h>

// Structure to represent a process
typedef struct {
    int process_id;
    int arrival_time;
    int burst_time;
    int remaining_time; // For tracking the remaining burst time
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int started_at; // To track when the process started for the first time
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

void calculateSJFPreemptiveTimes(Process processes[], int n) {
    int total_time = 0, completed = 0, currentTime = 0;
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        total_time += processes[i].burst_time;
        processes[i].started_at = -1; // Initialize with -1 indicating not started
    }
    while (completed != n) {
        int index = findNextProcess(processes, n, currentTime);
        if (index != -1) { // Process found to execute
            if (processes[index].started_at == -1) {
                processes[index].started_at = currentTime; // Mark start time if not started
            }
            processes[index].remaining_time--;
            if (processes[index].remaining_time == 0) {
                completed++;
                processes[index].completion_time = currentTime + 1;
                processes[index].turnaround_time = processes[index].completion_time - processes[index].arrival_time;
                processes[index].waiting_time = processes[index].turnaround_time - processes[index].burst_time;
            }
        }
        currentTime++;
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
    // The SJF Preemptive scheduling times for each process are calculated
    calculateSJFPreemptiveTimes(processes, 4);
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
