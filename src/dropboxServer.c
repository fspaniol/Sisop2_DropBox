//
//  dropboxServer.c
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h> 

#include "../include/dropboxServer.h"
#include "dropboxUtil.c"

//Cria o socket do servidor

int criaSocketServidor(char *host, int port){
	int socketServidor;
	struct sockaddr_in enderecoServidor;

	//Primeiro parametro indica qual o dominio da internet, nesse caso IPV4
	//Segundo parametro indica que estaremos usando TCP, ao inves de UDP
	//Terceiro parametro fala pro SO usar o protocolo padrão
	socketServidor = socket(PF_INET, SOCK_STREAM, 0);

	enderecoServidor = retornaEndereco(host,port);

	bind(socketServidor, (struct sockaddr *) &enderecoServidor, sizeof(enderecoServidor)); // Conecta o socket com o endereço do servidor


	return socketServidor;
}

// Sincroniza o servidor com o diretório "sync_dir_<nomeusuário>" com o cliente

void sync_server(){
    
}

// Recebe um arquivo file do cliente. Deverá ser executada quando for realizar upload de um arquivo. file - path/filename.ext do arquivo a ser recebido

void receive_file(char *file, int socket){
    
}

// Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void send_file(char *file, int socket){

	char buffer[1024];
	time_t ticks;

	ticks = time(NULL);

	strcpy(buffer,("%.24s \n", ctime(&ticks)));
	send(socket,buffer,sizeof(buffer),0);
    
}

// Setando a conexão TCP com o cliente

int main(){
    
  int socketServidor, novoSocket;
  struct sockaddr_storage depositoServidor;
  socklen_t tamanhoEndereco;

  socketServidor = criaSocketServidor("127.0.0.1", 4200);

  // O servidor fica rodando para sempre e quando algum cliente aparece chama a função send_file para mandar algo
  // O segundo parametro do listen diz quantas conexões podemos ter

  while (1){

  	printf("Servidor esperando algum cliente... \n");

  	listen(socketServidor,10);

  	tamanhoEndereco = sizeof depositoServidor;
  	novoSocket = accept(socketServidor, (struct sockaddr *) &depositoServidor, &tamanhoEndereco);

  	send_file(NULL,novoSocket);
}

  return 0;
}