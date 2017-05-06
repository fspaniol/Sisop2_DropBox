//
//  dropboxUtil.c
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/dropboxUtil.h"

#define OK 			0
#define NO_INPUT 	1
#define TOO_LONG 	2

// Cria o endereco do servidor

struct sockaddr_in retornaEndereco(char *host, int port){

		struct sockaddr_in enderecoServidor; // Cria uma instancia da estrutura de endereco ip 
    	enderecoServidor.sin_family = AF_INET; // Declara que o endereco do servidor pertence a internet
    	enderecoServidor.sin_port = htons(port); // Indica qual porta estaremos usando
    	enderecoServidor.sin_addr.s_addr = inet_addr(host); // Indica qual o IP de fato estaremos usando
    	memset(enderecoServidor.sin_zero, '\0', sizeof enderecoServidor.sin_zero); // Seta todos os bits do padding pra 0

    return enderecoServidor;
}

static int getLine (char *prmpt, char *buff, size_t sz) {
	int ch, extra;

	if (prmpt != NULL) {
		printf("%s", prmpt);
		fflush(stdout);
	}
	if (fgets (buff, sz, stdin) == NULL)
		return NO_INPUT;

	if (buff[strlen(buff)-1] != '\n') {
		extra = 0;
		while (((ch = getchar()) != '\n') && (ch != EOF))
			extra = 1;
		return (extra == 1) ? TOO_LONG : OK;
	}

	buff[strlen(buff)-1] = '\0';
	// printf("sz: %lu", sz);
	return OK;
}
/*
int main () {
	int rc;
	char myBuffer[10];

	while (1) {
		printf("Digite o nome do arquivo desejado: \n");
		getLine(">> ", myBuffer, sizeof(myBuffer));
		if (rc == NO_INPUT) {
			printf("No input.\n");
			break;
		}

		if (rc == TOO_LONG) {
			printf("Input too long [%s]\n", myBuffer);
			//return 0;
		}

		printf("OK. [%s]\n", myBuffer);
		//return 0;
	}
	return 0;
}*/

void imprimir_menu(){
	puts("\n\n Qual operacao deseja realizar?");
    puts("Digite 1 para sincronizar seu diretorio");
    puts("Digite 2 para enviar um arquivo para o servidor");
    puts("Digite 3 para receber um arquivo do servidor");
    puts("Digite 0 para desconectar-se do dropbox");
    printf("Sua escolha eh: ");
}
