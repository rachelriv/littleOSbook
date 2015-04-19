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
