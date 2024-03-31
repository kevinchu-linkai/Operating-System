#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100

// Process structure
typedef struct {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int priority;
    int completionTime;
    int turnaroundTime;
    int boostTime;
} Process;

// Global process list and counters
Process processList[MAX_PROCESSES];
int processCount = 0;
int currentTime = 0;
int totalExecutionTime = 0;

// Function prototypes
void addProcess(int id, int arrivalTime, int burstTime);
void executeMLFQ();
void printProcesses();
void sortProcessList_ID();

int main() {
    // Sample process addition
    addProcess(1, 0, 6);
    addProcess(2, 0, 14);
    addProcess(3, 0, 30);
    addProcess(4, 0, 65);
    addProcess(5, 103, 2);
    
    executeMLFQ();
    printProcesses();

    return 0;
}

void printProcesses() {
    sortProcessList_ID();
    printf("Process execution summary:\n");
    for (int i = 0; i < processCount; i++) {
        printf("Process ID: %d, Arrival Time: %d, Burst Time: %d, Completion Time: %d, Turnaround Time: %d\n",
               processList[i].id, processList[i].arrivalTime, processList[i].burstTime,
               processList[i].completionTime, processList[i].turnaroundTime);
    }
}

void printSchedulerState(int currentTime) {
    // Assuming a maximum of 3 priorities for illustration purposes
    printf("t = %d\n", currentTime);
    printf("Q1 ");
    for (int i = 0; i < processCount; i++) {
        if (processList[i].priority == 1 && processList[i].remainingTime > 0 && processList[i].arrivalTime <= currentTime) {
            printf("P%d(%d) ", processList[i].id, processList[i].remainingTime);
        }
    }
    printf("\nQ2 ");
    for (int i = 0; i < processCount; i++) {
        if (processList[i].priority == 2 && processList[i].remainingTime > 0 && processList[i].arrivalTime <= currentTime) {
            printf("P%d(%d) ", processList[i].id, processList[i].remainingTime);
        }
    }
    printf("\nQ3 ");
    for (int i = 0; i < processCount; i++) {
        if (processList[i].priority == 3 && processList[i].remainingTime > 0 && processList[i].arrivalTime <= currentTime) {
            printf("P%d(%d) ", processList[i].id, processList[i].remainingTime);
        }
    }
    printf("\n\n");
}


void addProcess(int id, int arrivalTime, int burstTime) {
    if (processCount < MAX_PROCESSES) {
        processList[processCount].id = id;
        processList[processCount].arrivalTime = arrivalTime;
        processList[processCount].burstTime = burstTime;
        processList[processCount].remainingTime = burstTime;
        processList[processCount].priority = 1; // Example priority assignment
        processList[processCount].completionTime = 0;
        processList[processCount].turnaroundTime = 0;
        processList[processCount].boostTime = 0;
        totalExecutionTime += burstTime;
        processCount++;
    } else {
        printf("Maximum process limit reached.\n");
    }
}

void sortProcessList() {
    for (int i = 0; i < processCount - 1; i++) {
        for (int j = 0; j < processCount - i - 1; j++) {
            if (processList[j].boostTime > processList[j + 1].boostTime) {
                Process temp = processList[j];
                processList[j] = processList[j + 1];
                processList[j + 1] = temp;
            }
        }
    }
}

void sortProcessList_ID() {
    for (int i = 0; i < processCount - 1; i++) {
        for (int j = 0; j < processCount - i - 1; j++) {
            if (processList[j].id > processList[j + 1].id) {
                Process temp = processList[j];
                processList[j] = processList[j + 1];
                processList[j + 1] = temp;
            }
        }
    }
}

