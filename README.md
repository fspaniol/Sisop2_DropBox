# Sisop2_DropBox
Trabalho de Drop Box de Sistemas Operacionais 2 da UFRGS

O Objetivo do trabalho é: 
Criar um dropbox no modelo Cliente Servidor com o uso de Sockets.

Problemas na versão atual:

Servidor não reconhece a deleção de arquivos

Não é possivel pegar arquivos de qualquer lugar do computador

O que fazer para a segunda parte:

1 - Sincronização de relógios

2 - Replicação passiva

3 - Autenticação do Cliente/Servidor e comunicação Segura

De agora em diante, na hora de compilar o servidor e o cliente, tem que usar as flags -lssl -lcrypto

Não esquecer de relatar problemas encontrados para botar no relatório



Detalhes: Os arquivos estão sendo mandados não como um stream de bytes, mas como blocos de 1024 bytes, isso facilita a criação dos buffers
e não da overload no sistema e sockets como um todo.

Atenção, ao conectar no servidor com um cliente, favor sempre sair com a opção de desconectar, ao inves de simplesmente "parar" a execução com
ctrl + c, isso pode levar o servidor a pensar que tu ainda estás conectado e então dar erro quando outro cliente tentar conectar-se.
