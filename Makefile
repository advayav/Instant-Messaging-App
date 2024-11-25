signal_handler: signal_handler.c
	gcc -o signal_handler signal_handler.c
clean:
	rm -f signal_handler

submission:
	zip -r submission signal_handler.c README.txt Makefile

