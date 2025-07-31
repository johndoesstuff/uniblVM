" ~/.vim/syntax/uasm.vim

syntax keyword uniblInstruction HALT LDA STA SWP JMP JMPBZ ADDAB SUBAB CMPAB VOID NANDAB SHRA
syntax match uniblMacroInstruction /^\s*\zs\w\+\ze/ containedin=ALLBUT,uniblInstruction,uniblLabel
syntax keyword uniblDefinition CALLSTACK STACK P_STDIN P_STDOUT P_CALLSTACK P_STACK CALLSTACK_SIZE STACK_SIZE RETURN STDIN STDOUT STDOUT_FLUSH STDOUT_MODE
syntax match uniblDirective /^\s*\$\(PC\|DEF\|INCLUDE\|DEBUG\|DUMP\)\>/
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

highlight uniblMacroInstruction ctermfg=DarkCyan guifg=#00aaff
highlight link uniblDefinition Constant
