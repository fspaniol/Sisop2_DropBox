//
//  dropboxClient.c
//
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <arpa/inet.h> 

#include "../include/dropboxClient.h"
#include "dropboxUtil.c"

#define TAM_MAX 1024

#if defined(__linux)
#include <stdio_ext.h>
#endif



int semaforo = 0;
time_t ultimo_sync = 0;
time_t ultimo_sync_parcial = 0;
time_t diferenca;
const SSL_METHOD *method;
SSL_CTX *ctx;
SSL *ssl;

void *daemonMain(void *parametros){
    printf("\n [DAEMON] Daemon thread initialized \n");

    while (1){
        while (semaforo == 1){

        }
        semaforo = 1;
        printf("[DAEMON] Synchronizing the folder \n");
        getTimeServer(ssl);
        sync_client(ssl);
        printf("[DAEMON] Synchronization done \n");
        semaforo = 0;
        sleep(10);

    }

    return 0;
}

void getTimeServer(SSL *socketCliente){
    time_t horario_envio;
    time_t horario_recebimento;
    time_t horario_servidor;

    int opcao = 7;
    opcao = htonl(opcao);
    time(&horario_envio);
    //send(socketCliente,&opcao,sizeof(opcao),0);
    SSL_write(socketCliente,&opcao,sizeof(opcao));
    //recv(socketCliente,&horario_servidor,sizeof(horario_servidor),0);
    SSL_read(socketCliente,&horario_servidor,sizeof(horario_servidor));
    time(&horario_recebimento);

    algoritmo_cristian(horario_envio,horario_recebimento,horario_servidor);

    /*printf("Horario de envio: %zd \n", horario_envio);
    printf("Horario de recebimento: %zd \n", horario_recebimento);
    printf("Horario de servidor: %zd \n", horario_servidor);*/
}

void algoritmo_cristian(time_t horario_envio, time_t horario_recebimento, time_t horario_servidor){
    time_t tempo_processamento;

    tempo_processamento = horario_recebimento - horario_envio;
    tempo_processamento = tempo_processamento / 2;
    tempo_processamento += horario_envio; // A hora que era no cliente quando o servidor estava pegando seu horario local

    diferenca = horario_servidor - tempo_processamento; // Atualiza a diferenca entre os dois relogios
}


