;all tunable constants (change values here, nowhere else)

MED_COUNT   = 12            ; medicines the program starts with
MAX_CART    = 10            ; max different medicines in one bill
LOW_STOCK   = 10            ; qty <= 10 counts as low stock
TODAY       = 20261103      ; "current date" (YYYYMMDD) used for expiry checks
SOON_LIMIT  = 20261203      ; expiring on or before this = "expiring soon"
TIER1       = 1000          ; subtotal at which the first discount starts (Rs)
TIER2       = 2000          ; subtotal at which the second discount starts (Rs)
PCT1        = 5             ; discount % for TIER1
PCT2        = 10            ; discount % for TIER2
MAX_RESTOCK = 1000          ; max units added in one restock
NAME_W      = 22            ; width of the name column when printing
BILLER_MAX  = 30            ; max characters in the biller's name
MAX_MED       = 30          ; max medicines the inventory can hold
NAME_LEN_MAX  = 20          ; max characters in a medicine name
NAME_REC      = 21          ; bytes per stored name (20 chars + zero)
MAX_PRICE     = 100000      ; max price per unit when adding a medicine
MAX_INIT_QTY  = 10000       ; max starting stock when adding a medicine

; File handling
FILE_BUF_SIZE   = 4096      ; bytes read from / written to a text file at once
LOG_BUF_SIZE    = 2048      ; bytes for one bill's entry in the sales log
FIO_APPEND_DATA = 4         ; access: writes always go to the end of the file
FIO_SHARE_READ  = 1         ; other programs may read the file while it is open
FIO_OPEN_ALWAYS = 4         ; open the file, or create it if it does not exist
FIO_ATTR_NORMAL = 80h       ; normal file attributes

; Text colours (used with SetTextColor)
C_RED       = 12
C_YEL       = 14
C_GRN       = 10
C_CYN       = 11
C_WHT       = 15
C_DEF       = 7
