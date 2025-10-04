// client.h

#ifndef CLIENT_H
#define CLIENT_H

// Inicia a conexão com o servidor e retorna o socket
// Retorna -1 em caso de erro.
int connect_to_server(const char *ip, int port);

// Inicia as threads de envio e recebimento de mensagens
void start_client_loops(int sock);

// Fecha a conexão e libera recursos
void shutdown_client(int sock);

#endif
