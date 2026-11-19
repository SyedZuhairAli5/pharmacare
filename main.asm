INCLUDE Irvine32.inc
INCLUDE constants.inc
INCLUDE data.inc

.code
main PROC
    call LoadInventory          ; read inventory.txt (or create it on first run)
  
menuLoop:
    call ShowMenu
    call GetInt
    cmp eax, 13
    je exitProgram
    cmp eax, 1
    jne chk2
    call Clrscr
    call DisplayAll
    jmp afterOp

chk2:
    cmp eax, 2
    jne chk3
    call SearchMedicine
    jmp afterOp

chk3:
    cmp eax, 3
    jne chk4
    call RestockMedicine
    call SaveInventory
    jmp afterOp

chk4:
    cmp eax, 4
    jne chk5
    call CalcInventoryValue
    mov edx, OFFSET lblInvVal
    call PrintLV
    jmp afterOp

chk5:
    cmp eax, 5
    jne chk6
    call NewSale
    call SaveInventory
    jmp afterOp

chk6:
    cmp eax, 6
    jne chk7
    mov edx, OFFSET lblLowHdr
    call WriteString
    mov ebx, 1
    call ListFiltered
    jmp afterOp

chk7:
    cmp eax, 7
    jne chk8
    mov edx, OFFSET lblExpList
    call WriteString
    mov ebx, 2
    call ListFiltered
    call Crlf
    mov edx, OFFSET lblSoonList
    call WriteString
    mov ebx, 3
    call ListFiltered
    jmp afterOp

chk8:
    cmp eax, 8
    jne chk9
    call SalesSummary
    jmp afterOp

chk9:
    cmp eax, 9
    jne chk10
    call InventoryReport
    jmp afterOp

chk10:
    cmp eax, 10
    jne chk11
    call AddMedicine
    call SaveInventory
    jmp afterOp

chk11:
    cmp eax, 11
    jne chk12
    call RemoveMedicine
    call SaveInventory
    jmp afterOp

chk12:
    cmp eax, 12
    jne badChoice
    call ViewSalesLog
    jmp afterOp

badChoice:
    mov edx, OFFSET msgBadChoice
    call PrintWarn

afterOp:
    call Crlf
    call WaitMsg
    jmp menuLoop

exitProgram:
    call Clrscr
    exit
main ENDP

INCLUDE helpers.inc
INCLUDE inventory.inc
INCLUDE catalog.inc
INCLUDE billing.inc
INCLUDE monitoring.inc
INCLUDE reports.inc
INCLUDE fileio.inc

END main