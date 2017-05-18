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
#include <sys/stat.h> 

#include "../include/dropboxServer.h"
#include "dropboxUtil.c"

#define TAM_MAX 1024

struct Client clientes[10];
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


// Ve se o usuario ja possui uma pasta, se nao, cria
void cria_pasta_usuario(char* usuario){

   struct stat st = {0};
   char diretorio[50] = "sync_dir_";
   strcat(diretorio,usuario);

    if (stat(diretorio, &st) != 0) {
        mkdir(diretorio, 07777);
        printf("[Server][User: %s] Created client folder. (Cliend didn't have a folder) \n", usuario);
    }
    else{
        printf("[Server][User: %s] Client already has folder.\n", usuario);
    }
}

void *atendeCliente(void *indice){
    int *temp = (int *) indice;
    int index = *temp;

    struct Client clienteLogado;
    clienteLogado = clientes[index];

    int opcao_recebida = 1;

    char usuario[TAM_MAX];
    int flag = 1;
    flag = htonl(flag);

    recv(clienteLogado.devices[0], clienteLogado.userid, sizeof(usuario), 0); // Recebe o numero do usuario
    cria_pasta_usuario(clienteLogado.userid);
    send(clienteLogado.devices[0], &flag, sizeof(flag), 0); // Envia o aval dizendo que ja recebeu
        
    while (opcao_recebida != 0){ // enquanto a opção do cliente não for sair da conexao, ele fica atendendo esse cliente
        opcao_recebida = 5;
        //puts("Estou esperando acao de algum cliente... \n");
            
        recv(clienteLogado.devices[0], &opcao_recebida, sizeof(opcao_recebida), 0); // recebe do usuario que opção ele quer
        opcao_recebida = htonl(opcao_recebida);
            
        switch(opcao_recebida) {
            case 1: sync_server();
                break;
            case 2: receive_file(clienteLogado.devices[0], clienteLogado.userid);
                break;
            case 3: send_file_servidor(clienteLogado.devices[0], clienteLogado.userid);
                break;
            case 0: printf("[Server][User: %s] Client %d disconnected.\n", clienteLogado.userid, index);
        }          
    }

    clientes[index].logged_in = 0; 

    return 0;
}

// Sincroniza o servidor com o diretório "sync_dir_<nomeusuário>" com o cliente

void sync_server() {

}

// Recebe um arquivo file do cliente. Deverá ser executada quando for realizar upload de um arquivo. file - path/filename.ext do arquivo a ser recebido

void receive_file(int socket, char* usuario){
   	printf("[Server][User: %s] Server will receive file from client.\n", usuario);
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes que vem sido recebidos
    ssize_t bytesRecebidos = 0; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesEnviados; 
    FILE* handler; // Inteiro para manipulação do arquivo que botaremos no servidor
    bzero(buffer, TAM_MAX);
    int flag = 1;
    flag = htonl(flag);
    char diretorio[100] = "sync_dir_";
    strcat(diretorio,usuario);
    strcat(diretorio,"/");

    while((bytesRecebidos = recv(socket, buffer, sizeof(buffer),0)) < 0){ // recebe o nome do arquivo que vai receber do cliente
    }

    printf("[Server] The file to be sent by client is: %s\n", buffer); // Escreve o nome do arquivo
    
    if ((bytesEnviados = send(socket, &flag, sizeof(flag), 0)) < 0){
        printf("[ERROR ][User: %s] Error sending acknowledgement to client for file receiving.", usuario); // Envia uma flag dizendo pro cliente que ta tudo pronto e a transferencia do conteudo do arquivo pode começar
        return;
    }

    strcat(diretorio,buffer);

    bytesRecebidos = 0; // Reseta o numero de bytes lidos

    handler = fopen(diretorio, "w"); // Abre o arquivo 

    bzero(buffer, TAM_MAX); // Reseta o buffer

    while ((bytesRecebidos = recv(socket, buffer, TAM_MAX, 0)) > 0){  // Enquanto tiver coisa sendo lida, continua lendo
    	if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
       		printf("[ERROR ][User: %s] Error when trying to receive client package.\n", usuario);
            fclose(handler);
            return;
    	}
        if (buffer[0] == '\0'){
            printf("[Server][User: %s] Client could not send file.\n", usuario);
            fclose(handler);
            return; 
        }

    	fwrite(buffer, 1, bytesRecebidos, handler); // Escreve no arquivo

        bzero(buffer, TAM_MAX); // Reseta o buffer

    	if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
    		fclose(handler);
            printf("[Server][User: %s] Successfully received client file.\n", usuario);
    		return;
    	}
    }
}

// Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void send_file_servidor(int socket, char* usuario){

    char diretorio[100] = "sync_dir_";
    strcat(diretorio,usuario);
    strcat(diretorio,"/");

    printf("[Server][User: %s] Server will receive the desired filename... \n", usuario);

    char buffer[TAM_MAX];
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesLidos;
    ssize_t bytesEnviados;
    FILE* handler;

    bzero(buffer,TAM_MAX);

    bytesRecebidos = recv(socket, buffer, TAM_MAX, 0); // recebe o nome do arquivo que o cliente quer receber
    if (bytesRecebidos < 0)
        printf("[ERROR ][[User: %s]] Error receiving the filename to be sent. \n", usuario);
    else
        printf("[Server] The requested file by the client is: %s\n", buffer); // Escreve o nome do arquivo que o cliente quer

    strcat(diretorio,buffer);

    //handler = fopen(diretorio,"r");
    if ((handler = fopen(diretorio, "r")) == NULL) {
        puts("[ERROR ] Error sending the file.");
        return;
    }

    while ((bytesLidos = fread(buffer, 1,sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
        if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
            printf("[ERROR ][User: %s] Error sending the file.", usuario);
            return;
        }
        bzero(buffer, TAM_MAX); // Reseta o buffer
    }

    fclose(handler);
}

// Setando a conexão TCP com o cliente

int main(){
    
    int socketServidor;
    struct sockaddr_storage depositoServidor[10];
    socklen_t tamanhoEndereco[10];
    pthread_t threads[10];
    int cont = 0;
    int cont2;

    for (int x = 0; x < 10; x++)
        clientes[x].logged_in = 0;
    
    socketServidor = criaSocketServidor("127.0.0.1", 53000);
    
    // O servidor fica rodando para sempre e quando algum cliente aparece chama a função send_file para mandar algo
    // O segundo parametro do listen diz quantas conexões podemos ter
    
    while (1){
        
        printf("\n[Server] Server waiting for client...\n");
        
        if ((listen(socketServidor,10)) != 0){
            printf("[ERROR] Server is full. Try again later.\n");
        }

        tamanhoEndereco[cont] = sizeof(depositoServidor[cont]);
        clientes[cont].devices[0] = accept(socketServidor, (struct sockaddr *) &depositoServidor[cont], &tamanhoEndereco[cont]);
        puts("[Server] Client connected...");
        

        clientes[cont].logged_in = 1;
        cont2 = cont; // Uma cópia para mandar para a thread, pois se mandasse a mesma variavel, ela seria alterada pela main antes de a thread secundaria pegar
        printf("[Server] Thread %d created \n", cont2+1);

        if (pthread_create(&threads[cont],NULL,atendeCliente,&cont2)){
            puts("[ERROR] Error trying to create a thread ");
            return 0;
        }

        cont = 0;

        while (clientes[cont].logged_in == 1){
            printf("[Server] Client %d is logged, searching for next slot\n", cont+1);
            cont++;
            if (cont == 10)
                cont = 0;
        }
        
    }
    
    return 0;
}
