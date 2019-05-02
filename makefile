.PRECIOUS: myshell

myshell: myshell.c
	clear

	gcc --std=c99 -Wall -Werror -o myshell myshell.c

	clear

	@echo Launching testa shell...

	./myshell

	@echo
	@echo
