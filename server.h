// server.h (VERSÃO FINAL COM ESTATÍSTICAS E GRACEFUL SHUTDOWN)

#ifndef SERVER_H
#define SERVER_H

#include <stdatomic.h> // Para atomic_int e server_stats_t

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Estrutura para passar dados para a thread do worker
typedef struct {
    int socket;
    char ip_addr[16];
} client_context_t;

// Estrutura para coletar estatísticas
typedef struct {
    atomic_uint total_requests;
    atomic_uint successful_requests;
    atomic_uint not_found_errors;
    atomic_uint bad_requests;
} server_stats_t;


// --- Protótipos de Funções e Variáveis Globais ---

// Variável global para controlar o loop do servidor
extern volatile atomic_int keep_running;

// Variável global para as estatísticas
extern server_stats_t stats;

// Handler para o sinal de desligamento (SIGINT / Ctrl+C)
void shutdown_handler(int signal);

// Handler para o sinal de exibição de estatísticas (SIGUSR1)
void stats_handler(int signal);

// Função principal do servidor
int start_server(int port, int backlog);

#endif