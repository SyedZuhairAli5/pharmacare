.data
medID       DWORD 101,102,103,104,105,106,107,108,109,110,111,112
            DWORD (MAX_MED-MED_COUNT) DUP(0)
medPrice    DWORD 120,180,650,95,210,45,150,320,580,30,250,140
            DWORD (MAX_MED-MED_COUNT) DUP(0)
medQty      DWORD 100,8,40,0,25,60,5,30,12,200,70,15
            DWORD (MAX_MED-MED_COUNT) DUP(0)
medExpiry   DWORD 20271215,20270301,20261120,20260915,20270630,20281001
            DWORD 20261130,20270810,20261005,20270101,20280215,20271120
            DWORD (MAX_MED-MED_COUNT) DUP(0)
medSold     DWORD MAX_MED DUP(0)

n0  BYTE "Panadol 500mg",0
n1  BYTE "Brufen 400mg",0
n2  BYTE "Augmentin 625mg",0
n3  BYTE "Arinac Forte",0
n4  BYTE "Calpol Syrup",0
n5  BYTE "Disprin",0
n6  BYTE "Flagyl 400mg",0
n7  BYTE "Risek 20mg",0
n8  BYTE "Ventolin Inhaler",0
n9  BYTE "ORS Sachet",0
n10 BYTE "Vitamin C 500mg",0
n11 BYTE "Zyrtec 10mg",0

medNamePtr  DWORD OFFSET n0,OFFSET n1,OFFSET n2,OFFSET n3,OFFSET n4,OFFSET n5
            DWORD OFFSET n6,OFFSET n7,OFFSET n8,OFFSET n9,OFFSET n10,OFFSET n11
            DWORD (MAX_MED-MED_COUNT) DUP(0)

medCount     DWORD MED_COUNT                   ;medicines currently stored
newNameCount DWORD 0                           ;name slots used by added medicines
newNameBuf   BYTE MAX_MED*NAME_REC DUP(0)      ;storage for names of added medicines

;cart and current bill 
cartIdx     DWORD MAX_CART DUP(0)      ;medicine index of each cart line
cartQty     DWORD MAX_CART DUP(0)      ;quantity of each cart line
cartCount   DWORD 0
bSub        DWORD 0                    ;subtotal
bDisc       DWORD 0                    ;discount amount
bPct        DWORD 0                    ;discount percent
bTotal      DWORD 0                    ;total due
bPaid       DWORD 0
bChange     DWORD 0

;session totals 
totalSales      DWORD 0
totalUnitsSold  DWORD 0
totalDiscount   DWORD 0
billCount       DWORD 0
billerName      BYTE BILLER_MAX+1 DUP(0)   ; name of the person making the bill

;file handling 
invFileName  BYTE "inventory.txt",0
logFileName  BYTE "sales_log.txt",0
fileHandle   DWORD 0                           ;handle of the file being used
fileBytes    DWORD 0                           ;byte count for WriteToFile
fileBuf      BYTE FILE_BUF_SIZE+1 DUP(0)       ;text read from / written to a file
logBuf       BYTE LOG_BUF_SIZE DUP(0)          ;one bill's text for the sales log
tmpID        DWORD 0                           ;a record being parsed from the file
tmpPrice     DWORD 0
tmpQty       DWORD 0
tmpExpiry    DWORD 0

;menu and table 
sBanner   BYTE "========================================",0Dh,0Ah
          BYTE "        PHARMACARE  -  Main Menu        ",0Dh,0Ah
          BYTE "========================================",0Dh,0Ah,0
sMenu     BYTE " 1. Display all medicines",0Dh,0Ah
          BYTE " 2. Search medicine by ID",0Dh,0Ah
          BYTE " 3. Restock medicine",0Dh,0Ah
          BYTE " 4. Inventory value",0Dh,0Ah
          BYTE " 5. New sale (billing)",0Dh,0Ah
          BYTE " 6. Low / zero stock medicines",0Dh,0Ah
          BYTE " 7. Expired / expiring soon medicines",0Dh,0Ah
          BYTE " 8. Sales summary",0Dh,0Ah
          BYTE " 9. Inventory report",0Dh,0Ah
          BYTE "10. Add new medicine",0Dh,0Ah
          BYTE "11. Remove medicine",0Dh,0Ah
          BYTE "12. View sales log (from file)",0Dh,0Ah
          BYTE "13. Exit",0Dh,0Ah
          BYTE "----------------------------------------",0Dh,0Ah
          BYTE "Enter choice: ",0

