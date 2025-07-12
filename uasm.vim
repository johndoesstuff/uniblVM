" ~/.vim/syntax/uasm.vim

syntax keyword uniblInstruction HALT LDA STA SWP JMPA JMPBZ ADDAB SUBAB LDAB STAB CMPAB VOID LDPCA
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
