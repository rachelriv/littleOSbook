global gdt_flush
global idt_flush
global tss_flush

section .text
gdt_flush:          ; from: http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
  mov eax, [esp+4]  ; load eax with address of gdt_ptr_t
  lgdt [eax]
  mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
  mov ds, ax        ; Load all data segment selectors
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  jmp 0x08:.flush   ; 0x08 is the offset to our code segment: Far jump!
.flush:
  ret

idt_flush:          ; from: http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
  mov eax, [esp+4]
  lidt [eax]
  ret

tss_flush:         ; From tutorial http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html
  mov ax, 0x2B     ; Load the index of the TSS. Since the index is 0x28 but the last two bits
                   ; must be set, we have 0x28 | 0x3 = 0x2B.
  ltr ax           ; Load 0x2B into the task state register
  ret 
