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
    struct file_info;//[MAXFILES]   // Metadados de cada arquivo que o cliente possui no servidor
    int logged_in;                  // Cliente está logado ou não
};

int criaSocketServidor(char *host, int port); // Cria o socket do servidor

void sync_server();                 // Sincroniza o servidor com o diretório "sync_dir_<nomeusuário>" com o cliente

void receive_file(char *file, int socket);      // Recebe um arquivo file do cliente. Deverá ser executada quando for realizar upload de um arquivo. file - path/filename.ext do arquivo a ser recebido

void send_file(char *file, int socket);         // Envia o arquivo file para o usuário. Deverá ser executada quando for realizar download de um arquivo. file - filename.ext

#endif /* dropboxServer_h */
