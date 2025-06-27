TARGET:
	gcc vm/unibl_vm.c -o unibl_vm
	gcc assembler/unibl_asm.c -o unibl_asm
	flex -o assembler/build/lex.yy.c assembler/lexer.l
	./unibl_asm
