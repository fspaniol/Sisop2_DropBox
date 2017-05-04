# Sisop2_DropBox
Trabalho de Drop Box de Sistemas Operacionais 2 da UFRGS

O Objetivo do trabalho é: 
Criar um dropbox no modelo Cliente Servidor com o uso de Sockets.

Problemas na versão atual:

Passar o nome de arquivos por parâmetro via stdin resulta em segmentation fault, aparentemente quando o fgets é executado, ele bota junto
o CR LF na string, o que resulta em problema.

Falta fazer a sincronização dos folders ambos do cliente e do servidor.

Falta fazer o dropboxServer sempre pegar o localhost como default, como feito no exemplo pelo Alberto.

Tem que usar as estruturas fornecidas pelo professor para armazenar os dados dos clientes e dos arquivos.

Aplicar concorrencia no sistema para vários clientes se conectarem ao mesmo tempo.

Detalhes: Os arquivos estão sendo mandados não como um stream de bytes, mas como blocos de 1024 bytes, isso facilita a criação dos buffers
e não da overload no sistema e sockets como um todo.

Atenção, ao conectar no servidor com um cliente, favor sempre sair com a opção de desconectar, ao inves de simplesmente "parar" a execução com
ctrl + c, isso pode levar o servidor a pensar que tu ainda estás conectado e então dar erro quando outro cliente tentar conectar-se.
