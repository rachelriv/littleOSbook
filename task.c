#include "task.h"
#include "paging.h"

// The currently running task
volatile task_t * current_task;

// Pointer to the head of the task linked list.
volatile task_t * ready_queue;

extern page_directory_t * kernel_directory;
extern page_directory_t * current_directory;
extern void alloc_frame(page_t *, int, int);
extern uint32_t initial_esp;
extern uint32_t read_eip();

// The next available PID
uint32_t next_pid = 1;

void initialise_tasking() {
    asm volatile("cli");
    // Relocate stack
    move_stack((void*)0xE0000000, 0x2000);
    
    // Initialise the kernel task
    current_task = ready_queue = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = 0;

    // Reenable interrupts
    asm volatile("sti");
}

void switch_task() {
     if (!current_task) {
          return;
     }
     
     uint32_t esp, ebp, eip;
     asm volatile("mov %%esp, %0" : "=r" (esp));
     asm volatile("mov %%ebp, %0" : "=r" (ebp));
     /* We read the instruction pointer.
      * When this function exits, one of two things can happen
      * a) We just called the function and it returned the EIP as requested
      * b) We have just switched tasks and it will seems as if read_eip
      *    has just returned, since the saved EIP is the instruction right
      *    after the read_eip() function call.
      * If we are in case b), we need to return immediatel. We use a dummy 
      * value in EAX to detect this. When C returns values in EAX, it will 
      * seem as if the return calue is the dummy value 0x12345.
      */
      eip = read_eip();

      if (eip == 0x12345){
          // Return if we just switched tasks.
          return; 
      }
      // Else, we save register values and switch.
      current_task->eip = eip;
      current_task->esp = esp;
      current_task->ebp = ebp;

      current_task = current_task->next;
      // If we fell of the end of the list, go to the beginning.
      if (!current_task) {
          current_task = ready_queue;
      }

      eip = current_task->eip;
      esp = current_task->esp;
      ebp = current_task->ebp;
      
      // Make sure the memory manager knows we've changed
      // page directory
      current_directory = current_task->page_directory;
      asm volatile("          \
         cli;                 \
         mov %0, %%ecx;       \
         mov %1, %%esp;       \
         mov %2, %%ebp;       \
         mov $0x12345, %%eax; \
         sti;
         jmp *%%ecx           "
      : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_directory->physicalAddress));
}

// This will clone the current existing process/task
int fork() {
    // Disable the interrupts
    asm volatile("cli");
    // Keep pointer to this original task
    task_t * parent_task = (task_t*) current_task;

    // Clone the address space.
    page_directory_t * directory = clone_directory(current_directory);

    // Create a new process
    task_t * new_task = (task_t*)kmalloc(sizeof(task_t));
    new_task->id = next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    new_task->next = 0;

    // Add it to the end of the ready queue.
    // Find the end of the ready queue
    task_t * tmp_task = (task_t*)ready_queue;
    while (tmp_task->next) {
        tmp_task = tmp_task->next;
    }
    tmp_task->next = new_task;

    // Entry point for new process
    uint32_t eip = read_eip();
    // Check if we are the parent or child
    if (current_task == parent_task) {
        // If we are the parent, we set up esp/ebp/eip for the child
        uint32_t esp; asm volatile("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp; asm volatile("mov %%ebp, %0" : "=r"(esp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        // Reenable interrupts
        asm volatile("sti"); 
        return new_task->id;
    } else {
        // We are the child, so return 0
        return 0;
    }
}

void switch_to_user_mode() {

    
    // Set up a stack structure for switching to user mode.
    
    // Recall that our user mode segments in the GDT are
    // 0x18 and 0x20 for user code segment and user data segment,
    // respectively. However, the last two bits of the selector
    // are used in Intel to represent the Requested Privilege Level,
    // which is a 3 (11). Therefore, our segments need to be ORed with
    // 3 in order to be correct.

    // User Code Segment Selector: (0x18 | 0x3 = 0x1B)
    // User Date Segment Selector: (0x20 | 0x3 = 0x23)
    asm volatile(" "
      "cli;" 
      "mov $0x23, %ax;" 
      "mov %ax, %ds;" 
      "mov %ax, %es;" 
      "mov %ax, %fs;" 
      "mov %ax, %gs;" 
      " " 
      "mov %esp, %eax;" 
      "pushl $0x23;" 
      "pushl %eax;" 
      "pushf;"
      "pop %eax;" // Get EFLAGS back into EAX. The only way to read EFLAGS is to pushf then pop.
      "or %eax, 0x200;" // Set the IF flag.
      "push %eax;" // Push the new EFLAGS value back onto the stack.
      "pushl $0x1B;"
      "push $1f;"
      "iret;"
    "1:"
    );
}

void move_stack(void *new_stack_start, uint32_t size) {
    uint32_t i;
    uint32_t max = ((u32int)new_stack_start-size);
    // Allocate space for new stack.
    for(i = (uint32_t)new_stack_start; i >= max; i -= 0x1000) {
        alloc_frame( get_page(i, 1, current_directory), 0, 1); // In user mode and is writeable.
    }

    // Flush the translation lookaside buffer by writting to cr3.
    uint32_t pd_addr;
    asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
    asm volatile("mov %0, %%cr3" : : "r" (pd_addr));
    // Calculate new pointers with by using the old pointers
    // and an offset
    uint32_t old_stack_ptr; asm volatile("mov %%esp, %0" : "=r" (old_stack_ptr));
    uint32_t old_base_ptr; asm volatile("mov %%ebp, %0" : "=r" (old_base_ptr));
    uint32_t offset = (uint32_t)new_stack_start - initial_esp;

    uint32_t new_stack_ptr = old_stack_ptr + offset;
    uint32_t new_base_ptr = old_base_ptr + offset;
    // Copy the stack
    memcpy((void*)new_stack_ptr, (void*)old_stack_ptr, initial_esp - old_stack_ptr);

    // Trace through the original stack and copy new values into the new stack
    for(i = (uint32_t)new_stack_start; i > (uint32_t)new_stack_start-size; i -= 4) {
        uint32_t tmp = * (uint32_t *)i;
        if ((old_stack_ptr < tmp) && (tmp < initial_bsp)) {
            tmp += offset;
            uint32_t *tmp2 = (u32int*)i;
            *tmp2 = tmp;
        }
    }
    asm volatile("mov %0, %%esp" : : "r" (new_stack_ptr));
    asm volatile("mov %0, %%ebp" : : "r" (new_base_ptr));
}