sHeader   BYTE "ID    Name                  Price   Qty   Expiry      Status",0Dh,0Ah
          BYTE "---------------------------------------------------------------",0Dh,0Ah,0

sOK       BYTE "OK",0
sLow      BYTE "LOW STOCK",0
sOut      BYTE "OUT OF STOCK",0
sExpired  BYTE "EXPIRED",0
sSoon     BYTE "EXPIRING SOON",0

;messages 
msgInvalid   BYTE "Invalid input. Enter a whole number: ",0
msgBadChoice BYTE "Invalid menu choice.",0Dh,0Ah,0
msgAskID     BYTE "Enter medicine ID (0 to finish/cancel): ",0
msgNotFound  BYTE "Medicine ID not found.",0Dh,0Ah,0
msgAskQty    BYTE "Enter quantity: ",0
msgBadQty    BYTE "Quantity must be greater than 0.",0Dh,0Ah,0
msgNoStock   BYTE "Warning: medicine is out of stock and cannot be sold.",0Dh,0Ah,0
msgExpBlock  BYTE "Warning: medicine is EXPIRED and cannot be sold.",0Dh,0Ah,0
msgTooMany   BYTE "Warning: requested quantity exceeds available stock.",0Dh,0Ah,0
msgCartFull  BYTE "Cart is full (max 10 different medicines).",0Dh,0Ah,0
msgAdded     BYTE "Added to cart.",0Dh,0Ah,0
msgEmptyCart BYTE "Cart is empty. Sale cancelled.",0Dh,0Ah,0
msgCancel    BYTE "Payment cancelled. Sale not recorded.",0Dh,0Ah,0
msgAskPay    BYTE "Enter amount paid (0 to cancel): Rs ",0
msgShort     BYTE "Amount is less than the total due. Try again.",0Dh,0Ah,0
msgAskRest   BYTE "Enter quantity to add (1-1000): ",0
msgBadRest   BYTE "Invalid restock quantity (must be 1 to 1000).",0Dh,0Ah,0
msgRestocked BYTE "Stock updated successfully.",0Dh,0Ah,0
msgNone      BYTE "  (none)",0Dh,0Ah,0
msgSaleDone  BYTE "Sale completed. Inventory updated.",0Dh,0Ah,0
msgMedFull   BYTE "Inventory is full (max 30 medicines).",0Dh,0Ah,0
msgPoolFull  BYTE "No space left to store new medicine names.",0Dh,0Ah,0
msgAskNewID  BYTE "Enter new medicine ID (positive number): ",0
msgBadID     BYTE "ID must be greater than 0.",0Dh,0Ah,0
msgDupID     BYTE "A medicine with this ID already exists.",0Dh,0Ah,0
msgAskName   BYTE "Enter medicine name (max 20 characters): ",0
msgAskPrice  BYTE "Enter price per unit (Rs): ",0
msgBadPrice  BYTE "Price must be between 1 and 100000.",0Dh,0Ah,0
msgAskInitQty BYTE "Enter starting stock quantity (0-10000): ",0
msgBadInitQty BYTE "Quantity must be between 0 and 10000.",0Dh,0Ah,0
msgMedAdded  BYTE "Medicine added successfully.",0Dh,0Ah,0
msgAskConfirm BYTE "Remove this medicine? (1 = Yes, 0 = No): ",0
msgRemoved   BYTE "Medicine removed.",0Dh,0Ah,0
msgRemoveCancel BYTE "Removal cancelled.",0Dh,0Ah,0
msgAskBiller BYTE "Enter biller name: ",0
msgBadName   BYTE "Name cannot be empty.",0Dh,0Ah,0
msgNewBatch  BYTE "Enter the expiry date of the new batch:",0Dh,0Ah,0
msgAskDay    BYTE "  Day   (1-31)  : ",0
msgAskMonth  BYTE "  Month (1-12)  : ",0
msgAskYear   BYTE "  Year  (e.g. 2027): ",0
msgBadDate   BYTE "Invalid date. It must be a real date that is not before today.",0Dh,0Ah,0

