//
//  dropboxUtil.c
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <sys/socket.h>
#include <netinet/in.h>

#include "../include/dropboxUtil.h"

// Cria o endereco do servidor

struct sockaddr_in retornaEndereco(char *host, int port){

		struct sockaddr_in enderecoServidor; // Cria uma instancia da estrutura de endereco ip 
    	enderecoServidor.sin_family = AF_INET; // Declara que o endereco do servidor pertence a internet
    	enderecoServidor.sin_port = htons(port); // Indica qual porta estaremos usando
    	enderecoServidor.sin_addr.s_addr = inet_addr(host); // Indica qual o IP de fato estaremos usando
    	memset(enderecoServidor.sin_zero, '\0', sizeof enderecoServidor.sin_zero); // Seta todos os bits do padding pra 0

    return enderecoServidor;
}

// Cria um arquivo para salvar os ips de replica-managers

FILE* createRMFile() {
    FILE *handler;

    return handler;
}

char* readRMFile() {
    FILE* handler;
    char *rmList;

    return rmList;
}

void imprimir_menu(char *user) {
	printf("\n\n");
	printf("[Server] Greetings, %s. What do you want to do?\n", user);
    puts("\t [1] to syncronize your directory.");
    puts("\t [2] to send a file to the server.");
    puts("\t [3] to receive a file from the server.");
    puts("\t [4] to list the files in your directory.");
    puts("\t [0] to disconnect from the server.");
    printf(">> ");
}