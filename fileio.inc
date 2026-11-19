;AppendStr : EDX = zero-terminated string. Copies it (without the zero) to [EDI]
AppendStr PROC USES eax edx
asLoop:
    mov al, [edx]
    cmp al, 0
    je asDone
    mov [edi], al
    inc edi
    inc edx
    jmp asLoop

asDone:
    ret
AppendStr ENDP

;AppendChar : AL = character. Writes it to [EDI]
AppendChar PROC
    mov [edi], al
    inc edi
    ret
AppendChar ENDP

;AppendUInt : EAX = unsigned value. Writes its decimal digits to [EDI]
AppendUInt PROC USES eax ebx ecx edx
    mov ebx, 10
    xor ecx, ecx                ;digit count

auDiv:
    xor edx, edx
    div ebx                     ;EDX = next digit (last digit first)
    push edx
    inc ecx
    cmp eax, 0
    jne auDiv

auOut:
    pop eax                     ;digits come back in the right order
    add al, '0'
    mov [edi], al
    inc edi
    loop auOut
    ret
AppendUInt ENDP

;ParseUInt : ESI = text pointer. Skips spaces, reads decimal digits, then
;skips one comma. Returns EAX = value, ESI advanced, CF = 1 if no digits found.
ParseUInt PROC USES ebx ecx edx
    xor eax, eax
    xor ecx, ecx                ;digit count

puSkip:
    cmp BYTE PTR [esi], ' '
    jne puDigits
    inc esi
    jmp puSkip

puDigits:
    movzx ebx, BYTE PTR [esi]
    sub ebx, '0'
    cmp ebx, 9
    ja puEnd                    ; not a digit (unsigned compare also catches < '0')
    imul eax, eax, 10
    add eax, ebx
    inc esi
    inc ecx
    jmp puDigits

puEnd:
    cmp BYTE PTR [esi], ','
    jne puNoComma
    inc esi

puNoComma:
    cmp ecx, 1                  ; CF = 1 when ECX = 0 (no digits)
    ret
ParseUInt ENDP

;LoadInventory : reads inventory.txt into the parallel arrays.
;- file missing -> keeps the built-in data and creates the file
;- no valid lines -> keeps the built-in data
;- bad lines are skipped (wrong fields, duplicate ID, price/date out of range)
; Names are stored in newNameBuf, one NAME_REC slot per record.
LoadInventory PROC USES eax ebx ecx edx esi edi
    mov edx, OFFSET invFileName
    call OpenInputFile
    cmp eax, -1
    jne liOpened
    mov edx, OFFSET msgNoInvFile    ;first run: write the built-in data out
    call PrintWarn
    call SaveInventory
    call WaitMsg
    ret

liOpened:
    mov fileHandle, eax
    mov edx, OFFSET fileBuf
    mov ecx, FILE_BUF_SIZE
    call ReadFromFile               ;EAX = bytes read
    mov edi, eax
    pushfd                          ;remember CF (read error) across CloseFile
    mov eax, fileHandle
    call CloseFile
    popfd
    jc liBad
    mov BYTE PTR fileBuf[edi], 0    ;terminate the text

    mov esi, OFFSET fileBuf
    xor ebx, ebx                    ;EBX = records loaded so far

liSkipWS:
    mov al, [esi]
    cmp al, 0
    je liEnd
    cmp al, ' '
    ja liRec                        ;a visible character starts a record
    inc esi                         ;space / CR / LF / tab: skip
    jmp liSkipWS

liRec:
    cmp ebx, MAX_MED
    jae liEnd
    call ParseUInt                  ;ID
    jc liSkipLine
    mov tmpID, eax
    call ParseUInt                  ;price
    jc liSkipLine
    mov tmpPrice, eax
    call ParseUInt                  ;quantity
    jc liSkipLine
    mov tmpQty, eax
    call ParseUInt                  ;expiry YYYYMMDD
    jc liSkipLine
    mov tmpExpiry, eax

    cmp tmpID, 0                     ;validate
    je liSkipLine
    cmp tmpPrice, 0
    je liSkipLine
    cmp tmpPrice, MAX_PRICE
    ja liSkipLine
    cmp tmpExpiry, 19000101
    jb liSkipLine
    cmp tmpExpiry, 99991231
    ja liSkipLine
    xor ecx, ecx                    ;duplicate ID among records loaded so far?

