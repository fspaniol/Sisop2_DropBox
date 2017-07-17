//
//  dropboxServer.h
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#ifndef dropboxServer_h
#define dropboxServer_h

#include <stdio.h>
#include <pthread.h>
#include "dropboxUtil.h"

struct file_info{
    char name[MAXNAME];             // Refere-se ao nome do arquivo
    char extension[MAXNAME];        // Refere-se ao tipo de extensão do arquivo
    char last_modified[MAXNAME];    // Refere-se a data da última modificação do arquivo
    int size;                       // Indica o tamanho do arquivo, em bytes
};

struct Client{
    int devices[2];                 // Associado aos dispositivos do usuário
    char userid[MAXNAME];           // Id do usuário no servidor, que deverá ser único. Informado pela linha de comando
    struct file_info files[MAXFILES]; // Metadados de cada arquivo que o cliente possui no servidor
    int logged_in;                  // Cliente está logado ou não
};

struct Replica{
	int binded;						// Pra dizer se a replica está ok
	char ip[16];					// IP da replica
	int pid;						// PID da replica (caso a gente faça um algoritmo de eleição de líder)
};

int criaSocketServidor(char *host, int port); // Cria o socket do servidor

void sync_server();                 // Sincroniza o servidor com o diretório "sync_dir_<nomeusuário>" com o cliente

void receive_file(int socket, char* usuario);      // Recebe um arquivo file do cliente. Deverá ser executada quando for realizar upload de um arquivo. file - path/filename.ext do arquivo a ser recebido

void receive_file_sync(int socket, char* usuario); // Recebe o arquivo e envia de volta o tempo no qual foi modificado

void send_file_servidor(int socket, char* usuario);         // Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

void list_files_server(int socket, char* usuario); // Lista todos os files do diretório de cada usuário

void cria_pasta_usuario(char* usuario); // Cria uma pasta no servidor para o usuario passado como parametro

void *atendeCliente(void *indice); // Menu principal para atender a cada usuario

int conta_conexoes_usuario(char *usuario); // Conta as conexoes presentes para um usuario

void send_time_modified(int socket, char* usuario); // Envia a data quando o arquivo foi modificado

// REPLICA MANAGER

void send_ServerList(int socket, char* usuario); // Envia para o cliente a lista de servers

int updateReplicas(); // Repassa as mudanças feitas no RM primário para os secundários

void initializePrimary(int argc, char *argv[], int sckt); // Inicializa o server como server primário

void primaryLoop(int socketServidor); // Abre o loop de recebimento do servidor

void initializeReplica(int argc, char *argv[], int sckt); // Inizializa o server como replica

void replicaLoop(int socketServidor); // Abre o loop da replica, que ouve o primario


#endif /* dropboxServer_h */
