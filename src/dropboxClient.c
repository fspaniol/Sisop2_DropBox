//
//  dropboxClient.c
//
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h>  

#include "../include/dropboxClient.h"
#include "dropboxUtil.c"


#define TAM_MAX_RECEBER 256
#define TAM_MAX_ENVIO 256

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
    
    puts("\n\n Entrei na função de enviar arquivos para o servidor");

    int handler; // Inteiro para a manipulação do arquivo que tentaremos abrir
    ssize_t bytesLidos = 0; // Estrutura para guardar a quantidade de bytes lidos pelo sistema
    ssize_t bytesEnviados = 0; // Estrutura para guardar a quantidade de bytes enviados para o servidor
    ssize_t tamanhoArquivoEnviado = 0;
    char bufferEnvio[TAM_MAX_ENVIO]; // Buffer que armazena os pacotes para enviar
    int qtdePacotes = 0;

    if ((handler = open(file, O_RDONLY)) < 0){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("Erro ao abrir o arquivo"); // Nem precisa informar o servidor, creio eu
    }
    else{

        while ((bytesLidos = read(handler, bufferEnvio, TAM_MAX_ENVIO-1)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            if ((bytesEnviados = send(socket,bufferEnvio,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
                puts("Deu erro ao enviar o arquivo");
                return;
            }
            qtdePacotes++;
            tamanhoArquivoEnviado += bytesEnviados;
        }
    }

    close(handler);


    printf("Foram enviados %zd bytes em %d pacotes de tamanho %d", tamanhoArquivoEnviado, qtdePacotes, TAM_MAX_ENVIO);
}

char *get_file_name(){
    /*
    char filename[100];

    printf("Entre com o nome do arquivo: \n");
    fgets(filename, sizeof(filename), stdin);

    printf("filename: %s\n", filename);

    return filename;*/

    char data[56];
    fpurge(stdin);
    printf("Digite o nome do arquivo: ");
    fgets(data, sizeof(data), stdin);
    printf("Arquivo escolhido: %s\n", data);
    return data;

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
            case 2:
                send_file(get_file_name(), socketCliente);
                break;
            case 3: get_file(NULL, socketCliente);
                break;
            case 0: close_connection(socketCliente);
                
        }
        
    }
    
    return 0;
}
