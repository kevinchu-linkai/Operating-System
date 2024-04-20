#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Define the struct for a node in the linked list
struct node_t {
    int tickets;            // Number of tickets for this job
    int count;              // Original number of tickets for this job
    struct node_t *next;    // Pointer to the next node
};

// Global variables
int gtickets = 0;           // Total number of tickets
struct node_t *head = NULL; // Head of the linked list

// Function prototypes
void insert(int tickets);
void print_list();
// Add other function prototypes as needed

void remove_job(struct node_t *prev, struct node_t *to_remove) {
    if (prev != NULL) {
        prev->next = to_remove->next;
    } else {
        head = to_remove->next;  // Update head if the head is to be removed
    }
    free(to_remove);  // Free the memory allocated to the node
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: lottery <seed> <loops>\n");
        exit(1);
    }

    int seed = atoi(argv[1]);
    int loops = atoi(argv[2]);
    srandom(seed);

    insert(70);
    insert(90);
    insert(35);
    print_list();

    for (int i = 0; i < loops; i++) {
        if (gtickets == 0) {
            printf("All tickets have been exhausted.\n");
            break;
        }
        int winner_ticket = random() % gtickets;
        int cumulative_tickets = 0;
        struct node_t *current = head, *prev = NULL;

        while (current != NULL) {
            cumulative_tickets += current->tickets;
            if (cumulative_tickets > winner_ticket) {
                current->count--;  // Decrement the ticket count for the winner
                gtickets--;  // Decrement the global ticket count
                printf("Winner: %d (Job with %d tickets)\n", winner_ticket, current->tickets);
                if (current->count <= 0) {
                    remove_job(prev, current);
                }
                break;
            }
            prev = current;
            current = current->next;
        }
        print_list();
    }
    return 0;
}


// Function to insert a new job into the linked list
void insert(int tickets) {
    struct node_t *new_node = (struct node_t*) malloc(sizeof(struct node_t));
    assert(new_node != NULL);  // Ensure the node was allocated
    new_node->tickets = tickets;
    new_node->count = tickets;
    new_node->next = head;
    head = new_node;
    gtickets += tickets;  // Update the total number of tickets
}

void print_list() {
    struct node_t *current = head;
    printf("Current List:\n");
    while (current != NULL) {
        printf("Job with %d tickets (Remaining: %d)\n", current->tickets, current->count);
        current = current->next;
    }
}

