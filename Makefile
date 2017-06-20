CC=gcc
INC_DIR= ./include
SRC_DIR= ./src

all: 	client server	
debug:	clientDebug serverDebug

client: $(SRC_DIR)/dropboxClient.c
	$(CC) -o client $(SRC_DIR)/dropboxClient.c -lpthread

server: $(SRC_DIR)/dropboxServer.c
	$(CC) -o server $(SRC_DIR)/dropboxServer.c -lpthread

clientDebug: $(SRC_DIR)/dropboxClient.c
	$(CC) -o client $(SRC_DIR)/dropboxClient.c -lpthread -Wall

serverDebug: $(SRC_DIR)/dropboxServer.c
	$(CC) -o server $(SRC_DIR)/dropboxServer.c -lpthread -Wall

clean:
	rm client
	rm server
