#include "error.h"
#include "string.h"
extern void error(const char *message, const char *file, uint32_t line){
    // We encountered a massive problem and have to stop.
    asm("cli"); // Disable interrupts.

    printf("ERROR(");
    printf(message);
    printf(") at ");
    printf(file);
    printf(":");
    printf("%d", line);
    printf("\n");
    // Halt by going into an infinite loop.
    while(1);
}
