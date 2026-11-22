;IsExpired : ECX = index. Returns EAX = 1 if expired else 0
IsExpired PROC
    mov eax, medExpiry[ecx*4]
    cmp eax, TODAY
    jb expYes
    mov eax, 0
    ret

expYes:
    mov eax, 1
    ret
IsExpired ENDP

;IsExpiringSoon : ECX = index. Returns EAX = 1 if not expired but expires
;on or before SOON_LIMIT
IsExpiringSoon PROC
    mov eax, medExpiry[ecx*4]
    cmp eax, TODAY
    jb notSoon
    cmp eax, SOON_LIMIT
    ja notSoon
    mov eax, 1
    ret

notSoon:
    mov eax, 0
    ret
IsExpiringSoon ENDP

;Matches : ECX = index, EBX = mode. Returns EAX = 1 if the medicine matches
;mode 1 = low stock (qty <= LOW_STOCK)   mode 2 = expired
;mode 3 = expiring soon                  mode 4 = zero stock
Matches PROC
    cmp ebx, 1
    je mLow
    cmp ebx, 2
    je mExp
    cmp ebx, 3
    je mSoon
    mov eax, 1                  ; mode 4: zero stock
    cmp medQty[ecx*4], 0
    je mDone
    mov eax, 0
    jmp mDone

mLow:
    mov eax, 1
    cmp medQty[ecx*4], LOW_STOCK
    jbe mDone
    mov eax, 0
    jmp mDone

mExp:
    call IsExpired
    jmp mDone

mSoon: call IsExpiringSoon
mDone: ret
Matches ENDP

;ListFiltered : EBX = mode (see Matches). Prints matching rows.
;Returns EAX = number of matches
ListFiltered PROC USES ebx ecx esi
    call PrintHeader
    xor esi, esi
    xor ecx, ecx

lfLoop:
    cmp ecx, medCount
    jae lfEnd
    call Matches
    cmp eax, 1
    jne lfNext
    call PrintRow
    inc esi

lfNext:
    inc ecx
    jmp lfLoop

lfEnd:
    cmp esi, 0
    jne lfRet
    push edx
    mov edx, OFFSET msgNone
    call WriteString
    pop edx

lfRet:
    mov eax, esi
    ret
ListFiltered ENDP

;CountMatches : EBX = mode. Returns EAX = number of matching medicines
CountMatches PROC USES ecx esi
    xor esi, esi
    xor ecx, ecx

cnLoop:
    cmp ecx, medCount
    jae cnDone
    call Matches
    add esi, eax
    inc ecx
    jmp cnLoop

cnDone:
    mov eax, esi
    ret
CountMatches ENDP