liDup:
    cmp ecx, ebx
    jae liNoDup
    mov eax, tmpID
    cmp eax, medID[ecx*4]
    je liSkipLine
    inc ecx
    jmp liDup

liNoDup:
    mov edi, ebx                    ;name slot = newNameBuf + index * NAME_REC
    imul edi, edi, NAME_REC
    add edi, OFFSET newNameBuf

liNameLead:
    cmp BYTE PTR [esi], ' '         ;skip spaces before the name
    jne liName0
    inc esi
    jmp liNameLead

liName0: xor ecx, ecx               ;characters copied

liName:
    mov al, [esi]
    cmp al, 0Dh
    je liNameEnd
    cmp al, 0Ah
    je liNameEnd
    cmp al, 0
    je liNameEnd
    cmp ecx, NAME_LEN_MAX
    jae liNameSkip                  ;name too long: drop the extra characters
    mov [edi+ecx], al
    inc ecx

liNameSkip:
    inc esi
    jmp liName

liNameEnd:
    mov BYTE PTR [edi+ecx], 0
    cmp ecx, 0                      ;empty name = bad line
    je liSkipLine

    mov eax, tmpID                  ;store the record
    mov medID[ebx*4], eax
    mov eax, tmpPrice
    mov medPrice[ebx*4], eax
    mov eax, tmpQty
    mov medQty[ebx*4], eax
    mov eax, tmpExpiry
    mov medExpiry[ebx*4], eax
    mov medNamePtr[ebx*4], edi
    mov medSold[ebx*4], 0
    inc ebx
    jmp liSkipWS

liSkipLine:                         ;throw away the rest of this line
    mov al, [esi]
    cmp al, 0
    je liEnd
    inc esi
    cmp al, 0Ah
    jne liSkipLine
    jmp liSkipWS

liEnd:
    cmp ebx, 0
    je liBad                        ;nothing valid: keep the built-in data
    mov medCount, ebx
    mov newNameCount, ebx           ;next free name slot
    ret

liBad:
    mov edx, OFFSET msgBadInvFile
    call PrintWarn
    call WaitMsg
    ret
LoadInventory ENDP

;SaveInventory : writes all medicines to inventory.txt (replacing the file)
SaveInventory PROC USES eax ebx ecx edx esi edi
    mov edi, OFFSET fileBuf         ;build the whole text in memory first
    xor esi, esi

siLoop:
    cmp esi, medCount
    jae siWrite
    mov eax, medID[esi*4]
    call AppendUInt
    mov al, ','
    call AppendChar
    mov eax, medPrice[esi*4]
    call AppendUInt
    mov al, ','
    call AppendChar
    mov eax, medQty[esi*4]
    call AppendUInt
    mov al, ','
    call AppendChar
    mov eax, medExpiry[esi*4]
    call AppendUInt
    mov al, ','
    call AppendChar
    mov edx, medNamePtr[esi*4]
    call AppendStr
    mov al, 0Dh
    call AppendChar
    mov al, 0Ah
    call AppendChar
    inc esi
    jmp siLoop

siWrite:
    mov eax, edi
    sub eax, OFFSET fileBuf
    mov fileBytes, eax              ;number of bytes to write
    mov edx, OFFSET invFileName
    call CreateOutputFile           ;creates the file, or empties it if it exists
    cmp eax, -1
    je siFail
    mov fileHandle, eax
    mov edx, OFFSET fileBuf
    mov ecx, fileBytes
    call WriteToFile                ;EAX = handle already
    mov eax, fileHandle
    call CloseFile
    mov edx, OFFSET msgSaved
    call PrintOK
    ret

