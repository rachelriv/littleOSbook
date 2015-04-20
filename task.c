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
