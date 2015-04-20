#ifndef TASK_H
#define TASK_H

#include "paging.h"
#include <stdint.h>

// This structure describes a task.
// This is part of a linked list, so
// each task points to the next one.
typedef struct task {
    int id;
    uint32_t esp, ebp;   // Stack and base pointers
    uint32_t eip;        // Instruction pointer
    page_directory_t * page_directory; // Page directory
    struct task * next;
} task_t;

void initialise_tasking();

// Used with a timer.
void task_switch();

// Forks a current process, creating a new one with different memory space
int fork();

int get_process_id();

void switch_to_user_mode();

#endif
