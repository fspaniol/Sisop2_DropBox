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

///////////////////
#include <sys/types.h>
#include <dirent.h>
///////////////////

#include "../include/dropboxClient.h"
#include "dropboxUtil.c"
#include <sys/stat.h>

#define TAM_MAX 1024

#if defined(__linux)
#include <stdio_ext.h>
#endif

int semaforo = 0;
time_t ultimo_sync = 0;
time_t ultimo_sync_parcial = 0;

void *daemonMain(void *parametros){

    int *temp = (int *) parametros;
    int socketCliente = *temp;

    printf("\n [DAEMON] Daemon thread initialized \n");

    while (1){
        while (semaforo == 1){

        }
        semaforo = 1;
        printf("[DAEMON] Synchronizing the folder \n");
        sync_client(socketCliente);
        printf("[DAEMON] Synchronization done \n");
        semaforo = 0;
        sleep(10);

    }

    return 0;
}

void sync_client(int socketCliente){

    int opcao = 4;
    opcao = htonl(opcao);
    char clientFiles[TAM_MAX]; // buffer
    int x = 0;
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    time_t horario_servidor;
    time_t horario_cliente;
    struct stat *arquivo_cliente = malloc(sizeof(struct stat));

    send(socketCliente,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar

    bytesRecebidos = recv(socketCliente, clientFiles, sizeof(clientFiles), 0);

    if (strcmp("You have no files!",clientFiles) == 0)
        return;

    char *ch;
    ch = strtok(clientFiles, "\n");
    char buffer[TAM_MAX];
    bzero(buffer,TAM_MAX);

    while (ch != NULL) {
        opcao = 5;
        opcao = htonl(opcao);
        strcat(buffer,ch);
        send(socketCliente,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar
        send(socketCliente,buffer,sizeof(buffer),0); // Informa o servidor qual a opção que ele vai realizar
        bzero(buffer,TAM_MAX);

        if (lstat(ch, arquivo_cliente) != 0) {
            horario_cliente = 0;
        }
        else{
            horario_cliente = arquivo_cliente->st_mtime;
        }

        printf("[DAEMON] Synchronizing file: %s \n", ch);

        bytesRecebidos = recv(socketCliente,&horario_servidor,sizeof(horario_servidor),0);

        /*printf("Cliente: %zd \n", horario_cliente);
        printf("Servidor: %zd \n", horario_servidor);
        printf("ultimo_sync: %zd \n", ultimo_sync);*/

        if (horario_cliente > horario_servidor && horario_cliente > ultimo_sync){
            printf("[DAEMON] File: %s being sent to the server \n", ch);
            send_file_sync(socketCliente,ch);
        }
        else{
            if(horario_servidor > horario_cliente && horario_servidor > ultimo_sync){
                printf("[DAEMON] File: %s being received from the server \n", ch);
                get_file_sync(socketCliente,ch);
            }
            else
                printf("[DAEMON] File: %s does not need to be changed\n", ch);
        }
        usleep(10); // Da uma dormidinha minima de 10 milisegundo, para conseguir sincronizar as coisas*/
        ch = strtok(NULL, "\n");
    }
    ultimo_sync = ultimo_sync_parcial;
}

void send_file_sync(int socket, char* arquivo){
    
    //puts("\n[Client] entered 'Send file client' function");

    FILE* handler; // Inteiro para a manipulação do arquivo que tentaremos abrir
    ssize_t bytesLidos = 0; // Estrutura para guardar a quantidade de bytes lios pelo sistema
    ssize_t bytesEnviados = 0; // Estrutura para guardar a quantidade de bytes enviados para o servidor
    ssize_t tamanhoArquivoEnviado = 0;
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes para enviar
    int qtdePacotes = 0;
    int avalServidor = 0;
    int opcao = 6;
    opcao = htonl(opcao);
    struct stat *time_modified = malloc(sizeof(struct stat));
    ssize_t enviado;
    time_t time_dummy;

    strcpy(buffer,arquivo);

    send(socket,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar


    if ((bytesEnviados = send(socket,buffer,sizeof(buffer),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
        puts("[ERROR] An error has occured while sending file request to server.");
        return;
    }

    while(avalServidor == 0){
        recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(buffer, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("[ERROR] File not found."); // Nem precisa informar o servidor, creio eu PRECISA S
        char read = '\0';
        send(socket, &read, sizeof(read), 0);
        return;
    }

    else{
        bzero(buffer, TAM_MAX); // Limpa o buffer
        while ((bytesLidos = fread(buffer, 1, sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
                puts("[ERROR] Error while sending the file.");
                return;
            }
            bzero(buffer, TAM_MAX); // Limpa o buffer
            qtdePacotes++;
            tamanhoArquivoEnviado += bytesEnviados;
        }
        fclose(handler);
    }
    enviado = recv(socket,&time_dummy,sizeof(time_dummy),0);
    if (time_dummy > ultimo_sync_parcial)
        ultimo_sync_parcial = time_dummy;
}

void get_file_sync(int socket, char* arquivo){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    int opcao = 3;
    opcao = htonl(opcao);

    bzero(buffer, TAM_MAX);
    strcat(buffer,arquivo);

    struct stat *time_modified = malloc(sizeof(struct stat));

    send(socket,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar

    if ((send(socket,buffer,sizeof(buffer),0)) < 0){ // Envia o nome do arquivo que deseja receber pro Servidor
        puts("[ERROR] Error while sending the filename...");
        exit(1);
    }
    handler = fopen(buffer,"w"); // Abre o arquivo no qual vai armazenar as coisas

        bzero(buffer, TAM_MAX);

        while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
            if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
                puts("[ERROR] Error when receiving client's packages.");
            }

            fwrite(buffer, 1,bytesRecebidos, handler); // Escreve no arquivo

            bzero(buffer, TAM_MAX);

            if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
                fclose(handler);
                if (lstat(arquivo, time_modified) != 0) {
                    printf("[ERROR] Error trying to get the time when changed");
                 }
                else{
                    if (time_modified->st_mtime > ultimo_sync_parcial)
                        ultimo_sync_parcial = time_modified->st_mtime;
                }
                return;
            }
        }
}

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

    #if defined(__linux)
    __fpurge(stdin);
    
    #elif(__APPLE__)
    fpurge(stdin);
    
    #endif

    printf("\n %s ", mensagem);
    fgets(buffer,50,stdin);
    strtok(buffer, "\n");
    printf("[Client] Requested information: %s\n", buffer);

    #if defined(__linux)
    __fpurge(stdin);
    
    #elif(__APPLE__)
    fpurge(stdin);
    
    #endif

}

// Envia um arquivo file para o servidor
// Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void send_file_cliente(int socket){
    
    //puts("\n[Client] entered 'Send file client' function");

    FILE* handler; // Inteiro para a manipulação do arquivo que tentaremos abrir
    ssize_t bytesLidos = 0; // Estrutura para guardar a quantidade de bytes lios pelo sistema
    ssize_t bytesEnviados = 0; // Estrutura para guardar a quantidade de bytes enviados para o servidor
    ssize_t tamanhoArquivoEnviado = 0;
    char buffer[TAM_MAX]; // Buffer que armazena os pacotes para enviar
    int qtdePacotes = 0;
    int avalServidor = 0;

    printf("[Client] Please enter the name of the file:");
    get_info(buffer,">> ");


    if ((bytesEnviados = send(socket,buffer,sizeof(buffer),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
        puts("[ERROR] An error has occured while sending file request to server.");
        return;
    }

    while(avalServidor == 0){
        recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(buffer, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("[ERROR] File not found."); // Nem precisa informar o servidor, creio eu PRECISA S
        char read = '\0';
        send(socket, &read, sizeof(read), 0);
        return;
    }

    else{
        bzero(buffer, TAM_MAX); // Limpa o buffer
        while ((bytesLidos = fread(buffer, 1, sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            printf("\n Bytes read: %zd \n", bytesLidos);
            if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
                puts("[ERROR] Error while sending the file.");
                return;
            }
            bzero(buffer, TAM_MAX); // Limpa o buffer
            qtdePacotes++;
            tamanhoArquivoEnviado += bytesEnviados;
        }
        fclose(handler);
    }

    printf("[Client] There were sent %zd bytes in %d packages of %d of size.\n", tamanhoArquivoEnviado, qtdePacotes, TAM_MAX);
}

// Obtém um arquivo file do servidor
// Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void get_file(int socket){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    bzero(buffer, TAM_MAX);

    get_info(buffer,"[Client] Please, inform the desired file to download:\n>> ");

    if ((send(socket,buffer,sizeof(buffer),0)) < 0){ // Envia o nome do arquivo que deseja receber pro Servidor
        puts("[ERROR] Error while sending the filename...");
        exit(1);
    }
    if( access( buffer, F_OK ) != -1 ) {
        handler = fopen(buffer,"w"); // Abre o arquivo no qual vai armazenar as coisas, por enquanto hard-coded "clienteRecebeu.txt"

        bzero(buffer, TAM_MAX);

        while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
            if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
                puts("[ERROR] Error when receiving client's packages.");
            }

            fwrite(buffer, 1,bytesRecebidos, handler); // Escreve no arquivo

            bzero(buffer, TAM_MAX);

            if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
                fclose(handler);
                return;
            }
        }
    }else{
        puts("[ERROR] File does not exist");
    }
}

void list_files(int socket) {

    char clientFiles[TAM_MAX]; // buffer
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    while ((bytesRecebidos = recv(socket, clientFiles, sizeof(clientFiles), 0)) > 0){
            if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
                puts("[ERROR ] Error when receiving client's directory information.");
            }

            printf("[CLIENT] Your files:\n");
            puts(clientFiles);

            bzero(clientFiles, TAM_MAX);
            return;
        }
}

// Fecha a conexão com o servidor.

void close_connection(int socket){
    
    printf("\n[Client] Connection terminated.\n");
    close(socket);
    
}

int main(int argc, char *argv[]){
    
    int socketCliente;
    int opcao = 1;
    int opcao_convertida;
    char* usuario;
    pthread_t daemon;

    if (argc < 3) {
        printf("[Client] Please, insert your login and the desired IP to connect...\n");
        exit(0);
    } 

    socketCliente = connect_server(argv[2],53000);


    send(socketCliente,argv[1],sizeof(argv[1]),0); // Envia o nome do usuario para o servidor

    recv(socketCliente,&opcao,sizeof(opcao),0); // Recebe o aval do servidor

    if (opcao == 0){
        puts("Este usuario ja possui 2 conexoes abertas, desconectando...");
        return 0;
    }

    if (pthread_create(&daemon,NULL,daemonMain, &socketCliente)){
        puts("[ERROR] Error trying to create a Daemon thread ");
        return 0;
    }

    while (opcao != 0){

        while (semaforo == 1){
            
        }

        imprimir_menu(argv[1]);
        
        scanf("%d", &opcao);
        opcao_convertida = htonl(opcao);


        semaforo = 1;
        
        send(socketCliente,&opcao_convertida,sizeof(opcao_convertida),0); // Informa o servidor qual a opção que ele vai realizar
        
        switch(opcao) {
            case 1: 
                sync_client(socketCliente);
                break;
            case 2:
                send_file_cliente(socketCliente);
                break;
            case 3: 
                get_file(socketCliente);
                break;
            case 4:
                list_files(socketCliente);
                break;
            case 0: 
                close_connection(socketCliente);         
        }

        semaforo = 0;
        
    }
    
    return 0;
}
