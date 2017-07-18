CC=gcc
INC_DIR= ./include
SRC_DIR= ./src

all: 	client server	
debug:	clientDebug serverDebug

client: $(SRC_DIR)/dropboxClient.c
	$(CC) -o $(SRC_DIR)/client $(SRC_DIR)/dropboxClient.c -lpthread -lssl -lcrypto

server: $(SRC_DIR)/dropboxServer.c
	$(CC) -o $(SRC_DIR)/server $(SRC_DIR)/dropboxServer.c -lpthread -lssl -lcrypto

clientDebug: $(SRC_DIR)/dropboxClient.c
	$(CC) -o $(SRC_DIR)/client $(SRC_DIR)/dropboxClient.c -lpthread -lssl -lcrypto -Wall

serverDebug: $(SRC_DIR)/dropboxServer.c
	$(CC) -o $(SRC_DIR)/server $(SRC_DIR)/dropboxServer.c -lpthread -lssl -lcrypto -Wall

clean:
	rm $(SRC_DIR)/client
	rm $(SRC_DIR)/server
