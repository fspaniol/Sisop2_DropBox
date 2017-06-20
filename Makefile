CC=gcc
INC_DIR= ./include
SRC_DIR= ./src

all: 	client server	

client: $(SRC_DIR)/dropboxClient.c
	$(CC) -o client $(SRC_DIR)/dropboxClient.c -lpthread

server: $(SRC_DIR)/dropboxServer.c
	$(CC) -o server $(SRC_DIR)/dropboxServer.c -lpthread

clean:
	rm client
	rm server