void executeMLFQ() {
    printf("MLFQ Scheduler Execution...\n");
    const int Q1_quantum = 8;
    const int Q2_quantum = 15;
    int lastBoostTime = 0;
    const int boostInterval = 110;
    int processExecuted = 0; // To track the number of processes executed

    // Main execution loop
    while (processExecuted < processCount) {
        int executedThisCycle = 0;

        sortProcessList();
        // First, try to execute all processes in Q1
        for (int i = 0; i < processCount; i++) {
            if (processList[i].remainingTime > 0 && processList[i].priority == 1) {
                int executionTime = (processList[i].remainingTime < Q1_quantum) ? processList[i].remainingTime : Q1_quantum;
                if (currentTime >= processList[i].arrivalTime) {
                    if(currentTime + executionTime > lastBoostTime + boostInterval) {
                        processList[i].priority = 1;
                        processList[i].boostTime++;
                        executedThisCycle = 1;
                        sortProcessList();
                        printSchedulerState(currentTime);
                        break; // Execute one process at a time
                    }
                    processList[i].remainingTime -= executionTime;
                    currentTime += executionTime;
                    
                    if (processList[i].remainingTime <= 0) {
                        processList[i].completionTime = currentTime;
                        processList[i].turnaroundTime = processList[i].completionTime - processList[i].arrivalTime;
                        processExecuted++;
                    } else {
                        // Move to the next lower priority queue if not finished
                        processList[i].priority = 2;
                    }
                    
                    executedThisCycle = 1;
                    printSchedulerState(currentTime);
                    break; // Execute one process at a time
                }
            }
        }

        // Execute processes in Q2 if no Q1 processes were executed
        if (!executedThisCycle) {
            for (int i = 0; i < processCount; i++) {
                if (processList[i].remainingTime > 0 && processList[i].priority == 2) {
                    int executionTime = (processList[i].remainingTime < Q2_quantum) ? processList[i].remainingTime : Q2_quantum;
                    if (currentTime >= processList[i].arrivalTime) {
                        if(currentTime + executionTime > lastBoostTime + boostInterval) {
                            processList[i].priority = 1;
                            processList[i].boostTime++;
                            executedThisCycle = 1;
                            sortProcessList();
                            printSchedulerState(currentTime);
                            break; // Execute one process at a time
                        }
                        processList[i].remainingTime -= executionTime;
                        currentTime += executionTime;

                        if (processList[i].remainingTime <= 0) {
                            processList[i].completionTime = currentTime;
                            processList[i].turnaroundTime = processList[i].completionTime - processList[i].arrivalTime;
                            processExecuted++;
                        } else {
                            // Keep in the same queue or move to Q3 based on your policy
                            processList[i].priority = 3;
                        }

                        executedThisCycle = 1;
                        printSchedulerState(currentTime);
                        break; // Execute one process at a time
                    }
                }
            }
        }

        // Execute processes in Q3 if no Q1 or Q2 processes were executed
        if (!executedThisCycle) {
            for (int i = 0; i < processCount; i++) {
                if (processList[i].remainingTime > 0 && processList[i].priority == 3) {
                    if (currentTime >= processList[i].arrivalTime) {
                        if(currentTime + processList[i].remainingTime > lastBoostTime + boostInterval) {
                            int executionTime = lastBoostTime + boostInterval - currentTime;
                            processList[i].remainingTime -= executionTime;
                            currentTime += executionTime;
                            processList[i].priority = 1;
                            processList[i].boostTime++;
                            executedThisCycle = 1;
                            sortProcessList();
                            printSchedulerState(currentTime);
                            break; // Execute one process at a time
                        }
                        currentTime += processList[i].remainingTime;
                        processList[i].completionTime = currentTime;
                        processList[i].turnaroundTime = processList[i].completionTime - processList[i].arrivalTime;
                        processList[i].remainingTime = 0;
                        processExecuted++;

                        executedThisCycle = 1;
                        printSchedulerState(currentTime);
                        break; // Execute one process at a time
                    }
                }
            }
        }

        // Boosting mechanism
        if (currentTime >= lastBoostTime + boostInterval) {
            for (int i = 0; i < processCount; i++) {
                if (processList[i].priority != 1) {
                    processList[i].priority = 1; // Boost all processes back to high priority
                    processList[i].boostTime++;
                    sortProcessList();
                }
            }
            lastBoostTime = currentTime;
        }

        // If no process was executed in this cycle, advance time to simulate waiting
        if (!executedThisCycle) {
            currentTime++;
        }
    }
}