void sync_client(SSL *socketCliente){

    int opcao = 4;
    opcao = htonl(opcao);
    char clientFiles[TAM_MAX]; // buffer
    int x = 0;
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    time_t horario_servidor;
    time_t horario_cliente;
    struct stat *arquivo_cliente = malloc(sizeof(struct stat));

    //send(socketCliente,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar
    SSL_write(socketCliente,&opcao,sizeof(opcao));

    //bytesRecebidos = recv(socketCliente, clientFiles, sizeof(clientFiles), 0);
    bytesRecebidos = SSL_read(socketCliente, clientFiles, sizeof(clientFiles));

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
        //send(socketCliente,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar
        //send(socketCliente,buffer,sizeof(buffer),0); // Informa o servidor qual a opção que ele vai realizar
        SSL_write(socketCliente,&opcao,sizeof(opcao)); // Informa o servidor qual a opção que ele vai realizar
        SSL_write(socketCliente,buffer,sizeof(buffer)); // Informa o servidor qual a opção que ele vai realizar
        bzero(buffer,TAM_MAX);

        if (lstat(ch, arquivo_cliente) != 0) {
            horario_cliente = 0;
        }
        else{
            horario_cliente = arquivo_cliente->st_mtime;
        }

        printf("[DAEMON] Synchronizing file: %s \n", ch);

        //bytesRecebidos = recv(socketCliente,&horario_servidor,sizeof(horario_servidor),0);
        bytesRecebidos = SSL_read(socketCliente,&horario_servidor,sizeof(horario_servidor));

/*        printf("Cliente: %zd \n", horario_cliente);
        printf("Servidor: %zd \n", horario_servidor);
        printf("ultimo_sync: %zd \n", ultimo_sync);
        printf ( "Diferenca entre cliente e servidor: %zd \n", diferenca);*/


        if ((horario_cliente + diferenca) > horario_servidor && horario_cliente > ultimo_sync){
            printf("[DAEMON] File: %s being sent to the server \n", ch);
            send_file_sync(socketCliente,ch);
        }
        else{
            if(horario_servidor > (horario_cliente + diferenca) && horario_servidor > ultimo_sync){
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

void send_file_sync(SSL *socket, char* arquivo){
    
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

    //send(socket,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar
    SSL_write(socket,&opcao,sizeof(opcao));

    //if ((bytesEnviados = send(socket,buffer,sizeof(buffer),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
    if ((bytesEnviados = SSL_write(socket,buffer,sizeof(buffer))) < 0) {  
        puts("[ERROR] An error has occured while sending file request to server.");
        return;
    }

    while(avalServidor == 0){
        SSL_read(socket,&avalServidor,sizeof(avalServidor));
        //recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(buffer, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("[ERROR] File not found."); // Nem precisa informar o servidor, creio eu PRECISA S
        char read = '\0';
        //send(socket, &read, sizeof(read), 0);
        SSL_write(socket,&read,sizeof(read));
        return;
    }

    else{
        bzero(buffer, TAM_MAX); // Limpa o buffer
        while ((bytesLidos = fread(buffer, 1, sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            //if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
            if ((bytesEnviados = SSL_write(socket,buffer,bytesLidos)) < bytesLidos) {
                puts("[ERROR] Error while sending the file.");
                return;
            }
            bzero(buffer, TAM_MAX); // Limpa o buffer
            qtdePacotes++;
            tamanhoArquivoEnviado += bytesEnviados;
        }
        fclose(handler);
    }
    //enviado = recv(socket,&time_dummy,sizeof(time_dummy),0);
    enviado = SSL_read(socket,&time_dummy,sizeof(time_dummy));
    if (time_dummy > ultimo_sync_parcial)
        ultimo_sync_parcial = time_dummy;
}

void get_file_sync(SSL *socket, char* arquivo){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    int opcao = 3;
    opcao = htonl(opcao);

    bzero(buffer, TAM_MAX);
    strcat(buffer,arquivo);

    struct stat *time_modified = malloc(sizeof(struct stat));

    //send(socket,&opcao,sizeof(opcao),0); // Informa o servidor qual a opção que ele vai realizar
    SSL_write(socket,&opcao,sizeof(opcao));

    //if ((send(socket,buffer,sizeof(buffer),0)) < 0){ // Envia o nome do arquivo que deseja receber pro Servidor
    if ((SSL_write(socket,buffer,sizeof(buffer))) < 0){
        puts("[ERROR] Error while sending the filename...");
        exit(1);
    }
    handler = fopen(buffer,"w"); // Abre o arquivo no qual vai armazenar as coisas

        bzero(buffer, TAM_MAX);

        //while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
        while ((bytesRecebidos = SSL_read(socket, buffer, sizeof(buffer))) > 0){   
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

void send_file_cliente(SSL *socket){
    
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


    //if ((bytesEnviados = send(socket,buffer,sizeof(buffer),0)) < 0) { // Envia o nome do arquivo que ira ser mandado para o servidor, por enquanto hardcoded "recebido.txt"
    if ((bytesEnviados = SSL_write(socket,buffer,sizeof(buffer))) < 0) {
        puts("[ERROR] An error has occured while sending file request to server.");
        return;
    }

    while(avalServidor == 0){
        SSL_read(socket,&avalServidor,sizeof(avalServidor));
        //recv(socket,&avalServidor,sizeof(avalServidor),0); // Recebe a flag do servidor indicando que ja pode começar a enviar o arquivo
    }

    if ((handler = fopen(buffer, "r")) == NULL){ // Se f for menor que 0, quer dizer que o sistema não conseguiu abrir o arquivo
        puts("[ERROR] File not found."); // Nem precisa informar o servidor, creio eu PRECISA S
        char read = '\0';
        //send(socket, &read, sizeof(read), 0);
        SSL_write(socket,&read,sizeof(read));
        return;
    }

    else{
        bzero(buffer, TAM_MAX); // Limpa o buffer
        while ((bytesLidos = fread(buffer, 1, sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
            printf("\n Bytes read: %zd \n", bytesLidos);
            //if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
            if ((bytesEnviados = SSL_write(socket,buffer,bytesLidos)) < bytesLidos) {
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

void get_file(SSL *socket){
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    bzero(buffer, TAM_MAX);

    get_info(buffer,"[Client] Please, inform the desired file to download:\n>> ");

    //if ((send(socket,buffer,sizeof(buffer),0)) < 0){ // Envia o nome do arquivo que deseja receber pro Servidor
    if ((SSL_write(socket,buffer,sizeof(buffer))) < 0){
        puts("[ERROR] Error while sending the filename...");
        exit(1);
    }
    if( access( buffer, F_OK ) != -1 ) {
        handler = fopen(buffer,"w"); // Abre o arquivo no qual vai armazenar as coisas, por enquanto hard-coded "clienteRecebeu.txt"

        bzero(buffer, TAM_MAX);

        //while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
        while ((bytesRecebidos = SSL_read(socket, buffer, sizeof(buffer))) > 0){    
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

void list_files(SSL *socket) {

    char clientFiles[TAM_MAX]; // buffer
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    //while ((bytesRecebidos = recv(socket, clientFiles, sizeof(clientFiles), 0)) > 0){
    while ((bytesRecebidos = SSL_read(socket, clientFiles, sizeof(clientFiles))) > 0){
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

void initializeSSL(){
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

// Replica Manager - Cliente recebe a lista de IPs do server, caso não tenha uma
int receiveServerList(int socket) {
    char buffer[TAM_MAX]; // buffer
    FILE* handler; // handler do arquivo
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem

    bzero(buffer, TAM_MAX);

    if ((send(socket,buffer,sizeof(buffer),0)) < 0){ // Envia o nome do arquivo que deseja receber pro Servidor
        puts("[ERROR] Error while sending the filename...");
        exit(1);
    }
    if( access( buffer, F_OK ) != -1 ) {
        handler = fopen("RMFile.txt","w"); // Abre o arquivo no qual vai armazenar as coisas, por enquanto hard-coded "clienteRecebeu.txt"

        bzero(buffer, TAM_MAX);

        while ((bytesRecebidos = recv(socket, buffer, sizeof(buffer), 0)) > 0){
            if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
                puts("[ERROR] Error when receiving server list.");
            }

            fwrite(buffer, 1,bytesRecebidos, handler); // Escreve no arquivo

            bzero(buffer, TAM_MAX);

            if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
                fclose(handler);
                return 0;
            }
        }
        return -1;
    }else{
        puts("[ERROR] File not found.");
        return -1;
    }
}

int main(int argc, char *argv[]){
    
    int socketCliente;
    int opcao = 1;
    int opcao_convertida;
    char* usuario;
    pthread_t daemon;

    initializeSSL();
    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);
    if (ctx == NULL){
        ERR_print_errors_fp(stderr);
        abort();
    } 
	

    if (argc < 3) {
        printf("[Client] Please, insert your login and the desired IP to connect...\n");
        exit(0);
    }
    socketCliente = connect_server(argv[2],53000);


    ssl = SSL_new(ctx);
    SSL_set_fd(ssl,socketCliente);
    if(SSL_connect(ssl) == -1)
        ERR_print_errors_fp(stderr);
    else{
        X509 *cert;
        char *line;
        cert = SSL_get_peer_certificate(ssl);
	if(cert != NULL){
	     line = X509_NAME_oneline(X509_get_subject_name(cert),0,0);
	     printf("The subject of the certificate is: %s \n",line);
	     free(line);
	     line = X509_NAME_oneline(X509_get_issuer_name(cert),0,0);
	     printf("The issuer of the certificate is: %s \n",line);
	     free(line);
	}
    }

    SSL_write(ssl,argv[1],sizeof(argv[1]));
    //send(socketCliente,argv[1],sizeof(argv[1]),0); // Envia o nome do usuario para o servidor

    SSL_read(ssl,&opcao,sizeof(opcao));

    //recv(socketCliente,&opcao,sizeof(opcao),0); // Recebe o aval do servidor

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
        
        //send(socketCliente,&opcao_convertida,sizeof(opcao_convertida),0); // Informa o servidor qual a opção que ele vai realizar
        SSL_write(ssl,&opcao_convertida,sizeof(opcao_convertida));
        switch(opcao) {
            case 1: 
                sync_client(ssl);
                break;
            case 2:
                send_file_cliente(ssl);
                break;
            case 3: 
                get_file(ssl);
                break;
            case 4:
                list_files(ssl);
                break;
            case 0: 
                close_connection(socketCliente);         
        }

        semaforo = 0;
        
    }
    
    return 0;
}
