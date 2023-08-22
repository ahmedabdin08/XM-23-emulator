#pragma once
/*Header for priority queeu
* Author: Ahmed
*/
#define PRIORITYLEVELS 8

typedef struct node {
    void* data;
    int priority;
    struct node* next;
    struct node* prev;
} node_t;

typedef struct priority_queue {
    node_t* head;
    node_t* tail;
    int count;
} priority_queue_t;

extern priority_queue_t* MLQ[PRIORITYLEVELS];//multilevel queue for the pending interrupts
//ordered by priority first and then by the interrupt vector table index

priority_queue_t* initialize_pq();
void enqueue(priority_queue_t* q, void* data, int priority);
void* peek(priority_queue_t* q);
void dequeue(priority_queue_t* q);
void* check_pending_interrupts();
void initialize_MLQ(priority_queue_t* q[]);
int is_empty(priority_queue_t* q);