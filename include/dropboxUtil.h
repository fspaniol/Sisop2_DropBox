//
//  dropboxUtil.h
//  
//
//  Created by Athos Lagemann, Fernando Spaniol, Gabriel Conte and Ricardo Sabedra on 23/04/17.
//
//

//  Constantes e métodos auxiliares

#ifndef dropboxUtil_h
#define dropboxUtil_h
#define MAXNAME 100
#define MAXFILES 10

#include <stdio.h>
#include "dropboxClient.h"
#include "dropboxServer.h"

struct sockaddr_in retornaEndereco(char *host, int port); // Cria o endereco do servidor
void imprimir_menu(); // imprime o menu para o cliente

#endif /* dropboxUtil_h */
