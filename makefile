TARGET:
	gcc vm/unibl_vm.c -o unibl_vm
	flex -o assembler/build/lex.yy.c assembler/lexer.l
	bison -H assembler/parser.y -o assembler/build/parser.tab.c
	gcc -o unibl_asm assembler/unibl_codegen.c assembler/main.c assembler/build/parser.tab.c assembler/build/lex.yy.c
