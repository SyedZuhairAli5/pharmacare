;AddMedicine : asks for ID, name, price, starting stock and expiry date.
;Nothing is stored until every value has been validated.
AddMedicine PROC USES eax ebx ecx edx esi edi
    cmp medCount, MAX_MED
    jb amSpace
    mov edx, OFFSET msgMedFull
    call PrintWarn
    ret

amSpace:
    cmp newNameCount, MAX_MED
    jb amPool
    mov edx, OFFSET msgPoolFull
    call PrintWarn
    ret

amPool:
    mov edx, OFFSET msgAskNewID
    call WriteString
    call GetInt
    cmp eax, 0
    jg amIdOK
    mov edx, OFFSET msgBadID
    call PrintWarn
    ret

amIdOK:
    mov edi, eax                ;EDI = new ID
    call FindIndex              ;EAX = ID in, EBX = index or -1
    cmp ebx, -1
    je amUnique
    mov edx, OFFSET msgDupID
    call PrintWarn
    ret

amUnique:
    mov edx, OFFSET msgAskName
    call WriteString
    mov esi, newNameCount       ;address of the next free name slot:
    imul esi, esi, NAME_REC     ;newNameBuf + slot * NAME_REC
    add esi, OFFSET newNameBuf
    push edi
    push esi
    mov edx, esi
    mov ecx, NAME_LEN_MAX
    call ReadString             ;EAX = characters typed
    pop esi
    pop edi
    cmp eax, 0
    jne amNameOK
    mov edx, OFFSET msgBadName
    call PrintWarn
    ret

amNameOK:
    mov edx, OFFSET msgAskPrice
    call WriteString
    call GetInt
    cmp eax, 1
    jl amBadPrice
    cmp eax, MAX_PRICE
    jg amBadPrice
    mov ebx, eax                ;EBX = price
    mov edx, OFFSET msgAskInitQty
    call WriteString
    call GetInt
    cmp eax, 0
    jl amBadQty
    cmp eax, MAX_INIT_QTY
    jg amBadQty
    mov ecx, eax                ;ECX = starting stock
    call GetExpiryDate          ;EAX = expiry (YYYYMMDD), keeps EBX/ECX/EDX
    mov edx, medCount           ;EDX = index of the new record
    mov medExpiry[edx*4], eax
    mov medPrice[edx*4], ebx
    mov medQty[edx*4], ecx
    mov medID[edx*4], edi
    mov medNamePtr[edx*4], esi
    mov medSold[edx*4], 0
    inc medCount
    inc newNameCount
    mov ecx, edx                ; show the new record
    mov edx, OFFSET msgMedAdded
    call PrintOK
    call PrintHeader
    call PrintRow
    ret

amBadPrice:
    mov edx, OFFSET msgBadPrice
    call PrintWarn
    ret

amBadQty:
    mov edx, OFFSET msgBadInitQty
    call PrintWarn
    ret
AddMedicine ENDP

;RemoveMedicine : asks for an ID, shows the record, asks for confirmation,
;then removes it by shifting all later records down by one place.
RemoveMedicine PROC USES eax ebx ecx edx
    mov edx, OFFSET msgAskID
    call WriteString
    call GetInt
    call FindIndex
    cmp ebx, -1
    jne rmHit
    mov edx, OFFSET msgNotFound
    call PrintWarn
    ret

rmHit:
    mov ecx, ebx
    call PrintHeader
    call PrintRow
    mov edx, OFFSET msgAskConfirm
    call WriteString
    call GetInt
    cmp eax, 1
    je rmDo
    mov edx, OFFSET msgRemoveCancel
    call PrintWarn
    ret

rmDo: mov ecx, ebx              ;start at the removed index

rmShift:
    mov edx, medCount
    dec edx                     ;last valid index
    cmp ecx, edx
    jae rmDone
    mov eax, medID[ecx*4+4]     ;record i+1 -> record i, for all six arrays
    mov medID[ecx*4], eax
    mov eax, medPrice[ecx*4+4]
    mov medPrice[ecx*4], eax
    mov eax, medQty[ecx*4+4]
    mov medQty[ecx*4], eax
    mov eax, medExpiry[ecx*4+4]
    mov medExpiry[ecx*4], eax
    mov eax, medSold[ecx*4+4]
    mov medSold[ecx*4], eax
    mov eax, medNamePtr[ecx*4+4]
    mov medNamePtr[ecx*4], eax
    inc ecx
    jmp rmShift

rmDone:
    dec medCount
    mov edx, OFFSET msgRemoved
    call PrintOK
    ret
RemoveMedicine ENDP