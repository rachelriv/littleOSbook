OBJECTS =  error.o keyboard.o multiboot.asm.o interrupt.asm.o serial.o framebuffer.o kmain.o loader.asm.o \
	   io.asm.o string.o descriptor_tables.o ldt.asm.o isr.o ordered_array.o kheap.o paging.o \
           syscall.o task.o\
                                        
CC = gcc
CFLAGS = -m32 -fno-stack-protector \
					-ffreestanding \
					-Wall -Wextra -g -c # -Werror
LDFLAGS = -T link.ld -melf_i386
AS = nasm
ASFLAGS = -f elf

all: kernel.elf os.iso

run: os.iso
	bochs -f bochsrc.txt -q

os.iso: kernel.elf
	cp kernel.elf iso/boot/
	genisoimage -R \
              -b boot/grub/stage2_eltorito    \
              -no-emul-boot                   \
              -boot-load-size 4               \
              -A os                           \
              -input-charset utf8             \
              -quiet                          \
              -boot-info-table                \
              -o os.iso                       \
              iso

kernel.elf: $(OBJECTS)
	ld $(LDFLAGS) $(OBJECTS) -o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.asm.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f kernel.elf iso/boot/kernel.elf *.o os.iso

