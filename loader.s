global loader                  

FL_ALIGN equ 1 << 0
MEMINFO equ 1 << 1
FLAGS equ FL_ALIGN | MEMINFO
MAGIC equ 0x1BADB002    
CHECKSUM equ -(MAGIC + FLAGS)

section .text:
align 4
  dd MAGIC
  dd FLAGS
  dd CHECKSUM

loader:     
  mov eax, 0xCAFEBABE
.loop:
  jmp .loop
