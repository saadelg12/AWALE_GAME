BIN_DIR = bin
TARGET_CLIENT = bin/client

$(TARGET_CLIENT): directories bin/client.o bin/client_command_parser.o 
	gcc -o bin/client bin/client.o bin/client_command_parser.o

directories:
	@mkdir -p $(BIN_DIR)

bin/client.o: client.c
	gcc -c client.c -o bin/client.o

bin/client_command_parser.o: client_command_parser.c
	gcc -c client_command_parser.c -o bin/client_command_parser.o

clean:
	rm -Rf $(BIN_DIR) 
