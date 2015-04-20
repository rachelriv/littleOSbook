global copy_page_physical

copy_page_physical:
    push ebx               ; We protect the contents of EBX
    pushf                  ; We push EFLAGS, which allows us reenable
                           ; interrupts.
    cli                    ; Disable interrupts
    
    mov ebx, [esp + 12]    ; Source address
    mov ecx, [esp + 16]    ; Destination address
    mov edx, cr0           ; Use the control register
    and edx, 0x7fffffff    ; to disable paging
    mov cr0, edx           

    mov edx, 1024          ; We will copy 1024*4 = 4096 bytes

.loop
    mov eax, [ebx]         ; Get the word at the source address
    mov [ecx], eax         ; Store it at the destination address
    add ebx, 4             ; Increase source address
    add ecx, 4             ; Increase destination address
    dec edx                ; Decrease counter
    jnz                    ; loop until edx is 0

    mov edx, cr0           ; Reenable paging with the control 
    or  edx, 0x80000000    ; register.
    mov cr0, edx           

    popf                   ; Pop flags
    pop ebx                ; Get the original value of ebx back
    ret
