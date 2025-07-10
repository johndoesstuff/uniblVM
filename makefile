TARGET:
	gcc vm/unibl_vm.c -o unibl_vm
	flex -o assembler/build/lex.asm.c assembler/lexer.l
	flex -o assembler/build/lex.pp.c assembler/preprocessor.l
	bison -H assembler/parser.y -o assembler/build/parser.tab.c -p asm_
	bison -H assembler/preprocessor.y -o assembler/build/preprocessor.tab.c -p pp_
	gcc -g -o unibl_asm \
		assembler/unibl_codegen.c \
		assembler/main.c \
		assembler/unibl_preprocessor.c \
		assembler/build/parser.tab.c \
		assembler/build/preprocessor.tab.c \
		assembler/build/lex.asm.c \
		assembler/build/lex.pp.c
	mkdir -p ~/.vim/syntax
	sudo cp uasm.vim ~/.vim/syntax/uasm.vim
	mkdir -p ~/.vim/ftdetect
	echo "au BufRead,BufNewFile *.uasm set filetype=uasm" > ~/.vim/ftdetect/uasm.vim
