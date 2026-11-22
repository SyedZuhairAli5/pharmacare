;CartFind : ECX = medicine index. Returns EBX = position in cart, or -1
CartFind PROC USES edx
    xor edx, edx

cfLoop:
    cmp edx, cartCount
    jae cfFail
    cmp ecx, cartIdx[edx*4]
    je cfHit
    inc edx
    jmp cfLoop

cfHit:
    mov ebx, edx
    ret

cfFail:
    mov ebx, -1
    ret
CartFind ENDP

;AddToCart : ECX = medicine index. Validates expiry/stock/quantity, adds line
AddToCart PROC USES eax ebx edx esi
    call IsExpired
    cmp eax, 1
    jne acNotExp
    mov edx, OFFSET msgExpBlock
    call PrintWarn
    ret

acNotExp:
    cmp medQty[ecx*4], 0
    jne acHasStock
    mov edx, OFFSET msgNoStock
    call PrintWarn
    ret

acHasStock:
    mov edx, OFFSET msgAskQty
    call WriteString
    call GetInt
    cmp eax, 0
    jg acQtyOK
    mov edx, OFFSET msgBadQty
    call PrintWarn
    ret

acQtyOK:
    mov esi, eax                ;requested quantity
    call CartFind               ;EBX = cart position or -1
    cmp ebx, -1
    je acNewLine
    mov eax, esi                ;already in cart: add to existing line
    add eax, cartQty[ebx*4]
    cmp eax, medQty[ecx*4]
    ja acTooMany
    mov cartQty[ebx*4], eax
    mov edx, OFFSET msgAdded
    call PrintOK
    ret

acNewLine:
    cmp esi, medQty[ecx*4]
    ja acTooMany
    mov eax, cartCount
    cmp eax, MAX_CART
    jb acRoom
    mov edx, OFFSET msgCartFull
    call PrintWarn
    ret

acRoom:
    mov cartIdx[eax*4], ecx
    mov cartQty[eax*4], esi
    inc cartCount
    mov edx, OFFSET msgAdded
    call PrintOK
    ret

acTooMany:
    mov edx, OFFSET msgTooMany
    call PrintWarn
    ret
AddToCart ENDP

;CalcSubtotal : returns EAX = sum(price * qty) over the cart
CalcSubtotal PROC USES ebx ecx edx esi
    xor ebx, ebx
    xor ecx, ecx

csLoop:
    cmp ecx, cartCount
    jae csDone
    mov esi, cartIdx[ecx*4]
    mov eax, medPrice[esi*4]
    mul cartQty[ecx*4]
    add ebx, eax
    inc ecx
    jmp csLoop

csDone:
    mov eax, ebx
    ret
CalcSubtotal ENDP

;ApplyDiscount : EAX = subtotal. Returns EAX = discount amount, EBX = percent
ApplyDiscount PROC USES ecx edx
    mov ecx, 0
    cmp eax, TIER2
    jae adTier2
    cmp eax, TIER1
    jae adTier1
    jmp adCalc

adTier2:
    mov ecx, PCT2
    jmp adCalc
adTier1:mov ecx, PCT1

adCalc:
    mov ebx, ecx
    mul ecx                     ;EDX:EAX = subtotal * percent
    mov ecx, 100
    div ecx                     ;EAX = discount (EDX = 0 after mul)
    ret
ApplyDiscount ENDP

;PrintTotalsBlock : prints subtotal, discount, total due from the bill vars
PrintTotalsBlock PROC USES eax edx
    mov eax, bSub
    mov edx, OFFSET lblSub
    call PrintLV
    mov edx, OFFSET lblDiscA
    call WriteString
    mov eax, bPct
    call WriteDec
    mov edx, OFFSET lblDiscB
    call WriteString
    mov eax, bDisc
    call WriteDec
    call Crlf
    mov eax, bTotal
    mov edx, OFFSET lblTotal
    call PrintLV
    ret
PrintTotalsBlock ENDP

;ProcessPayment : returns EAX = 1 if payment accepted, 0 if cancelled
ProcessPayment PROC USES edx
    call Crlf
    call PrintTotalsBlock

payAgain:
    mov edx, OFFSET msgAskPay
    call WriteString
    call GetInt
    cmp eax, 0
    jle payCancel
    cmp eax, bTotal
    jae payOK
    mov edx, OFFSET msgShort
    call PrintWarn
    jmp payAgain

