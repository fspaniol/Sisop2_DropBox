//
//  dropboxServer.c
//
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <time.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <arpa/inet.h> 


#include "../include/dropboxServer.h"
#include "dropboxUtil.c"

#define TAM_MAX 1024

struct Client clientes[10];
struct Replica replicas[10];
int semaforo = 0;
const SSL_METHOD *method;
SSL_CTX *ctx;
SSL *ssl;
//Cria o socket do servidor
int isPrimaryServer = -1;
char primaryIP[16];
char serversIPs[16][16];

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

int conta_conexoes_usuario(char *usuario){
    int x, cont = 0;

    while (semaforo == 1){

    }

    semaforo = 1;

    for (x = 0; x < 10; x++){
        if (strcmp(usuario,clientes[x].userid) == 0 && clientes[x].logged_in == 1)
            cont++;
    }

    printf("[Server][User: %s] The client has %d connections... \n", usuario, cont);

    semaforo = 0;

    return cont;
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
    int conexoes;

    int opcao_recebida = 1;

    char usuario[TAM_MAX];
    int flag = 1;
    flag = htonl(flag);

    //recv(clientes[index].devices[0],  clientes[index].userid, sizeof(usuario), 0); // Recebe o numero do usuario
    SSL_read(clientes[index].socket,clientes[index].userid, sizeof(usuario));

    conexoes = conta_conexoes_usuario(clientes[index].userid);

    if (conexoes > 2){
        printf("[Server][User: %s] The user already has 2 connections open, closing this one... \n", clientes[index].userid);
        flag = 0;
        flag = htonl(flag);
        //send(clientes[index].devices[0], &flag, sizeof(flag), 0); // Envia o aval dizendo que ja recebeu
        SSL_write(clientes[index].socket, &flag, sizeof(flag));
        clientes[index].logged_in = 0;
        return 0;
    }
    cria_pasta_usuario(clientes[index].userid);
    //send(clientes[index].devices[0], &flag, sizeof(flag), 0); // Envia o aval dizendo que ja recebeu
    SSL_write(clientes[index].socket,&flag,sizeof(flag));
        
    while (opcao_recebida != 0){ // enquanto a opção do cliente não for sair da conexao, ele fica atendendo esse cliente
        opcao_recebida = 8;
        //puts("Estou esperando acao de algum cliente... \n");
            
        //recv(clientes[index].devices[0], &opcao_recebida, sizeof(opcao_recebida), 0); // recebe do usuario que opção ele quer
        SSL_read(clientes[index].socket,&opcao_recebida,sizeof(opcao_recebida));
        opcao_recebida = htonl(opcao_recebida);
            
        switch(opcao_recebida) {
            case 1: sync_server();
                break;
            case 2: receive_file(clientes[index].socket, clientes[index].userid);
                break;
            case 3: send_file_servidor(clientes[index].socket, clientes[index].userid);
                break;
            case 4: list_files_server(clientes[index].socket, clientes[index].userid);
                break;
            case 5: send_time_modified(clientes[index].socket, clientes[index].userid);
                break;
            case 6: receive_file_sync(clientes[index].socket, clientes[index].userid);
                break;
            case 7: send_time(clientes[index].socket, clientes[index].userid);
                break;
            case 8: send_ServerList(clientes[index].socket, clientes[index].userid);
            	break;
            case 0: printf("[Server][User: %s] Client %d disconnected.\n", clientes[index].userid, index);
        }          
    }

    clientes[index].logged_in = 0; 

    return 0;
}

// Sincroniza o servidor com o diretório "sync_dir_<nomeusuário>" com o cliente
void sync_server() {
}

