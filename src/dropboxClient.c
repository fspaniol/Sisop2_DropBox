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

// Sincroniza o diretório "sync_dir_<nomeusuário>" com o servidor

void sync_client(){
    
}

// Envia um arquivo file para o servidor
// Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void send_file_cliente(char file[], int socket){
    
    puts("\n\n Entrei na função de enviar arquivos para o servidor");
    printf("Nome do arquivo: {%s}\n", file);

    FILE* handler; // Inteiro para a manipulação do arquivo que tentaremos abrir
    ssize_t bytesLidos = 0; // Estrutura para guardar a quantidade de bytes lidos pelo sistema
    ssize_t bytesEnviados = 0; // Estrutura para guardar a quantidade de bytes enviados para o servidor
    ssize_t tamanhoArquivoEnviado = 0;
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes para enviar
    int qtdePacotes = 0;
    int avalServidor = 0;

    if ((bytesEnviados = send(socket,"recebido.txt",sizeof("recebido.txt"),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
        puts("Deu erro ao enviar o nome do arquivo para o servidor");
        return;
    }

    bzero(buffer, TAM_MAX); // Limpa o buffer

    while(avalServidor == 0){
        recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(file, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("Erro ao abrir o arquivo"); // Nem precisa informar o servidor, creio eu
    }
    else{
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

char* get_file_name(){

    char data[50];
    char opcao;
    char *dat;
    int receiver;
    //fpurge(stdin);

    /// INICIO DA GAMBIARRA
    fflush(stdin);
    scanf("%c", &opcao);
    fflush(stdin);
    fflush(stdout);
    /// FIM DA GAMBIARRA

    receiver = getLine("Digite o nome do arquivo: ", data, sizeof(data));
    if (receiver == 0) {
        printf("Arquivo escolhido: [%s]\n", data);
        dat = &data[0];
        return dat;
    }
    printf("Deu ruim com o nome do arquivo... \n");

    return NULL;
}

// Obtém um arquivo file do servidor
// Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void get_file(char *file, int socket){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    bzero(buffer, TAM_MAX);

    //if ((send(socket,"teste.txt",sizeof("teste.txt"),0)) < 0) // Envia o nome do arquivo que deseja receber pro Servidor
    if ((send(socket,file,sizeof(file),0)) < 0) {// Envia o nome do arquivo que deseja receber pro Servidor
        //puts("Erro ao enviar o nome do arquivo...");
        printf("Erro ao enviar o nome do arquivo... %s", file);
        return;
    }

    char *receivedFile = "Received.txt";
    handler = fopen(receivedFile,"w"); // Abre o arquivo no qual vai armazenar as coisas, por enquanto hard-coded "clienteRecebeu.txt"

    while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
        if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
            puts("Erro tentando receber algum pacote do cliente");
            fclose(handler);
            return;
        }

        fwrite(buffer, 1, bytesRecebidos, handler); // Escreve no arquivo

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

    if (argc < 2) {
        printf("Por favor inserir o valor do IP pelo qual deseja se conectar... \n");
        exit(0);
    }

    socketCliente = connect_server(argv[1],50000);

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
                send_file_cliente(get_file_name(), socketCliente);
                //send_file_cliente("teste.txt", socketCliente);
                break;
            case 3: get_file(NULL, socketCliente);
                break;
            case 0: close_connection(socketCliente);
                
        }
        
    }
    
    return 0;
}
