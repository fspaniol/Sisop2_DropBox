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
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h>  


#include "../include/dropboxServer.h"
#include "dropboxUtil.c"

#define TAM_MAX_RECEBER 256
#define TAM_MAX_ENVIO 256

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
   	puts ("\n\n Vou receber o arquivo enviado pelo cliente ");
    char bufferReceber[TAM_MAX_RECEBER]; // Buffer que armazena os pacotes que vem sido recebidos
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    int handler; // Inteiro para manipulação do arquivo que botaremos no servidor

    handler = open(file, O_CREAT | O_WRONLY);


    while ((bytesRecebidos = recv(socket, bufferReceber, TAM_MAX_RECEBER, 0)) > 0){ 
    	if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
       		puts("Erro tentando receber algum pacote do cliente");
    	}

    	write(handler,bufferReceber,sizeof(bufferReceber)); // Escreve no arquivo

    	if(bytesRecebidos < TAM_MAX_RECEBER){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
    		close(handler);
    		return;
    	}
    }    
}

// Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void send_file(char *file, int socket){

    printf("Entrou no send\n");
    
    char buffer[1024];
     time_t ticks;
     
     ticks = time(NULL);
     
     strcpy(buffer,("%.24s \n", ctime(&ticks)));
     send(socket,buffer,sizeof(buffer),0);
     
     /*CODIGO DO FEFEZUDO*/
    
}



// Setando a conexão TCP com o cliente

int main(){
    
    int socketServidor, novoSocket;
    struct sockaddr_storage depositoServidor;
    socklen_t tamanhoEndereco;
    int opcao_recebida = 1;
    
    socketServidor = criaSocketServidor("127.0.0.1", 4200);
    
    // O servidor fica rodando para sempre e quando algum cliente aparece chama a função send_file para mandar algo
    // O segundo parametro do listen diz quantas conexões podemos ter
    
    while (1){
        
        printf("Servidor esperando algum cliente... \n");
        
        if ((listen(socketServidor,10)) != 0){
            printf("Servidor cheio, tente mais tarde \n");
        }
        
        tamanhoEndereco = sizeof depositoServidor;
        novoSocket = accept(socketServidor, (struct sockaddr *) &depositoServidor, &tamanhoEndereco);
        
        while (opcao_recebida != 0){ // enquanto a opção do cliente não for sair da conexao, ele fica atendendo esse cliente
            
            recv(novoSocket, &opcao_recebida, sizeof(opcao_recebida), 0); // recebe do usuario que opção ele quer
            opcao_recebida = htonl(opcao_recebida);
            
            switch(opcao_recebida) {
                case 1: sync_server();
                    break;
                case 2: receive_file("recebido.txt", novoSocket);
                    break;
                case 3: send_file(NULL,novoSocket);
                    break;
                case 0: printf("Cliente desconectado \n");
                    break;
            }
            opcao_recebida = 5;
            
        }
        
        
    }
    
    return 0;
}
