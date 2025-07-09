" ~/.vim/syntax/uasm.vim

syntax keyword uniblInstruction LDA STA JMPA SWP ADDAB SUBAB CMPAB HALT LDPCA VOID
syntax match uniblDirective /^\s*\$\(PC\|DEF\)\>/
syntax match uniblMacro /^\s*\.\(MACRO\|ENMAC\)\>/

syntax match uniblNumber /\<\d\+\>/
syntax match uniblHex /0x[0-9A-Fa-f]\+/

syntax match uniblLabel /\<[A-Za-z_][A-Za-z0-9_]*\>:/ 

syntax match uniblComment /;.*$/

highlight link uniblInstruction Keyword
highlight link uniblDirective PreProc
highlight link uniblMacro Define
highlight link uniblNumber Number
highlight link uniblHex Number
highlight link uniblLabel Label
highlight link uniblComment Comment
