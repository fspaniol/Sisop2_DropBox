# Sisop2_DropBox
Trabalho de Drop Box de Sistemas Operacionais 2 da UFRGS

O Objetivo do trabalho é: 
Criar um dropbox no modelo Cliente Servidor com o uso de Sockets.

Problemas na versão atual:

Falta fazer a sincronização dos folders ambos do cliente e do servidor.

Falta fazer o dropboxServer sempre pegar o localhost como default, como feito no exemplo pelo Alberto.

Tem que usar as estruturas fornecidas pelo professor para armazenar os dados dos clientes e dos arquivos.

Todas threads usam o mesmo valor em indice, então ao tentarem sinalizar que estão desconectadas, sempre sinalizam que a ultima conectada se desconectou

Detalhes: Os arquivos estão sendo mandados não como um stream de bytes, mas como blocos de 1024 bytes, isso facilita a criação dos buffers
e não da overload no sistema e sockets como um todo.

Atenção, ao conectar no servidor com um cliente, favor sempre sair com a opção de desconectar, ao inves de simplesmente "parar" a execução com
ctrl + c, isso pode levar o servidor a pensar que tu ainda estás conectado e então dar erro quando outro cliente tentar conectar-se.
