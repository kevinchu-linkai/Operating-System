#include <stdio.h>

typedef struct {
    int process_id;
    int arrival_time;
    int burst_time;
    int remaining_time; // For tracking the remaining burst time
    int completion_time;
    int waiting_time;
    int turnaround_time;
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

void calculateRoundRobinTimes(Process processes[], int n, int quantum) {
    int total_time = 0;
    for (int i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        total_time += processes[i].burst_time;
    }

    int current_time = 0, processed = 0;
    while (total_time > 0) {
        for (int i = 0; i < n; i++) {
            if (processes[i].remaining_time > 0 && processes[i].arrival_time <= current_time) {
                int time_slice = (processes[i].remaining_time > quantum) ? quantum : processes[i].remaining_time;
                processes[i].remaining_time -= time_slice;
                current_time += time_slice;
                total_time -= time_slice;

                if (processes[i].remaining_time == 0) {
                    processes[i].completion_time = current_time;
                    processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
                    processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
                }
            }
        }
    }
}

int main() {
    Process processes[4] = {
        {1, 0, 8}, // Process ID, Arrival Time, Burst Time for Round Robin
        {2, 2, 5},
        {3, 4, 6},
        {4, 6, 7}
    };

    int quantum = 4;
    // Sort processes by arrival time
    sortProcessesByArrival(processes, 4);
    // The Round Robin scheduling times for each process are calculated
    calculateRoundRobinTimes(processes, 4, quantum);
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
