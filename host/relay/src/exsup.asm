; exsup.asm — Define __except_list as absolute symbol at TEB offset 0.
; The VS2003 compiler and CRT objects reference fs:__except_list for SEH
; chain access. This MUST resolve to fs:[0] (TEB exception list head),
; NOT to a data variable address.
.386
.model flat
PUBLIC __except_list
__except_list EQU 0
END
