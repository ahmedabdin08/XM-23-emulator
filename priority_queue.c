#include "priority_queue.h"
#include <stdlib.h>
#include <assert.h>

priority_queue_t* MLQ[PRIORITYLEVELS] = { NULL };

void initialize_MLQ(priority_queue_t* q[]) {
    for (int i = 0; i < PRIORITYLEVELS; i++) {
        q[i] = initialize_pq();
    }
}

priority_queue_t* initialize_pq() {
    priority_queue_t* q = malloc(sizeof(priority_queue_t));
    assert(q);
    q->tail = q->head = NULL;
    q->count = 0;
    return q;
}

int is_empty(priority_queue_t* q) {
    return (q->count == 0);
}

void enqueue(priority_queue_t* q, void* data, int priority) {
    node_t* node = malloc(sizeof(node_t));
    assert(node);
    node->data = data;
    node->priority = priority;
    node->next = node->prev = NULL;

    if (is_empty(q)) {
        q->tail = q->head = node;
    }
    else {
        node_t* current = q->tail;
        while (current != NULL && current->priority >= priority)
            current = current->next;

        if (current == q->tail) {
            node->next = q->tail;
            q->tail->prev = node;
            q->tail = node;
        }
        else if (current == NULL) {
            node->prev = q->head;
            q->head->next = node;
            q->head = node;
        }
        else {
            node->prev = current->prev;
            node->next = current;
            current->prev->next = node;
            current->prev = node;
        }
    }
    q->count++;
}

void* peek(priority_queue_t* q) {
    assert(!is_empty(q));
    return q->head->data;
}

void dequeue(priority_queue_t* q) {
    assert(!is_empty(q));
    node_t* node = q->head;
    if (q->tail == q->head) {
        q->tail = q->head = NULL;
    }
    else {
        q->head = q->head->prev;
        q->head->next = NULL;
    }
    free(node);
    q->count--;
}


void* check_pending_interrupts() {
    for (int i = PRIORITYLEVELS - 1; i >= 0; i--) { // Check from highest to lowest priority
        if (!is_empty(MLQ[i])) {
            return peek(MLQ[i]); // Return the highest-priority pending interrupt
        }
    }
    return NULL; // No pending interrupts
}
