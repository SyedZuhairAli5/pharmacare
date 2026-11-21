;ShowMenu : clears screen and prints the menu
ShowMenu PROC USES eax edx
    call Clrscr
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET sBanner
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    mov edx, OFFSET sMenu
    call WriteString
    ret
ShowMenu ENDP

;GetInt : reads a validated integer. Returns EAX. Repeats until valid.
GetInt PROC USES edx
again:
    call ReadInt
    jno gotIt                   ;ReadInt sets OverflowFlag on invalid input
    mov edx, OFFSET msgInvalid
    call PrintWarn
    jmp again

gotIt:
    ret
GetInt ENDP

;PrintWarn : EDX = string, prints in red
PrintWarn PROC USES eax
    mov eax, C_RED
    call SetTextColor
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    ret
PrintWarn ENDP

;PrintOK : EDX = string, prints in green
PrintOK PROC USES eax
    mov eax, C_GRN
    call SetTextColor
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    ret
PrintOK ENDP

;PrintLV : EDX = label string, EAX = value. Prints "label value" + newline
PrintLV PROC USES eax edx
    call WriteString
    call WriteDec
    call Crlf
    ret
PrintLV ENDP

;Print2 : EAX (0..99) printed with two digits
Print2 PROC USES eax
    cmp eax, 10
    jae two
    push eax
    mov al, '0'
    call WriteChar
    pop eax

two:
    call WriteDec
    ret
Print2 ENDP

;PrintDate : EAX = YYYYMMDD, prints DD/MM/YYYY
PrintDate PROC USES eax ebx ecx edx
    mov ebx, 100
    xor edx, edx
    div ebx                     ; eax = YYYYMM, edx = DD
    mov ecx, edx
    xor edx, edx
    div ebx                     ; eax = YYYY, edx = MM
    push eax
    push edx
    mov eax, ecx
    call Print2
    mov al, '/'
    call WriteChar
    pop eax
    call Print2
    mov al, '/'
    call WriteChar
    pop eax
    call WriteDec
    ret
PrintDate ENDP

;PrintName : EDX = string. Prints it padded with spaces to NAME_W columns
PrintName PROC USES eax ebx ecx edx
    push edx
    call WriteString
    pop edx
    xor ecx, ecx

lenLoop:
    cmp BYTE PTR [edx+ecx], 0
    je lenDone
    inc ecx
    jmp lenLoop

lenDone:
    mov ebx, NAME_W
    sub ebx, ecx
    jle nameDone
    mov ecx, ebx

padLoop:
    mov al, ' '
    call WriteChar
    loop padLoop

nameDone:
    ret
PrintName ENDP

;PrintDecPad : EAX = value, EBX = column width. Prints value then pads
PrintDecPad PROC USES eax ebx ecx edx esi
    push eax
    push ebx
    mov ecx, 1                  ; digit count
    mov esi, 10

digitLoop:
    cmp eax, 10
    jb digitsDone
    xor edx, edx
    div esi
    inc ecx
    jmp digitLoop

digitsDone:
    pop ebx
    pop eax
    call WriteDec
    sub ebx, ecx
    jle decDone
    mov ecx, ebx

spLoop:
    mov al, ' '
    call WriteChar
    loop spLoop

decDone:
    ret
PrintDecPad ENDP

; FindIndex : EAX = medicine ID. Returns EBX = array index, or -1 if not found
FindIndex PROC USES ecx
    xor ecx, ecx

findLoop:
    cmp ecx, medCount
    jae findFail
    cmp eax, medID[ecx*4]
    je findHit
    inc ecx
    jmp findLoop

findHit:
    mov ebx, ecx
    ret

findFail:
    mov ebx, -1
    ret
FindIndex ENDP