payOK:
    mov bPaid, eax
    sub eax, bTotal
    mov bChange, eax
    mov eax, 1
    ret

payCancel:
    mov eax, 0
    ret
ProcessPayment ENDP

;PrintReceipt : prints the full receipt for the current cart
PrintReceipt PROC USES eax ebx ecx edx esi
    call Clrscr                 ;receipt is shown on a clean screen
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET lblRcptHdr
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    mov edx, OFFSET lblBillNo
    call WriteString
    mov eax, billCount
    inc eax
    call WriteDec
    call Crlf
    mov edx, OFFSET lblBiller
    call WriteString
    mov edx, OFFSET billerName
    call WriteString
    call Crlf
    mov edx, OFFSET lblRcptCols
    call WriteString
    xor ecx, ecx

rcLoop:
    cmp ecx, cartCount
    jae rcDone
    mov esi, cartIdx[ecx*4]
    mov edx, medNamePtr[esi*4]
    call PrintName
    mov eax, cartQty[ecx*4]
    mov ebx, 6
    call PrintDecPad
    mov eax, medPrice[esi*4]
    mov ebx, 10
    call PrintDecPad
    mov eax, medPrice[esi*4]
    mul cartQty[ecx*4]
    call WriteDec
    call Crlf
    inc ecx
    jmp rcLoop

rcDone:
    mov edx, OFFSET lblLine
    call WriteString
    call PrintTotalsBlock
    mov eax, bPaid
    mov edx, OFFSET lblPaid
    call PrintLV
    mov eax, bChange
    mov edx, OFFSET lblChange
    call PrintLV
    mov eax, C_CYN
    call SetTextColor
    mov edx, OFFSET lblRcptEnd
    call WriteString
    mov eax, C_DEF
    call SetTextColor
    ret
PrintReceipt ENDP

;CommitSale : deducts stock, updates sold counts and session totals
CommitSale PROC USES eax ecx esi
    xor ecx, ecx

cmLoop:
    cmp ecx, cartCount
    jae cmDone
    mov esi, cartIdx[ecx*4]
    mov eax, cartQty[ecx*4]
    sub medQty[esi*4], eax
    add medSold[esi*4], eax
    add totalUnitsSold, eax
    inc ecx
    jmp cmLoop

cmDone:
    mov eax, bTotal
    add totalSales, eax
    mov eax, bDisc
    add totalDiscount, eax
    inc billCount
    mov cartCount, 0
    ret
CommitSale ENDP

;GetBillerName : reads the biller's name into billerName (cannot be empty)
GetBillerName PROC USES eax ecx edx
gbAgain:
    mov edx, OFFSET msgAskBiller
    call WriteString
    mov edx, OFFSET billerName
    mov ecx, BILLER_MAX
    call ReadString             ; EAX = number of characters read
    cmp eax, 0
    jne gbDone
    mov edx, OFFSET msgBadName
    call PrintWarn
    jmp gbAgain

gbDone:
    ret
GetBillerName ENDP

;NewSale : full billing workflow
NewSale PROC USES eax ebx ecx edx
    mov cartCount, 0
    call Clrscr
    call GetBillerName
    call Crlf
    call DisplayAll
    call Crlf

saleLoop:
    mov edx, OFFSET msgAskID
    call WriteString
    call GetInt
    cmp eax, 0
    je saleFinish
    call FindIndex
    cmp ebx, -1
    jne saleFound
    mov edx, OFFSET msgNotFound
    call PrintWarn
    jmp saleLoop

saleFound:
    mov ecx, ebx
    call AddToCart
    jmp saleLoop

saleFinish:
    cmp cartCount, 0
    jne saleHaveCart
    mov edx, OFFSET msgEmptyCart
    call PrintWarn
    ret

saleHaveCart:
    call CalcSubtotal
    mov bSub, eax
    call ApplyDiscount          ;EAX = discount, EBX = percent
    mov bDisc, eax
    mov bPct, ebx
    mov eax, bSub
    sub eax, bDisc
    mov bTotal, eax
    call ProcessPayment
    cmp eax, 1
    je salePaid
    mov edx, OFFSET msgCancel
    call PrintWarn
    mov cartCount, 0
    ret

salePaid:
    call PrintReceipt
    call LogSale                ;append this bill to sales_log.txt
    call CommitSale
    call Crlf
    mov edx, OFFSET msgSaleDone
    call PrintOK
    ret
NewSale ENDP