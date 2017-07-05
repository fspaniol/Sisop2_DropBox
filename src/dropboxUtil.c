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

int createRMFile(char iplist[]) {
    FILE *handler = NULL;
    handler = fopen("RMFile.txt", "w+");

    if (handler != NULL) {
	    
	    fprintf(handler, "%s", iplist);
	    printf("[CLIENT] Successfully created a RM File.\n");
	    fclose(handler);
	    return 0;
	} else {
		printf("[CLIENT] Could not create the RM file.\n");
		return 1;
	} 
}

char* readRMFile() {
    FILE* handler;
    handler = fopen("RMFile.txt", "r+");
   	char rmList[1024];
	char *theList;
   	int i = 0;
   	char c;

   	if (handler != NULL) {
		while ((c = getc(handler)) != EOF) {			
			rmList[i] = c;
			i++;
			putchar(c);
		}

		fclose(handler);
		theList = rmList;
    		return theList;
   	} else {
   		return NULL;
   	}
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
