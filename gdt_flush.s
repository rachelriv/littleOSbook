; code inspiration from: http://www.osdever.net/bkerndev/Docs/gdt.htm

global gdt_flush

section .text
gdt_flush:
  mov eax, [esp+4]  ; load eax with address of gdt_ptr_t
  lgdt [eax]
  ; since each GDT entry is 8 bytes and the kernel data descriptor is 
  ; the third segment, the offset is 16 (0x10)
  mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
  mov ds, ax        ; Load all data segment selectors
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  ; since each GDT entry is 8 bytes and the kernel code descriptor is
  ; the second segment, the offset is 8 (0x8)
  jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
  ret