msgNoInvFile  BYTE "inventory.txt not found - creating it from the built-in data.",0Dh,0Ah,0
msgBadInvFile BYTE "inventory.txt has no valid records - using the built-in data.",0Dh,0Ah,0
msgSaved      BYTE "Inventory saved to inventory.txt.",0Dh,0Ah,0
msgSaveFail   BYTE "Warning: could not write inventory.txt.",0Dh,0Ah,0
msgLogFail    BYTE "Warning: could not write sales_log.txt.",0Dh,0Ah,0
msgNoLog      BYTE "No sales log yet (complete a sale first).",0Dh,0Ah,0

;sales log file 
lgBill    BYTE "Bill #",0
lgBy      BYTE " | Biller: ",0
lgSub     BYTE " | Subtotal: Rs ",0
lgDisc    BYTE " | Discount: Rs ",0
lgTotal   BYTE " | Total: Rs ",0
lgPaid    BYTE " | Paid: Rs ",0
lgChange  BYTE " | Change: Rs ",0
lgIndent  BYTE "    ",0
lgX       BYTE " x ",0
lgAt      BYTE " @ Rs ",0
lgEq      BYTE " = Rs ",0
lgNL      BYTE 0Dh,0Ah,0
lblLogHdr BYTE "================ SALES LOG (sales_log.txt) ================",0Dh,0Ah,0

;labels 
lblInvVal    BYTE "Total inventory value : Rs ",0
lblSub       BYTE "Subtotal              : Rs ",0
lblDiscA     BYTE "Discount (",0
lblDiscB     BYTE "%)         : Rs ",0
lblTotal     BYTE "TOTAL DUE             : Rs ",0
lblPaid      BYTE "Amount paid           : Rs ",0
lblChange    BYTE "Change                : Rs ",0
lblRcptHdr   BYTE "========== PHARMACARE RECEIPT ==========",0Dh,0Ah,0
lblBillNo    BYTE "Bill No: ",0
lblBiller    BYTE "Billed by: ",0
lblRcptCols  BYTE "Item                  Qty   Price     Total",0Dh,0Ah
             BYTE "----------------------------------------------",0Dh,0Ah,0
lblRcptEnd   BYTE "==== Thank you for choosing PharmaCare ====",0Dh,0Ah,0
lblLine      BYTE "----------------------------------------------",0Dh,0Ah,0

lblSalesHdr  BYTE "=========== SALES SUMMARY (this session) ===========",0Dh,0Ah,0
lblBills     BYTE "Bills generated       : ",0
lblUnits     BYTE "Total units sold      : ",0
lblSales     BYTE "Total sales           : Rs ",0
lblDiscGiven BYTE "Total discount given  : Rs ",0
lblAvg       BYTE "Average bill          : Rs ",0
lblTop       BYTE "Top seller            : ",0
lblTopUnits  BYTE "  (units sold: ",0
lblNoSales   BYTE "No sales recorded yet.",0Dh,0Ah,0

lblRepHdr    BYTE "============== INVENTORY REPORT ==============",0Dh,0Ah,0
lblMedCount  BYTE "Medicines in inventory: ",0
lblLowCnt    BYTE "Low stock (<=10)      : ",0
lblZeroCnt   BYTE "Out of stock          : ",0
lblExpCnt    BYTE "Expired               : ",0
lblSoonCnt   BYTE "Expiring within 30 d  : ",0
lblLowList   BYTE "Low-stock medicines:",0Dh,0Ah,0
lblExpList   BYTE "--- EXPIRED MEDICINES ---",0Dh,0Ah,0
lblSoonList  BYTE "--- EXPIRING WITHIN 30 DAYS ---",0Dh,0Ah,0
lblLowHdr    BYTE "--- LOW / ZERO STOCK MEDICINES ---",0Dh,0Ah,0
