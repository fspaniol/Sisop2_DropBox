//
//  dropboxClient.h
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

#ifndef dropboxClient_h
#define dropboxClient_h

#include <stdio.h>
#include "dropboxUtil.h"

int connect_server(char *host, int port); // Conecta o cliente com o servidor, host - endereço do servidor, port - porta aguardando conexão

void sync_client();                       // Sincroniza o diretório "sync_dir_<nomeusuário>" com o servidor

void send_file_cliente(char *file, int socket);               // Envia um arquivo file para o servidor. Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void get_file(char *file, int socket);                // Obtém um arquivo file do servidor. Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void close_connection();                  // Fecha a conexão com o servidor.

#endif /* dropboxClient_h */
