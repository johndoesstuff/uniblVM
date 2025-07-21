" ~/.vim/syntax/uasm.vim

syntax keyword uniblInstruction HALT LDA STA SWP JMP JMPBZ ADDAB SUBAB LDAB STAB CMPAB VOID NANDAB
syntax keyword uniblLibInstruction CALL RET STA64 LDA64 STAB64 LDAB64 SETBZ JMPA PUSHA POPA LDA64R STA64R STB64 LDB64 STB64R LDB64R LDB STB LDPCA SETAZ SETABZ SETB1 SETB8 SETA1 INCA64 INC2A64 INC8A64 INCB64 INC8B64 INCA16 INC8A16 INC8B16 DECA64 DEC8A64 DECB64 DEC8B64 DECA16 DEC8A16 DECB16 DEC8B16 SET64 SET16 SETAV16 PUTA64 FPRINT STB16 LDB16
syntax keyword uniblDefinition CALLSTACK STACK P_STDIN P_STDOUT P_CALLSTACK P_STACK CALLSTACK_SIZE STACK_SIZE RETURN
syntax match uniblDirective /^\s*\$\(PC\|DEF\|INCLUDE\|DEBUG\)\>/
syntax match uniblMacro /^\s*\.\(MACRO\|ENMAC\)\>/

syntax match uniblNumber /\<\d\+\>/
syntax match uniblHex /0x[0-9A-Fa-f]\+/

syntax match uniblLabel /\<[A-Za-z_][A-Za-z0-9_]*\>:/ 

syntax match uniblComment /;.*$/

syntax region uniblString start=/"/ skip=/\\"/ end=/"/ contains=uniblEscape
syntax region uniblFString start=/</ skip=/\\>/ end=/>/ contains=uniblEscape

syntax match uniblEscape /\\[nrt0"\\]/

highlight link uniblString String
highlight link uniblFString String
highlight link uniblEscape SpecialChar

highlight link uniblInstruction Keyword
highlight link uniblDirective PreProc
highlight link uniblMacro Define
highlight link uniblNumber Number
highlight link uniblHex Number
highlight link uniblLabel Label
highlight link uniblComment Comment

highlight uniblLibInstruction ctermfg=DarkCyan guifg=#00aaff
highlight link uniblDefinition Constant
