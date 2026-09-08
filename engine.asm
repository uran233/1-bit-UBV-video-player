section .text
global _process_frame_asm

_process_frame_asm:
    push ebp
    mov ebp, esp
    push edi
    push esi
    push ebx

    mov esi, [ebp+8]    ; dane wejsciowe (czysty 1-bit RAW)
    mov edi, [ebp+12]   ; tekstura wyjsciowa (XRGB)
    mov ecx, [ebp+16]   ; width
    imul ecx, [ebp+20]  ; total pixels
    shr ecx, 3          ; przetwarzamy bajtami (8 pikseli na raz)

.byte_loop:
    push ecx            ; zachowaj licznik bajtow
    lodsb               ; czytaj 8 bitow do AL
    mov bl, al
    mov ecx, 8          ; petla dla 8 bitow w bajcie

.bit_loop:
    test bl, 0x80       ; sprawdz najwyzszy bit
    jnz .white
    mov eax, 0x00000000 ; czarny
    jmp .write
.white:
    mov eax, 0x00FFFFFF ; bialy
.write:
    stosd               ; zapisz do tekstury
    shl bl, 1           ; nastepny bit
    loop .bit_loop

    pop ecx             ; przywroc licznik bajtow
    loop .byte_loop

    pop ebx
    pop esi
    pop edi
    mov esp, ebp
    pop ebp
    ret