void receive_file(SSL *socket, char* usuario){
// Recebe um arquivo file do cliente. Deverá ser executada quando for realizar upload de um arquivo. file - path/filename.ext do arquivo a ser recebido
// void receive_file(int socket, char* usuario){
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

    //while((bytesRecebidos = recv(socket, buffer, sizeof(buffer),0)) < 0){ // recebe o nome do arquivo que vai receber do cliente
    while((bytesRecebidos = SSL_read(socket, buffer, sizeof(buffer))) < 0){
    }

    printf("[Server][User: %s] The file to be sent by client is: %s\n", usuario, buffer); // Escreve o nome do arquivo
    
    //if ((bytesEnviados = send(socket, &flag, sizeof(flag), 0)) < 0){
    if ((bytesEnviados = SSL_write(socket, &flag, sizeof(flag))) < 0){
        printf("[ERROR ][User: %s] Error sending acknowledgement to client for file receiving.", usuario); // Envia uma flag dizendo pro cliente que ta tudo pronto e a transferencia do conteudo do arquivo pode começar
        return;
    }

    strcat(diretorio,buffer);

    bytesRecebidos = 0; // Reseta o numero de bytes lidos

    handler = fopen(diretorio, "w"); // Abre o arquivo 

    bzero(buffer, TAM_MAX); // Reseta o buffer

    //while ((bytesRecebidos = recv(socket, buffer, TAM_MAX, 0)) > 0){  // Enquanto tiver coisa sendo lida, continua lendo
    while ((bytesRecebidos = SSL_read(socket, buffer, TAM_MAX)) > 0){
    	if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
       		printf("[ERROR ][User: %s] Error when trying to receive client package.\n", usuario);
            fclose(handler);
            return;
    	}
        if (buffer[0] == '\0'){
            printf("[Server][User: %s] Client could not send file.\n", usuario);
            fclose(handler);

            fseek (handler, 0, SEEK_END); //verifica qual o tamanho do arquivo
            int size = ftell(handler);
            if (-1 == size) {   
                remove(diretorio); // se o arquivo nao existir, ele nao vai ser criado
            }   
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

void receive_file_sync(SSL *socket, char* usuario){
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
    struct stat *time_modified = malloc(sizeof(struct stat));
    time_t horario_modificado;

    //while((bytesRecebidos = recv(socket, buffer, sizeof(buffer),0)) < 0){ // recebe o nome do arquivo que vai receber do cliente
    while((bytesRecebidos = SSL_read(socket, buffer, sizeof(buffer))) < 0){
    }

    printf("[Server][User: %s] The file to be sent by client is: %s\n", usuario, buffer); // Escreve o nome do arquivo
    
    //if ((bytesEnviados = send(socket, &flag, sizeof(flag), 0)) < 0){
    if ((bytesEnviados = SSL_write(socket, &flag, sizeof(flag))) < 0){
        printf("[ERROR ][User: %s] Error sending acknowledgement to client for file receiving.", usuario); // Envia uma flag dizendo pro cliente que ta tudo pronto e a transferencia do conteudo do arquivo pode começar
        return;
    }

    strcat(diretorio,buffer);

    bytesRecebidos = 0; // Reseta o numero de bytes lidos

    handler = fopen(diretorio, "w"); // Abre o arquivo 

    bzero(buffer, TAM_MAX); // Reseta o buffer

    //while ((bytesRecebidos = recv(socket, buffer, TAM_MAX, 0)) > 0){  // Enquanto tiver coisa sendo lida, continua lendo
    while ((bytesRecebidos = SSL_read(socket, buffer, TAM_MAX)) > 0){
        if (bytesRecebidos < 0) { // Se a quantidade de bytes recebidos for menor que 0, deu erro
            printf("[ERROR ][User: %s] Error when trying to receive client package.\n", usuario);
            fclose(handler);
            return;
        }
        if (buffer[0] == '\0'){
            printf("[Server][User: %s] Client could not send file.\n", usuario);
            fclose(handler);

            fseek (handler, 0, SEEK_END); //verifica qual o tamanho do arquivo
            int size = ftell(handler);
            if (-1 == size) {   
                remove(diretorio); // se o arquivo nao existir, ele nao vai ser criado
            }   
            return; 
        }

        fwrite(buffer, 1, bytesRecebidos, handler); // Escreve no arquivo

        bzero(buffer, TAM_MAX); // Reseta o buffer

        if(bytesRecebidos < TAM_MAX){ // Se o pacote que veio, for menor que o tamanho total, eh porque o arquivo acabou
            fclose(handler);
            printf("[Server][User: %s] Successfully received client file.\n", usuario);

            lstat(diretorio,time_modified);
            horario_modificado = time_modified->st_mtime;

            //bytesRecebidos = send(socket,&horario_modificado,sizeof(horario_modificado),0);
            bytesRecebidos = SSL_write(socket,&horario_modificado,sizeof(horario_modificado));
            return;
        }
    }

    if (lstat(diretorio, time_modified) != 0) {
        printf("[SERVER][USER: %s] Error sending the date of the file \n", usuario);
        return;
    }
}

// Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void send_file_servidor(SSL *socket, char* usuario){
// void send_file_servidor(int socket, char* usuario){

    char diretorio[200] = "sync_dir_";
    strcat(diretorio,usuario);
    strcat(diretorio,"/");

    printf("[Server][User: %s] Server will receive the desired filename... \n", usuario);

    char buffer[TAM_MAX];
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesLidos;
    ssize_t bytesEnviados;
    FILE* handler;

    bzero(buffer,TAM_MAX);

    //bytesRecebidos = recv(socket, buffer, TAM_MAX, 0); // recebe o nome do arquivo que o cliente quer receber
    bytesRecebidos = SSL_read(socket, buffer, TAM_MAX);
    if (bytesRecebidos < 0)
        printf("[ERROR ][[User: %s]] Error receiving the filename to be sent. \n", usuario);
    else
        printf("[Server][User: %s] The requested file by the client is: %s\n", usuario, buffer); // Escreve o nome do arquivo que o cliente quer

    strcat(diretorio,buffer);

    //handler = fopen(diretorio,"r");
    if ((handler = fopen(diretorio, "r")) == NULL) {
        printf("[ERROR ][User: %s] Error sending the file. \n", usuario);
        return;
    }

    while ((bytesLidos = fread(buffer, 1,sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
        //if ((bytesEnviados = send(socket,buffer,bytesLidos,0)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
        if ((bytesEnviados = SSL_write(socket,buffer,bytesLidos)) < bytesLidos) {
            printf("[ERROR ][User: %s] Error sending the file.", usuario);
            return;
        }
        bzero(buffer, TAM_MAX); // Reseta o buffer
    }

    fclose(handler);
}

void send_time(SSL *socket, char* usuario){
    time_t horario;

    time(&horario);
    //send(socket,&horario,sizeof(horario),0);
    SSL_write(socket,&horario,sizeof(horario));

    printf("[SERVER][User: %s] Enviado o atual horario. \n", usuario);
}

void send_ServerList(SSL *socket, char* usuario) {

    printf("[Server][User: %s] Server will send the server list... \n", usuario);

    char buffer[TAM_MAX];
    ssize_t bytesRecebidos; // Quantidade de bytes que foram recebidos numa passagem
    ssize_t bytesLidos;
    ssize_t bytesEnviados;
    FILE* handler;

    bzero(buffer,TAM_MAX);

    if ((handler = fopen("RMFile.txt", "r")) == NULL) {
        printf("[ERROR ][User: %s] Error retrieving the server list. \n", usuario);
        return;
    }

    while ((bytesLidos = fread(buffer, 1,sizeof(buffer), handler)) > 0){ // Enquanto o sistema ainda estiver lendo bytes, o arquivo nao terminou
        if ((bytesEnviados = SSL_write(socket, buffer,bytesLidos)) < bytesLidos) { // Se a quantidade de bytes enviados, não for igual a que a gente leu, erro
            printf("[ERROR ][User: %s] Error sending the server list.", usuario);
            return;
        }
        bzero(buffer, TAM_MAX); // Reseta o buffer
    }

    fclose(handler);
    printf("[Server][User: %s] Successfully sent the server list.\n", usuario);
}


void send_time_modified(SSL *socket, char* usuario){
    char buffer[TAM_MAX];
    struct stat *horario = malloc(sizeof(struct stat));
    char diretorio[200] = "sync_dir_";
    strcat(diretorio,usuario);
    strcat(diretorio,"/");
    ssize_t bytesRecebidos;
    time_t horario_modificado;

    //bytesRecebidos = recv(socket, buffer, TAM_MAX, 0);
    bytesRecebidos = SSL_read(socket,buffer,TAM_MAX);

    strcat(diretorio,buffer);

    if (lstat(diretorio, horario) != 0) {
        printf("[SERVER][USER: %s] Error sending the date of the file \n", usuario);
        return;
    }
    
    horario_modificado = horario->st_mtime;

    //bytesRecebidos = send(socket,&horario_modificado,sizeof(horario_modificado),0);
    bytesRecebidos = SSL_write(socket,&horario_modificado,sizeof(horario_modificado));

}

void list_files_server(SSL *socket, char* usuario) {
// void list_files_server(int socket, char* usuario) {
    
    DIR *dir;
    struct dirent *ent;
    ssize_t bytesEnviados;

    char userDir[100] = "sync_dir_";
    strcat(userDir, usuario);
    strcat(userDir, "/");

    char userFiles[TAM_MAX] = "";
    
    if ((dir = opendir (userDir)) != NULL) {
        printf("[SERVER] Reading client's directory...\n");

        while ((ent = readdir (dir)) != NULL) {

            if((strcmp(ent->d_name,".") != 0 && strcmp(ent->d_name,"..") != 0)) {
                strcat(userFiles, ent->d_name);
                strcat(userFiles, "\n");
            }
        }
        
        if (strcmp(userFiles, "") == 0) {
            strcat(userFiles, "You have no files!");
        }
        strcat(userFiles, "\0");
        
        closedir (dir);

        //if ((bytesEnviados = send(socket, userFiles, TAM_MAX, 0)) < 0) {
        if ((bytesEnviados = SSL_write(socket, userFiles,TAM_MAX)) < 0) {
            printf("[ERROR ][User: %s] Error sending files list.", usuario);
            return;
        }

    } else {
      /* could not open directory */
        printf("[ERROR ][User: %s] Server could not find user's directory.\n", usuario);
      // perror ("");
      // return EXIT_FAILURE;
    }
    printf("[SERVER][User: %s] Done! Client's directory successfully read and sent.\n", usuario);
}

// Propaga alteracoes para as outras replicas.
// Retorno:
// 	0 se todas as replicas aceitaram as alteracoes normalmente
// 	1 se alguma replica nao pode aceitar as alteracoes
int updateReplicas(int funcao, struct Client cliente){
	int numberOfReplicas = 10; //temporario
	int i = 0;
	int timeout = 500; //temporario

	for (i = 0; i < numberOfReplicas; i++) {
		switch (funcao) {
			case 1: //criaPastaUsuario
				
				if (timeout == 0)
					return 1;
				break;

			case 2: //sendFile (cliente -> server -> replicas)
				if (timeout == 0)
					return 1;
				break;

			case 3: //sync
				if (timeout == 0)
					return 1;
				break;

			default:
				break;
		}
	}

	//tudo certo ate aqui!
    	return 0;
}

// Inicializa as bibliotecas de SSL
void initializeSSL(){
	SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

void *listenForReplicas() {
	int socketListener, cont = 0;
	struct sockaddr_storage depositoServidor[10];
	char recvbuffer[6000];
	// bzero(recvbuffer, 6000);
	ssize_t bytesRecebidos = 0;
	int connected = 0;
    socklen_t tamanhoEndereco[10];
	socketListener = criaSocketServidor(primaryIP, 53001);
	printf("[Listnr] Listener initialized. Server is now listening for incoming replica connections...\n");
	
	while (1) {
		
		if ((listen(socketListener,10)) != 0){
	            printf("[ERROR] Listener is deaf. Something went wrong.\n");
	    }

	    tamanhoEndereco[cont] = sizeof(depositoServidor[cont]);
        replicas[cont].binded = accept(socketListener, (struct sockaddr *) &depositoServidor[cont], &tamanhoEndereco[cont]);
        // printf("[Listnr] Connection incoming: %s\n", );

	    send(socketListener, &isPrimaryServer, sizeof(isPrimaryServer), 0); // Avisa os requerintes de que este é o primary server
	    printf("sent %d to socket.\n", isPrimaryServer);

	    if (recv(socketListener, recvbuffer, 6000, 0) < 0) {
	    	printf("[Listnr] Receive failed. (%s)\n", recvbuffer);
	    	printf("error: %zd\n", recv(socketListener, recvbuffer, 6000, 0));
	    	return 0;
	    }

        if (recvbuffer[0] == '\0'){
            printf("[Listnr] Could not receive Replica IP: %s\n", recvbuffer);
            return 0; 
        }

    	char *buf = malloc(16);
    	strcpy(buf, recvbuffer);

 		printf("[Listnr] Replica %s connected to the primary server. [%d]\n", buf, cont);

		cont++;
	}
	return 0;
}

// PRIMARY SERVER FUNCTIONS //

void initializePrimary(int argc, char *argv[], int sckt) {
	// Primary tem que verificar se existe um RMFile.txt e, caso não exista, criar um
	pthread_t primaryListener;
	FILE *handler;
	char *buffer;
	char iplist[1024];
	strcpy(iplist, argv[1]);
	strcat(iplist, "\n");
	strcpy(primaryIP, argv[1]);
	int status = 0;

	printf("[Server] Will look for IP list... \n");
	handler = fopen("RMFile.txt","r");

	if (handler != NULL) {
		printf("[Server] Found RMFile. Will read: \n");
		buffer = readRMFile();
		printf("%s\n", buffer);
		
		status = isAddressInFile(primaryIP);
		if (status == 0) {
			printf("[Server] Found Primary IP in file. All good.\n");
		}
		
		if (status == 1) {
			printf("[Server] Primary IP not found in file. Writing... ");
			if (addAddressRMFile(primaryIP) == 0) {
				printf("Done.\n");
			} else {
				printf("Error.\n");
			}
		}

	} else {
		printf("[Server] Could not find RMFile. Creating a new one... \n");
		if (createRMFile(iplist) == 0) {
			printf("[Server] Successfully created a RMFile.\n");
		} else {
			printf("[Server] Could not create a RMFile.\n");
		}
	}

	fclose(handler);

	// Cria uma segunda thread que roda em paralelo e se comunica com os replica-managers
	if (pthread_create(&primaryListener, NULL, listenForReplicas, 0)){
    	puts("[ERROR] Could not create replica listener thread.");
    	puts("[Server] Terminating...");
    	return;
	}
	primaryLoop(sckt);
}

void primaryLoop(int socketServidor) {
	// O servidor fica rodando para sempre e quando algum cliente aparece chama a função send_file para mandar algo
    // O segundo parametro do listen diz quantas conexões podemos ter
    socklen_t tamanhoEndereco[10];
    struct sockaddr_storage depositoServidor[10];
    int socketCliente;
    pthread_t threads[10];
	int cont = 0;
    int cont2;

    while (1){
        
        printf("\n[Server] Server waiting for client...\n");
        
        if ((listen(socketServidor,10)) != 0){
            printf("[ERROR] Server is full. Try again later.\n");
        }

        tamanhoEndereco[cont] = sizeof(depositoServidor[cont]);
        socketCliente = accept(socketServidor, (struct sockaddr *) &depositoServidor[cont], &tamanhoEndereco[cont]);
        puts("[Server] Client connected...");
		// Associa o socket ao SSL
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl,socketCliente);
		int ssl_err = SSL_accept(ssl);
		if (ssl_err <= 0){
			printf("[Error] SSL could not bind to the socket! \n");
//			return 0;
		}
        clientes[cont].socket = ssl;

        clientes[cont].logged_in = 1;
        cont2 = cont; // Uma cópia para mandar para a thread, pois se mandasse a mesma variavel, ela seria alterada pela main antes de a thread secundaria pegar
        printf("[Server] Thread %d created \n", cont2+1);

        if (pthread_create(&threads[cont],NULL,atendeCliente,&cont2)){
            puts("[ERROR] Error trying to create a thread ");
            return;
        }

        cont = 0;

        while (clientes[cont].logged_in == 1){
            printf("[Server] Client %d is logged, searching for next slot\n", cont+1);
            cont++;
            if (cont == 10)
                cont = 0;
        }
        
    }
}

// REPLICA SERVER FUNCTIONS //

void initializeReplica(int argc, char *argv[], int sckt) {
	// Replica Manager deve ver se há um RMFile e buscar pelo servidor primário
	FILE *handler;
	char *buffer;
	char *masterIP;
	int replicaSocket;
	int foundPrimary = 0;
	int index = 1;
	printf("[Server] Will look for IP list... \n");
	handler = fopen("RMFile.txt", "r");

	if (handler != NULL) {
		printf("[Server] Found RMFile. Reading...\n");
		buffer = readRMFile();
		printf("%s\n", buffer);

		do {
			masterIP = getAddressByIndex(index);
			if ((masterIP == NULL) || (strlen(masterIP) < 7)) {
				printf("[Server] Could not find any active primary.\n");
				printf("[Server] Terminating...\n");
				return;
			}

			printf("[Server] Will try to connect to %s at 53001 . . .\n", masterIP);
			// replicaSocket = criaSocketServidor(argv[1], 53001);
			replicaSocket = connectTo(masterIP, 53001, argv[1]);

			if (replicaSocket != -1) {
				foundPrimary = 1;
			}

			index++;
		} while (!foundPrimary);			

		printf("[Server] This replica manager is currently responding to [%s]. \n", masterIP);
		replicaLoop(replicaSocket);
		return;

	} else {
		printf("[Server] Could not find a RMFile. Aborting... \n");
		return;
	}
}

void replicaLoop(int socketServidor) {
	// O servidor fica rodando para sempre e quando algum cliente aparece chama a função send_file para mandar algo
    // O segundo parametro do listen diz quantas conexões podemos ter
    socklen_t tamanhoEndereco[10];
    struct sockaddr_storage depositoServidor[10];
    pthread_t threads[10];
	int cont = 0;
    int cont2;
	
	// if( send(socketServidor, primaryIP, sizeof(16), 0) < 0)
 //    {
 //        printf("Send failed... (%s)", primaryIP);
 //        return;
 //    }
 //    printf("IP (%s) Sent...\n", primaryIP);
    printf("[Server] Replica is waiting for Primary...\n");
    
    while (1){
    	if (pingServer(primaryIP, socketServidor) == 0)
    		break;
    }
}

// Setando a conexão TCP com o cliente
int main(int argc, char *argv[]){
    
    int socketServidor, socketCliente;
    struct sockaddr_storage depositoServidor[10];
    socklen_t tamanhoEndereco[10];
    pthread_t threads[10];
    pthread_t daemon;
    int cont = 0;
    int cont2, opt;

	initializeSSL();
    method = SSLv23_server_method();
	ctx = SSL_CTX_new(method);
    if (ctx == NULL){
		ERR_print_errors_fp(stderr);
		abort();
	}

    for (int x = 0; x < 10; x++)
        clientes[x].logged_in = 0;

    if (argc < 2) {
        printf("[Server] No server ip inserted. Hosting at default 127.0.0.1 : 53000.\n");
        socketServidor = criaSocketServidor("127.0.0.1", 53000);
        strcpy(primaryIP, "127.0.0.1");
    } else {
        socketServidor = criaSocketServidor(argv[1],53000);
        printf("[Server] Hosting server at %s : 53000.\n", argv[1]);
        strcpy(primaryIP, argv[1]);
    }


	SSL_CTX_use_certificate_file(ctx,"CertFile.pem",SSL_FILETYPE_PEM);
	SSL_CTX_use_PrivateKey_file(ctx,"KeyFile.pem",SSL_FILETYPE_PEM);

	if (argc >= 3) {
    	opt = atoi(&argv[2][0]);

    	if (opt == 1) {
    		isPrimaryServer = 1;
    		printf("[Server] Defined this server as primary server.\n");
    	}
    	if (opt == 0) {
    		isPrimaryServer = 0;
    		printf("[Server] Defined this server as replica manager.\n");
    	} 
    	if ((opt != 1) && (opt != 0)) {
    		printf("[Server] Unknow replica definition: %d\n", opt);
    		printf("[Server] Aborting...\n");
    		return 0;
    	}
    }

    // Definindo o server como primary server [1] ou replica manager [0]

    if (isPrimaryServer == -1) { // Nenhuma info foi adicionada, abre server versão "standalone".
    	printf("[Server] Opened a standalone server. No replicas will be linked.\n");
    	primaryLoop(socketServidor);
    }

    if (isPrimaryServer == 1) {
    	initializePrimary(argc, argv, socketServidor);
    }

    if (isPrimaryServer == 0) {
    	initializeReplica(argc, argv, socketServidor);
    }
    
    return 0;

}
