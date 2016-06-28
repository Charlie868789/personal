#Atmel9260 Client Makefile
CC=arm-none-linux-gnueabi-gcc 
#CC=gcc
obj=atmel9260_main.o atmel9260_communication.o atmel9260file.o sha1sum.o socket.o
atmel_client.v1.0: $(obj)
	@$(CC) $(obj) -o atmel9260.v1.0
atmel9260_main.o: atmel9260_main.c
	@$(CC) -c atmel9260_main.c
atmel9260_communication.o: atmel9260_communication.c
	@$(CC) -c atmel9260_communication.c
atmel9260file.o: atmel9260file.c
	@$(CC) -c atmel9260file.c
socket.o:socket.c
	@$(CC) -c socket.c
sha1sum.o:sha1sum.c
	@$(CC) -c sha1sum.c
.PHONY:clean
clean:
	rm -rf atmel9260_main.o atmel9260_communication.o atmel9260file.o socket.o sha1sum.o atmel9260.v1.0

