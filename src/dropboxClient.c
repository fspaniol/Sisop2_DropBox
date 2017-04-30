//
//  dropboxClient.c
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>

#include "../include/dropboxClient.h"
#include "../include/dropboxUtil.h"
#include "dropboxUtil.c"

// Conecta o cliente com o servidor
// Host - endereço do servidor
// Port - porta aguardando conexão

int connect_server(char *host, int port){

	//Primeiro parametro indica qual o dominio da internet, nesse caso IPV4
	//Segundo parametro indica que estaremos usando TCP, ao inves de UDP
	//Terceiro parametro fala pro SO usar o protocolo padrão
	int socketCliente = socket(PF_INET, SOCK_STREAM, 0); // Cria o socket do cliente
	socklen_t tamanho_endereco;

	//Setando as configurações básicas para se conectar ao servidor
	struct sockaddr_in enderecoServidor;
	enderecoServidor = retornaEndereco(host, port);

    tamanho_endereco = sizeof enderecoServidor;

    connect(socketCliente, (struct sockaddr *) &enderecoServidor, tamanho_endereco); // Conecta de fato agora o socket do cliente com o endereço do servidor

    return socketCliente; // Retorna o socket a ser usado
}

// Sincroniza o diretório "sync_dir_<nomeusuário>" com o servidor

void sync_client(){
    
}

// Envia um arquivo file para o servidor
// Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void send_file(char *file, int socket){
    
}

// Obtém um arquivo file do servidor
// Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void get_file(char *file, int socket){
	char buffer[1024];
    recv(socket, buffer, 1024, 0); // Recebe a mensagem sendo enviada pelo servidor

    printf("O servidor mandou: %s", buffer);
}

// Fecha a conexão com o servidor.

void close_connection(int socket){
    
	printf("\n Conexao encerrada\n");
    close(socket);
    
}


int main(){

  	int socketCliente = connect_server("127.0.0.1", 4200);
  	int opcao = 1;
  	int opcao_convertida;

  	while (opcao != 0){

  		puts("\n\n Qual operacao deseja realizar?");
  		puts("Digite 1 para sincronizar seu diretorio");
  		puts("Digite 2 para enviar um arquivo para o servidor");
  		puts("Digite 3 para receber um arquivo do servidor");
  		puts("Digite 0 para desconectar-se do dropbox");
  		printf("Sua escolha eh: ");

  		scanf("%d", &opcao);
  		opcao_convertida = htonl(opcao);


  		send(socketCliente,&opcao_convertida,sizeof(opcao_convertida),0); // Informa o servidor qual a opção que ele vai realizar

  		switch(opcao) {
  			case 1: sync_client();
  					  break;
  			case 2: send_file(NULL,socketCliente);
  					  break;
  			case 3: get_file(NULL, socketCliente);
  					  break;
  			case 0: close_connection(socketCliente);

  		}

  	}

  return 0;
}
