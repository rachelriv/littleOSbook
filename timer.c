static void timer_callback(registers_ t regs) {
    tick++;
    switch_task();
}