siFail:
    mov edx, OFFSET msgSaveFail
    call PrintWarn
    ret
SaveInventory ENDP

;LogSale : appends the current bill (cart + totals) to sales_log.txt.
;Call it BEFORE CommitSale (it needs the cart and uses billCount + 1).
LogSale PROC USES eax ebx ecx edx esi edi
    mov edi, OFFSET logBuf
    mov edx, OFFSET lgBill
    call AppendStr
    mov eax, billCount
    inc eax
    call AppendUInt
    mov edx, OFFSET lgBy
    call AppendStr
    mov edx, OFFSET billerName
    call AppendStr
    mov edx, OFFSET lgSub
    call AppendStr
    mov eax, bSub
    call AppendUInt
    mov edx, OFFSET lgDisc
    call AppendStr
    mov eax, bDisc
    call AppendUInt
    mov edx, OFFSET lgTotal
    call AppendStr
    mov eax, bTotal
    call AppendUInt
    mov edx, OFFSET lgPaid
    call AppendStr
    mov eax, bPaid
    call AppendUInt
    mov edx, OFFSET lgChange
    call AppendStr
    mov eax, bChange
    call AppendUInt
    mov edx, OFFSET lgNL
    call AppendStr

    xor esi, esi                    ;one indented line per cart item

lgLoop:
    cmp esi, cartCount
    jae lgWrite
    mov edx, OFFSET lgIndent
    call AppendStr
    mov ebx, cartIdx[esi*4]         ;EBX = medicine index
    mov edx, medNamePtr[ebx*4]
    call AppendStr
    mov edx, OFFSET lgX
    call AppendStr
    mov eax, cartQty[esi*4]
    call AppendUInt
    mov edx, OFFSET lgAt
    call AppendStr
    mov eax, medPrice[ebx*4]
    call AppendUInt
    mov edx, OFFSET lgEq
    call AppendStr
    mov eax, medPrice[ebx*4]
    mul cartQty[esi*4]
    call AppendUInt
    mov edx, OFFSET lgNL
    call AppendStr
    inc esi
    jmp lgLoop

lgWrite:
    mov edx, OFFSET lgNL            ;blank line between bills
    call AppendStr
    mov eax, edi
    sub eax, OFFSET logBuf
    mov fileBytes, eax

    ;Open for APPEND: FILE_APPEND_DATA + OPEN_ALWAYS = create if missing,
    ;never erase, every write goes to the end of the file.
    INVOKE CreateFileA, OFFSET logFileName, FIO_APPEND_DATA, FIO_SHARE_READ, 0, FIO_OPEN_ALWAYS, FIO_ATTR_NORMAL, 0
    cmp eax, -1
    je lgFail
    mov fileHandle, eax
    mov edx, OFFSET logBuf
    mov ecx, fileBytes
    call WriteToFile
    mov eax, fileHandle
    call CloseFile
    ret

lgFail:
    mov edx, OFFSET msgLogFail
    call PrintWarn
    ret
LogSale ENDP

;ViewSalesLog : reads sales_log.txt and prints it (in chunks, any file size)
ViewSalesLog PROC USES eax ecx edx
    mov edx, OFFSET logFileName
    call OpenInputFile
    cmp eax, -1
    jne vlOpen
    mov edx, OFFSET msgNoLog
    call PrintWarn
    ret

vlOpen:
    mov fileHandle, eax
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET lblLogHdr
    call WriteString
    mov eax, C_DEF
    call SetTextColor

vlRead:
    mov eax, fileHandle
    mov edx, OFFSET fileBuf
    mov ecx, FILE_BUF_SIZE
    call ReadFromFile               ; EAX = bytes read
    jc vlDone
    cmp eax, 0
    je vlDone                       ; end of file
    mov BYTE PTR fileBuf[eax], 0
    mov edx, OFFSET fileBuf
    call WriteString
    jmp vlRead

vlDone:
    mov eax, fileHandle
    call CloseFile
    ret
ViewSalesLog ENDP