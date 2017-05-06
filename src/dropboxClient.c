//
//  dropboxClient.c
//
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h>  

#include "../include/dropboxClient.h"
#include "dropboxUtil.c"


#define TAM_MAX 1024

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

// Le alguma informacao do stdin escrevendo alguma mensagem passada como parametro

void get_info(char* buffer, char* mensagem){

    fflush(stdin);
    __fpurge(stdin); //fpurge
    printf("\n %s ", mensagem);
    fgets(buffer,50,stdin);
    strtok(buffer, "\n");
    printf("Informacao requisitada: %s\n", buffer);
    fflush(stdin);
    __fpurge(stdin);
}

// Sincroniza o diretório "sync_dir_<nomeusuário>" com o servidor

void sync_client(){
    
}

// Envia um arquivo file para o servidor
// Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void send_file_cliente(int socket){
    
    puts("\n\n Entrei na função de enviar arquivos para o servidor");

    FILE* handler; // Inteiro para a manipulação do arquivo que tentaremos abrir
    ssize_t bytesLidos = 0; // Estrutura para guardar a quantidade de bytes lidos pelo sistema
    ssize_t bytesEnviados = 0; // Estrutura para guardar a quantidade de bytes enviados para o servidor
    ssize_t tamanhoArquivoEnviado = 0;
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes para enviar
    int qtdePacotes = 0;
    int avalServidor = 0;

    get_info(buffer,"Digite o arquivo que queres enviar: ");


    if ((bytesEnviados = send(socket,buffer,sizeof(buffer),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
        puts("Deu erro ao enviar o nome do arquivo para o servidor");
        return;
    }

    while(avalServidor == 0){
        recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(buffer, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("Erro ao abrir o arquivo"); // Nem precisa informar o servidor, creio eu
    }
    else{
        bzero(buffer, TAM_MAX); // Limpa o buffer
        while ((bytesLidos = fread(buffer, 1,sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            printf("\n Bytes Lidos: %zd \n", bytesLidos);
            if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
                puts("Deu erro ao enviar o arquivo");
                return;
            }
            bzero(buffer, TAM_MAX); // Limpa o buffer
            qtdePacotes++;
            tamanhoArquivoEnviado += bytesEnviados;
        }
        fclose(handler);
    }


    printf("Foram enviados %zd bytes em %d pacotes de tamanho %d\n", tamanhoArquivoEnviado, qtdePacotes, TAM_MAX);
}

// Obtém um arquivo file do servidor
// Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void get_file(int socket){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    bzero(buffer, TAM_MAX);

    get_info(buffer,"Digite o arquivo que queres baixar: ");

    if ((send(socket,buffer,sizeof(buffer),0)) < 0) // Envia o nome do arquivo que deseja receber pro Servidor
        puts("Erro ao enviar o nome do arquivo...");

    handler = fopen(buffer,"w"); // Abre o arquivo no qual vai armazenar as coisas, por enquanto hard-coded "clienteRecebeu.txt"

    bzero(buffer, TAM_MAX);

    while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
        if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
            puts("Erro tentando receber algum pacote do cliente");
        }

        fwrite(buffer, 1,bytesRecebidos, handler); // Escreve no arquivo

        bzero(buffer, TAM_MAX);

        if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
            fclose(handler);
            return;
        }
    }
}

// Fecha a conexão com o servidor.

void close_connection(int socket){
    
    printf("\n Conexao encerrada\n");
    close(socket);
    
}


int main(int argc, char *argv[]){
    
    int socketCliente;
    int opcao = 1;
    int opcao_convertida;
    char* usuario;

    if (argc < 3) {
        printf("Por favor inserir o seu login e o valor do IP pelo qual deseja se conectar... \n");
        exit(0);
    }

    socketCliente = connect_server(argv[2],53000);

    send(socketCliente,argv[1],sizeof(argv[1]),0); // Envia o nome do usuario para o servidor

    recv(socketCliente,&opcao,sizeof(opcao),0); // Recebe o aval do servidor

    while (opcao != 0){
            
        imprimir_menu(argv[1]);
        
        scanf("%d", &opcao);
        opcao_convertida = htonl(opcao);
        
        
        send(socketCliente,&opcao_convertida,sizeof(opcao_convertida),0); // Informa o servidor qual a opção que ele vai realizar
        
        switch(opcao) {
            case 1: sync_client();
                break;
            case 2:
                send_file_cliente(socketCliente);
                break;
            case 3: get_file(socketCliente);
                break;
            case 0: close_connection(socketCliente);         
        }
        
    }
    
    return 0;
}
