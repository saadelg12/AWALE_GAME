BIN_DIR = bin
UTIL_DIR = util
BCRYPT_DIR = $(UTIL_DIR)/libbcrypt

TARGET_SERVER = bin/server

CC = gcc
CFLAGS = -I$(BCRYPT_DIR) $(shell grep '^CFLAGS = ' $(BCRYPT_DIR)/crypt_blowfish/Makefile | cut -d= -f2-)

# Targets
$(TARGET_SERVER): directories bin/server.o bin/server_client_manager.o bin/server_command_handler.o bin/server_command_router.o bin/game_logic.o $(BCRYPT_DIR)/bcrypt.a
	$(CC) -o $(TARGET_SERVER) bin/server.o bin/server_client_manager.o bin/server_command_handler.o bin/server_command_router.o bin/game_logic.o $(BCRYPT_DIR)/bcrypt.a

# Directory creation
directories:
	@mkdir -p $(BIN_DIR)

# Bcrypt library
$(BCRYPT_DIR)/bcrypt.a:
	$(MAKE) -C $(BCRYPT_DIR)

# Object files
bin/server.o: server.c
	$(CC) -c server.c -o bin/server.o

bin/server_client_manager.o: server_client_manager.c
	$(CC) -c server_client_manager.c -o bin/server_client_manager.o

bin/server_command_handler.o: server_command_handler.c
	$(CC) -c server_command_handler.c -o bin/server_command_handler.o

bin/server_command_router.o: server_command_router.c
	$(CC) -c server_command_router.c -o bin/server_command_router.o

bin/game_logic.o: game_logic.c
	$(CC) -c game_logic.c -o bin/game_logic.o

# Clean up
clean:
	rm -Rf $(BIN_DIR)
	$(MAKE) -C $(BCRYPT_DIR) clean