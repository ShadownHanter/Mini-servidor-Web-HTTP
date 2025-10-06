// server.h (Versão Corrigida e Completa)

#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Esta struct vai carregar toda a informação que a thread do worker precisa.
typedef struct {
    int socket;
    char ip_addr[16]; // Espaço para um endereço IPv4 + terminador nulo
} client_context_t;


// --- Protótipos das Funções ---

// Função principal que inicia o servidor e entra no loop de aceitação
int start_server(int port);

// Função para enviar uma mensagem a todos, exceto o remetente
void broadcast_message(const char *message, int sender_socket); // <-- ADICIONE ESTA LINHA

#endif