ENTRY (main)

SECTIONS
{
. = 0x218000; /* Link all pointers with + 0x218000 */
.data : /* Initialized data. */
{
_DataStart = . ;
*(.data)
_DataEnd = . ;
}

.bss : /* Uninitialized data. */
{
_BssStart = . ;
*(.bss)
_BssEnd = . ;
}

.text : /* The actual instructions. */
{
*(.text)
*(.got)
*(.got.plt)
*(.rodata)
*(.glue_7)
*(.glue_7t)
}
}
