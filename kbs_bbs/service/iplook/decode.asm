[bits 32]
global dumpstr
extern strlen
extern dumpcallback
extern exit

[SEGMENT .text USE32 CLASS=CODE]        

dumpstr:

var_26C         EQU -26Ch
s               EQU -16Ch
var_16B         EQU -16Bh
pindex1         EQU -6Ch
var_68          EQU -68h
var_64          EQU -64h
index2          EQU -60h
index1          EQU -5Ch
tempStringList  EQU -58h
pClass          EQU -54h
var_50          EQU -50h
var_40          EQU -40h
var_34          EQU -34h
var_2C          EQU -2Ch
var_28          EQU -28h
var_24          EQU -24h
var_20          EQU -20h
var_1C          EQU -1Ch
var_18          EQU -18h
var_14          EQU -14h
tempstring      EQU -10h
var_C           EQU -0Ch
var_8           EQU -8
var_4           EQU -4
arg_0           EQU  8

                push    ebp
                mov     ebp, esp
                add     esp, 0FFFFFD94h

                mov     dl, 1
                mov     esi, [ebp+arg_0]
                mov     eax, [esi]
                mov     [ebp+index1], eax
                mov     edx, [esi+4]
                mov     [ebp+index2], edx
                mov     ecx, [ebp+index1]
                lea     eax, [esi+ecx]
                mov     [ebp+pindex1], eax
                mov     edx, [ebp+index1]
                cmp     edx, [ebp+index2]
                jg     near  loc_4021BD

loc_402038:                             ; CODE XREF: sub_401F2C+28Bj
                mov     [ebp+s], byte 0
                mov     [ebp+var_16B], byte 0
                mov     ecx, [ebp+pindex1]
                mov     eax, [ecx]
                mov     [ebp+var_64], eax
                mov     edx, [ebp+pindex1]
                mov     ebx, [edx+4]
                and     ebx, 0FFFFFFh
                mov     eax, [esi+ebx]
                add     ebx, 4
                mov     [ebp+var_68], eax
                mov     al, [esi+ebx]
                movsx   edx, al
                dec     edx
                jnz     short loc_402079
                mov     ebx, [esi+ebx+1]
                and     ebx, 0FFFFFFh
                mov     al, [esi+ebx]

loc_402079:                             ; CODE XREF: sub_401F2C+13Ej
                movsx   edx, al
                cmp     edx, 2
                jnz     short loc_4020B9
                mov     edi, [esi+ebx+1]
                push    esi
                and     edi, 0FFFFFFh
                xor     eax, eax
                add     edi, esi
                or      ecx, 0FFFFFFFFh
                push    edi
                lea     esi, [ebp+s]
                repne scasb
                not     ecx
                sub     edi, ecx
                mov     edx, ecx
                xchg    esi, edi
                shr     ecx, 2
                mov     eax, edi
                repe movsd
                mov     ecx, edx
                and     ecx, 3
                repe movsb
                pop     edi
                pop     esi
                add     ebx, 4
                jmp     short loc_4020F5
; 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�

loc_4020B9:                             ; CODE XREF: sub_401F2C+153j
                lea     eax, [ebx+esi]
                push    esi
                mov     edi, eax
                xor     eax, eax
                or      ecx, 0FFFFFFFFh
                lea     esi, [ebp+s]
                repne scasb
                not     ecx
                sub     edi, ecx
                mov     edx, ecx
                xchg    esi, edi
                shr     ecx, 2
                mov     eax, edi
                repe movsd
                mov     ecx, edx
                lea     eax, [ebp+s]
                and     ecx, 3
                repe movsb
                pop     esi
                push    eax             ; s
                call    strlen
                pop     ecx
                add     eax, ebx
                inc     eax
                mov     ebx, eax

loc_4020F5:                             ; CODE XREF: sub_401F2C+18Bj
                mov     al, [esi+ebx]
                movsx   edx, al
                cmp     edx, 2
                jnz     short loc_402135
                mov     edi, [esi+ebx+1]
                push    esi
                and     edi, 0FFFFFFh
                xor     eax, eax
                add     edi, esi
                or      ecx, 0FFFFFFFFh
                push    edi
                lea     esi, [ebp+var_26C]
                repne scasb
                not     ecx
                sub     edi, ecx
                mov     edx, ecx
                xchg    esi, edi
                shr     ecx, 2
                mov     eax, edi
                repe movsd
                mov     ecx, edx
                and     ecx, 3
                repe movsb
                pop     edi
                pop     esi
                jmp     short loc_40215E
loc_402135:                             ; CODE XREF: sub_401F2C+1D2j
                add     ebx, esi
                xor     eax, eax
                push    esi
                mov     edi, ebx
                or      ecx, 0FFFFFFFFh
                lea     esi, [ebp+var_26C]
                repne scasb
                not     ecx
                sub     edi, ecx
                mov     edx, ecx
                xchg    esi, edi
                shr     ecx, 2
                mov     eax, edi
                repe movsd
                mov     ecx, edx
                and     ecx, 3
                repe movsb
                pop     esi

loc_40215E:                             ; CODE XREF: sub_401F2C+207j
                mov     [ebp+var_40], word 14h
                lea     eax, [ebp+s]
                lea     edx, [ebp+var_26C]
                push    eax
                push    edx
                mov     ecx, [ebp+var_68]
                mov     edx, [ebp+var_64]
                push    ecx
                push    edx
                call    dumpcallback
                add     esp, 16
                add     [ebp+index1], dword 7
                add     [ebp+pindex1], dword 7
                mov     ecx, [ebp+index1]
                cmp     ecx, [ebp+index2]
                jle     near loc_402038
loc_4021BD:                             ; CODE XREF: sub_401F2C+106j

		push   dword 0
		call    exit
                mov     esp, ebp
                pop     ebp
                retn


