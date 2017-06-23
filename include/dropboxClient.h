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

void sync_client(int socketCliente);                       // Sincroniza o diretório "sync_dir_<nomeusuário>" com o servidor

void send_file_cliente(int socket);               // Envia um arquivo file para o servidor. Deverá ser executada quando for realizar upload de um arquivo, file - path/filename.ext do arquivo a ser enviado

void get_file(int socket);                // Obtém um arquivo file do servidor. Deverá ser executada quando for realizar download de um arquivo, file -filename.ext

void list_files(int socket); // Lista os arquivos contidos no diretório do cliente

void close_connection();                  // Fecha a conexão com o servidor.

void get_info(char* buffer, char* mensagem); // Pega alguma informacao do stdin e poe no buffer

void *daemonMain(void *parametros); // Funcao que faz a sincronizacao do cliente a cada 10 minutos

void send_file_sync(int socket, char* arquivo); // Envia arquivos quando sincronizando

void get_file_sync(int socket, char* arquivo); // Recebe arquivos quando sincronizando

void getTimeServer(); // Recebe o horário local do servidor

void algoritmo_cristian(); // Calcula o horario do server com o algoritmo de cristian

#endif /* dropboxClient_h */
