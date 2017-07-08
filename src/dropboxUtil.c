//
//  dropboxUtil.c
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 

#include "../include/dropboxUtil.h"

#define	EINPROGRESS	36		/* Operation now in progress */

#if !defined(_SYS_ERRNO_H_)
#define	_SYS_ERRNO_H_
#endif

#if !defined(KERNEL) && !defined(KERNEL_PRIVATE)
#include <sys/cdefs.h>
__BEGIN_DECLS
extern int * __error __P((void));
#define errno (*__error())
__END_DECLS
#endif

// Cria o endereco do servidor

struct sockaddr_in retornaEndereco(char *host, int port){

		struct sockaddr_in enderecoServidor; // Cria uma instancia da estrutura de endereco ip 
    	enderecoServidor.sin_family = AF_INET; // Declara que o endereco do servidor pertence a internet
    	enderecoServidor.sin_port = htons(port); // Indica qual porta estaremos usando
    	enderecoServidor.sin_addr.s_addr = inet_addr(host); // Indica qual o IP de fato estaremos usando
    	memset(enderecoServidor.sin_zero, '\0', sizeof enderecoServidor.sin_zero); // Seta todos os bits do padding pra 0

    return enderecoServidor;
}

// Cria um arquivo para salvar os ips de replica-managers

int createRMFile(char iplist[]) {
    FILE *handler = NULL;
    handler = fopen("RMFile.txt", "w+");

    if (handler != NULL) {
	    
	    fprintf(handler, "%s", iplist);
	    // printf("[CLIENT] Successfully created a RM File.\n");
	    fclose(handler);
	    return 0;
	} else {
		// printf("[CLIENT] Could not create the RM file.\n");
		return 1;
	} 
}

char* readRMFile() {
    FILE* handler;
    handler = fopen("RMFile.txt", "r+");
   	char rmList[1024];
	char *theList;
   	int i = 0;
   	char c;

   	if (handler != NULL) {
		while ((c = getc(handler)) != EOF) {			
			rmList[i] = c;
			i++;
			// putchar(c);
		}

		fclose(handler);
		theList = rmList;
    		return theList;
   	} else {
   		return NULL;
   	}
}

// Retorna o IP da (index) linha do RMFile
char* getAddressByIndex(int index) {

	FILE* handler;
    handler = fopen("RMFile.txt", "r+");
	char *indexIP = malloc(16);
	int i = 1;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    if (handler == NULL)
    	return NULL;

    while ((read = getline(&line, &len, handler)) != -1) {
    	// printf("Retrieved line of length %zu :\n", read);
        // printf("%s", line);
        if (i == index)
        	strcpy(indexIP, line);
        i++;
    }

    fclose(handler);
    if (line != NULL)
        free(line);

    indexIP[strlen(indexIP)-1] = '\0';

    return indexIP;
}

// int connectTo(char *host, int port) {
//     //Primeiro parametro indica qual o dominio da internet, nesse caso IPV4
//     //Segundo parametro indica que estaremos usando TCP, ao inves de UDP
//     //Terceiro parametro fala pro SO usar o protocolo padrão
//     int response;
//     struct timeval tmvl;

//     int skt = socket(PF_INET, SOCK_STREAM, 0); // Cria o socket do cliente
//     if (skt < 0) {
//     	printf("[Server] Error creating socket.\n");
//     	return 0;
//     }

//     socklen_t tamanho_endereco;
    
//     //Setando as configurações básicas para se conectar ao servidor
//     struct sockaddr_in endereco;
//     endereco = retornaEndereco(host, port);
    
//     tamanho_endereco = sizeof endereco;
    
//     response = connect(skt, (struct sockaddr *) &endereco, tamanho_endereco); // Conecta de fato agora o socket do cliente com o endereço do servidor

//     if (response < 0) {

//     }
    
//     return skt; // Retorna o socket a ser usado
// }

int connectTo(char *host, int port) {
	int res, valopt; 
	  struct sockaddr_in addr; 
	  long arg; 
	  fd_set myset; 
	  struct timeval tv; 
	  socklen_t lon; 

	  // Create socket 
	  int soc = socket(AF_INET, SOCK_STREAM, 0); 

	  // Set non-blocking 
	  arg = fcntl(soc, F_GETFL, NULL); 
	  arg |= O_NONBLOCK; 
	  fcntl(soc, F_SETFL, arg); 

	  // Trying to connect with timeout 
	  addr.sin_family = AF_INET; 
	  addr.sin_port = htons(2000); 
	  addr.sin_addr.s_addr = inet_addr(host); 
	  addr.sin_port = htons(port);
	  res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 

	  if (res < 0) { 
	     if (errno == EINPROGRESS) { 
	        tv.tv_sec = 5;
	        tv.tv_usec = 0; 
	        FD_ZERO(&myset); 
	        FD_SET(soc, &myset); 
	        if (select(soc+1, NULL, &myset, NULL, &tv) > 0) { 
	           lon = sizeof(int); 
	           getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
	           if (valopt) { 
	              fprintf(stderr, "[Server] Error in connection %d - %s\n", valopt, strerror(valopt)); 
	              return -1; 
	           } 
	        } 
	        else { 
	           fprintf(stderr, "[Server] Timeout from %s\n", host); 
	           return -1;
	        } 
	     } 
	     else { 
	        fprintf(stderr, "[Server] Error connecting %d - %s\n", errno, strerror(errno)); 
	        return -1;
	     } 
	  } 
	  // Set to blocking mode again... 
	  arg = fcntl(soc, F_GETFL, NULL); 
	  arg &= (~O_NONBLOCK); 
	  fcntl(soc, F_SETFL, arg); 
	  // I hope that is all 
	  printf("[Server] Connection successful!\n");
	  return res;
}

int checkPrimary(char primaryIP[], char myIP[]) {

	int replicaSocket;
	int isConnected = 0;
	char *ip = malloc(16);
	strcpy(ip, myIP);

	replicaSocket = connectTo(primaryIP, 53001);
	if (replicaSocket == -1)
		return 0;
	
	send(replicaSocket, ip, sizeof(ip),0); // Envia o ip da replica para o servidor primario
    recv(replicaSocket, &isConnected, sizeof(isConnected), 0); // Recebe o aval do servidor primario

	return isConnected;
}

void imprimir_menu(char *user) {
	printf("\n\n");
	printf("[Server] Greetings, %s. What do you want to do?\n", user);
    puts("\t [1] to syncronize your directory.");
    puts("\t [2] to send a file to the server.");
    puts("\t [3] to receive a file from the server.");
    puts("\t [4] to list the files in your directory.");
    puts("\t [0] to disconnect from the server.");
    printf(">> ");
}
