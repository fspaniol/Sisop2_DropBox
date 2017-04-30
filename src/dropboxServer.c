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
#include <fcntl.h> /* open, O_RDONLY */
#include <unistd.h> /* close, read */
#include <arpa/inet.h>  /* sockaddr_in, inet_ntop */


#include "../include/dropboxServer.h"
#include "dropboxUtil.c"

#define MAX_RECV_BUF 256
#define MAX_SEND_BUF 256

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
    printf("Entrou no receive\n");
    char recv_str[MAX_RECV_BUF]; /* to store received string */
    ssize_t rcvd_bytes; /* bytes received from socket */
    
    /* read name of requested file from socket */
    if ((rcvd_bytes = recv(socket, recv_str, MAX_RECV_BUF, 0)) < 0) {
        perror("Erro tentando recuperar");
        return;
    }
    sscanf (recv_str, "%s\n", file); /* discard CR/LF */
    
}

// Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void send_file(char *file, int socket){
    printf("Entrou no send\n");
    
    /*char buffer[1024];
     time_t ticks;
     
     ticks = time(NULL);
     
     strcpy(buffer,("%.24s \n", ctime(&ticks)));
     send(socket,buffer,sizeof(buffer),0);
     
     CODIGO DO FEFEZUDO*/
    
    int sent_count; /* how many sending chunks, for debugging */
    ssize_t read_bytes, /* bytes read from local file */ sent_bytes, /* bytes sent to connected socket */ sent_file_size;
    char send_buf[MAX_SEND_BUF]; /* max chunk size for sending file */
    char *errmsg_notfound = "Arquivo nao encontrado\n";
    int f; /* file handle for reading local file*/
    sent_count = 0;
    sent_file_size = 0;
    
    /* attempt to open requested file for reading */
    if((f = open(file, O_RDONLY)) < 0) /* can't open requested file */ {
        perror(file);
        if((sent_bytes = send(socket, errmsg_notfound, strlen(errmsg_notfound), 0)) < 0)
        {
            printf("Send error");
            return;
        } }
    else /* open file successful */ {
        printf("Enviando: %s\n", file);
        while( (read_bytes = read(f, send_buf, MAX_RECV_BUF)) > 0 ) {
            if( (sent_bytes = send(socket, send_buf, read_bytes, 0)) < read_bytes )
            {
                printf("Erro ao enviar");
                return;
            }
            sent_count++;
            sent_file_size += sent_bytes;
        }
        close(f);
    }
    printf("%zd bytes enviados de %d\n\n", sent_file_size, sent_count);
    //return sent_count;
    return;
    
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
                case 2: receive_file(NULL, novoSocket);
                    break;
                case 3: send_file(NULL,novoSocket);
                    break;
                case 0: printf("Cliente desconectado \n");
                    break;
            }
            
        }
        
        
    }
    
    return 0;
}
