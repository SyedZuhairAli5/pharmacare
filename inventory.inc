;PrintHeader prints the table header
PrintHeader PROC USES eax edx
    mov eax, C_WHT
    call SetTextColor
    mov edx, OFFSET sHeader
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    ret
PrintHeader ENDP

;PrintStatus : ECX = index. Prints coloured status text
;priority: expired > expiring soon > out of stock > low stock > OK
PrintStatus PROC USES eax edx
    call IsExpired
    cmp eax, 1
    jne stNotExp
    mov eax, C_RED
    call SetTextColor
    mov edx, OFFSET sExpired
    call WriteString
    jmp stDone

stNotExp:
    call IsExpiringSoon
    cmp eax, 1
    jne stNotSoon
    mov eax, C_YEL
    call SetTextColor
    mov edx, OFFSET sSoon
    call WriteString
    jmp stDone

stNotSoon:
    mov eax, medQty[ecx*4]
    cmp eax, 0
    jne stNotZero
    mov eax, C_RED
    call SetTextColor
    mov edx, OFFSET sOut
    call WriteString
    jmp stDone

stNotZero:
    cmp eax, LOW_STOCK
    ja stGood
    mov eax, C_YEL
    call SetTextColor
    mov edx, OFFSET sLow
    call WriteString
    jmp stDone

stGood:
    mov eax, C_GRN
    call SetTextColor
    mov edx, OFFSET sOK
    call WriteString

stDone:
    mov eax, C_DEF
    call SetTextColor
    ret
PrintStatus ENDP

;PrintRow : ECX = index. Prints one medicine row
PrintRow PROC USES eax ebx edx
    mov eax, medID[ecx*4]
    mov ebx, 6
    call PrintDecPad
    mov edx, medNamePtr[ecx*4]
    call PrintName
    mov eax, medPrice[ecx*4]
    mov ebx, 8
    call PrintDecPad
    mov eax, medQty[ecx*4]
    mov ebx, 6
    call PrintDecPad
    mov eax, medExpiry[ecx*4]
    call PrintDate
    mov al, ' '
    call WriteChar
    mov al, ' '
    call WriteChar
    call PrintStatus
    call Crlf
    ret
PrintRow ENDP

;DisplayAll : prints every medicine record
DisplayAll PROC USES ecx
    call PrintHeader
    xor ecx, ecx

dispLoop:
    cmp ecx, medCount
    jae dispDone
    call PrintRow
    inc ecx
    jmp dispLoop

dispDone:
    ret
DisplayAll ENDP

;SearchMedicine : search by ID and show the record
SearchMedicine PROC USES eax ebx ecx edx
    mov edx, OFFSET msgAskID
    call WriteString
    call GetInt
    call FindIndex
    cmp ebx, -1
    jne srHit
    mov edx, OFFSET msgNotFound
    call PrintWarn
    ret

srHit:
    mov ecx, ebx
    call PrintHeader
    call PrintRow
    ret
SearchMedicine ENDP

;GetExpiryDate : asks day, month, year and validates them.
;Returns EAX = YYYYMMDD (a real date, not before TODAY). Repeats until valid.
GetExpiryDate PROC USES ebx ecx edx
    mov edx, OFFSET msgNewBatch
    call WriteString

gdAgain:
    mov edx, OFFSET msgAskDay
    call WriteString
    call GetInt
    cmp eax, 1
    jl gdBad
    cmp eax, 31
    jg gdBad
    mov ebx, eax                ; EBX = day
    mov edx, OFFSET msgAskMonth
    call WriteString
    call GetInt
    cmp eax, 1
    jl gdBad
    cmp eax, 12
    jg gdBad
    mov ecx, eax                ; ECX = month
    cmp ecx, 2                  ; February: max 29 days
    jne gdNotFeb
    cmp ebx, 29
    jg gdBad

gdNotFeb:
    cmp ecx, 4                  ; April, June, September, November: max 30
    je gdThirty
    cmp ecx, 6
    je gdThirty
    cmp ecx, 9
    je gdThirty
    cmp ecx, 11
    je gdThirty
    jmp gdYear
gdThirty:
    cmp ebx, 30
    jg gdBad

gdYear:
    mov edx, OFFSET msgAskYear
    call WriteString
    call GetInt
    cmp eax, 2026
    jl gdBad
    cmp eax, 2100
    jg gdBad
    imul eax, eax, 10000        ; year * 10000
    imul ecx, ecx, 100          ; month * 100
    add eax, ecx
    add eax, ebx                ; + day  -> YYYYMMDD
    cmp eax, TODAY
    jb gdBad
    ret

gdBad:
    mov edx, OFFSET msgBadDate
    call PrintWarn
    jmp gdAgain
GetExpiryDate ENDP

;RestockMedicine : add stock to an existing medicine and set the expiry date
;of the new batch (each medicine keeps one expiry date, so it is replaced)
RestockMedicine PROC USES eax ebx ecx edx esi
    mov edx, OFFSET msgAskID
    call WriteString
    call GetInt
    call FindIndex
    cmp ebx, -1
    jne rsHit
    mov edx, OFFSET msgNotFound
    call PrintWarn
    ret

rsHit:
    mov ecx, ebx
    call PrintHeader
    call PrintRow
    mov edx, OFFSET msgAskRest
    call WriteString
    call GetInt
    cmp eax, 0
    jle rsBad
    cmp eax, MAX_RESTOCK
    ja rsBad
    mov esi, eax                ; quantity to add
    call GetExpiryDate          ; EAX = new expiry (YYYYMMDD)
    mov medExpiry[ecx*4], eax
    add medQty[ecx*4], esi
    mov edx, OFFSET msgRestocked
    call PrintOK
    call PrintRow
    ret

rsBad:
    mov edx, OFFSET msgBadRest
    call PrintWarn
    ret
RestockMedicine ENDP

;CalcInventoryValue : returns EAX = sum(price * qty)
CalcInventoryValue PROC USES ebx ecx edx
    xor ebx, ebx
    xor ecx, ecx

ivLoop:
    cmp ecx, medCount
    jae ivDone
    mov eax, medPrice[ecx*4]
    mul medQty[ecx*4]
    add ebx, eax
    inc ecx
    jmp ivLoop

ivDone:
    mov eax, ebx
    ret
CalcInventoryValue ENDP