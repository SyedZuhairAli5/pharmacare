;FindTopSeller : returns EAX = units sold of best seller, EBX = its index
;(EBX = -1 if nothing has been sold)
FindTopSeller PROC USES ecx edx
    mov ebx, -1
    xor edx, edx
    xor ecx, ecx

tsLoop:
    cmp ecx, medCount
    jae tsDone
    mov eax, medSold[ecx*4]
    cmp eax, edx
    jbe tsNext
    mov edx, eax
    mov ebx, ecx

tsNext:
    inc ecx
    jmp tsLoop

tsDone:
    mov eax, edx
    ret
FindTopSeller ENDP

;SalesSummary : session sales statistics
SalesSummary PROC USES eax ebx edx
    call Clrscr
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET lblSalesHdr
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    cmp billCount, 0
    jne ssHave
    mov edx, OFFSET lblNoSales
    call WriteString
    ret

ssHave:
    mov eax, billCount
    mov edx, OFFSET lblBills
    call PrintLV
    mov eax, totalUnitsSold
    mov edx, OFFSET lblUnits
    call PrintLV
    mov eax, totalSales
    mov edx, OFFSET lblSales
    call PrintLV
    mov eax, totalDiscount
    mov edx, OFFSET lblDiscGiven
    call PrintLV
    mov eax, totalSales         ;average bill = totalSales / billCount
    xor edx, edx
    div billCount
    mov edx, OFFSET lblAvg
    call PrintLV
    call FindTopSeller
    mov edx, OFFSET lblTop
    call WriteString
    mov edx, medNamePtr[ebx*4]
    call WriteString
    mov edx, OFFSET lblTopUnits
    call WriteString
    call WriteDec               ;EAX = units sold of top seller
    mov al, ')'
    call WriteChar
    call Crlf
    ret
SalesSummary ENDP

;InventoryReport : valuation, counts and low-stock list
InventoryReport PROC USES eax ebx edx
    call Clrscr
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET lblRepHdr
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    mov eax, medCount
    mov edx, OFFSET lblMedCount
    call PrintLV
    call CalcInventoryValue
    mov edx, OFFSET lblInvVal
    call PrintLV
    mov ebx, 1
    call CountMatches
    mov edx, OFFSET lblLowCnt
    call PrintLV
    mov ebx, 4
    call CountMatches
    mov edx, OFFSET lblZeroCnt
    call PrintLV
    mov ebx, 2
    call CountMatches
    mov edx, OFFSET lblExpCnt
    call PrintLV
    mov ebx, 3
    call CountMatches
    mov edx, OFFSET lblSoonCnt
    call PrintLV
    call Crlf
    mov edx, OFFSET lblLowList
    call WriteString
    mov ebx, 1
    call ListFiltered
    ret
InventoryReport ENDP