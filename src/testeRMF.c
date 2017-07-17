#include <stdio.h>
#include <string.h>
#include "dropboxUtil.c"

int main () {
	
	char *lido;
	int opt = createRMFile("127.0.0.1\n192.168.2.101\n");

	if (opt == 0) {
		printf("Rolou criar. Vou ler...");

		lido = readRMFile();
		printf("Lido: \n%s\n", lido);

	} else {

		printf("Nao rolou");
	}


}