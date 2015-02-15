global load_gdt

load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]
    ret
    
    jmp 0x08:.reload_segments

.reload_segments
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret
