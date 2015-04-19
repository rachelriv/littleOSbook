#include "syscall.h"
#include "isr.h"
#include "framebuffer.h"

DECL_SYSCALL1(fb_write_str, char *buf);
DECL_SYSCALL0(fb_clear);

static void syscall_handler(registers_t *regs);

static void *syscalls[2] = {
    &fb_write_str,
    &fb_clear
};

static int32_t num_syscalls = 2;

void initialise_syscalls() {
    register_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t *regs) {
    // Check that the requested system call number is valid.
    if (regs->eax >= num_syscalls){
        return;
    }
    
    // This is the location of the system call.
    void * location = syscalls[regs->eax];

    // We don't know how many parameters the function wants, so we just
   // push them all onto the stack in the correct order. The function will
   // use all the parameters it wants, and we can pop them all back off afterwards.
   int ret;
   asm volatile (" \ 
     push %1; \ 
     push %2; \ 
     push %3; \ 
     push %4; \ 
     push %5; \ 
     call *%6; \ 
     pop %%ebx; \ 
     pop %%ebx; \ 
     pop %%ebx; \ 
     pop %%ebx; \ 
     pop %%ebx; \ 
   " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
   regs->eax = ret;
}
