#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdatomic.h>

#include "libtslog.h"
#include "server.h"
#include "worker.h"

// Para o controle do loop principal e desligamento
volatile atomic_int keep_running = 1;
static int g_server_socket = -1;

// Para coletar estatísticas
server_stats_t stats = {0};


// --- Handlers de Sinais ---

// Handler para o sinal de desligamento (SIGINT / Ctrl+C)
void shutdown_handler(int signal) {
    char log_buffer[100];
    snprintf(log_buffer, sizeof(log_buffer), "Sinal %d (SIGINT) recebido, iniciando desligamento...", signal);
    tslog_info(log_buffer);
    keep_running = 0;
    if (g_server_socket != -1) {
        shutdown(g_server_socket, SHUT_RDWR);
    }
}

// Handler para o sinal de exibição de estatísticas (SIGUSR1)
void stats_handler(int signal) {
    char log_buffer[256];
    
    snprintf(log_buffer, sizeof(log_buffer),
             "\n--- ESTATÍSTICAS DO SERVIDOR ---\n"
             "    Requisições Totais..: %u\n"
             "    Respostas 200 OK....: %u\n"
             "    Erros 404 Not Found.: %u\n"
             "    Erros 400 Bad Request: %u\n"
             "    ----------------------------------",
             stats.total_requests,
             stats.successful_requests,
             stats.not_found_errors,
             stats.bad_requests);
    tslog_info(log_buffer);
}


// --- Função Principal do Servidor ---

int start_server(int port, int backlog) {
    struct sockaddr_in server_addr;
    char log_buffer[100]; 

    memset(&server_addr, 0, sizeof(server_addr));

    g_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_socket == -1) {
        tslog_fatal("Nao foi possivel criar o socket");
        return -1;
    }
    tslog_info("Socket do servidor criado.");
    
    int opt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(g_server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        snprintf(log_buffer, sizeof(log_buffer), "Bind falhou na porta %d", port);
        tslog_fatal(log_buffer);
        return -1;
    }
    snprintf(log_buffer, sizeof(log_buffer), "Bind realizado na porta %d", port);
    tslog_info(log_buffer);

    if (listen(g_server_socket, backlog) < 0) {
        tslog_fatal("Listen falhou");
        close(g_server_socket);
        return -1;
    }
    snprintf(log_buffer, sizeof(log_buffer), "Servidor escutando na porta %d, fila de %d conexões...", port, backlog);
    tslog_info(log_buffer);

    // Loop principal atualizado para verificar a flag 'keep_running'
    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(g_server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            if (keep_running) {
                tslog_error("accept falhou");
            }
            continue;
        }
        
        client_context_t *context = malloc(sizeof(client_context_t));
        if (context == NULL) {
            tslog_error("Falha ao alocar memória para o contexto do cliente");
            close(client_socket);
            continue;
        }
        context->socket = client_socket;
        inet_ntop(AF_INET, &client_addr.sin_addr, context->ip_addr, sizeof(context->ip_addr));

        snprintf(log_buffer, sizeof(log_buffer), "Conexao aceita de %s", context->ip_addr);
        tslog_info(log_buffer);
        
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client_thread, (void*) context) != 0) {
            tslog_error("Nao foi possivel criar a thread do worker");
            free(context);
            close(client_socket);
        } else {
            pthread_detach(thread_id);
        }
    }

    tslog_info("Loop principal encerrado. Fechando socket do servidor.");
    close(g_server_socket);
    g_server_socket = -1;
    return 0;
}