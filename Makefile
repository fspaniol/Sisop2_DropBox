CC=gcc
INC_DIR= ./include
SRC_DIR= ./src

all: 	client server	
debug:	clientDebug serverDebug

client: clientssl: $(SRC_DIR)/dropboxClient.c
	$(CC) -o client $(SRC_DIR)/dropboxClient.c -lpthread -lssl -lcrypto

server: $(SRC_DIR)/dropboxServer.c
	$(CC) -o server $(SRC_DIR)/dropboxServer.c -lpthread -lssl -lcrypto

clientDebug: $(SRC_DIR)/dropboxClient.c
	$(CC) -o client $(SRC_DIR)/dropboxClient.c -lpthread -Wall

serverDebug: $(SRC_DIR)/dropboxServer.c
	$(CC) -o server $(SRC_DIR)/dropboxServer.c -lpthread -Wall

clean:
	rm client
	rm server
