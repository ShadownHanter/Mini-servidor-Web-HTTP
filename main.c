#include <stdio.h>
#include <stdlib.h>
#include <signal.h> // Essencial para a função signal()
#include "libtslog.h"
#include "server.h"

int main(int argc, char *argv[]) {
    
    // Esta é a primeira coisa que fazemos para garantir que sejam capturados.
    signal(SIGINT, shutdown_handler);  // Para Ctrl+C
    signal(SIGUSR1, stats_handler); // Para pedir estatísticas

    // Verifica o número de argumentos
    if (argc < 3) {
        fprintf(stderr, "Erro: Argumentos insuficientes.\n");
        fprintf(stderr, "Uso: %s <porta> <limite_fila>\n", argv[0]);
        return 1;
    }

    // Inicializa o logger
    if (tslog_init("server.log", TSLOG_DEBUG) != 0) {
        perror("Falha ao inicializar o logger");
        return 1;
    }

    // Processa os argumentos da linha de comando
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        tslog_fatal("Porta inválida fornecida: %s", argv[1]);
        fprintf(stderr, "Erro: Porta inválida: '%s'.\n", argv[1]);
        tslog_close();
        return 1;
    }

    int backlog = atoi(argv[2]);
    if (backlog <= 0) {
        tslog_fatal("Limite de fila inválido: %s", argv[2]);
        fprintf(stderr, "Erro: Limite de fila inválido: '%s'.\n", argv[2]);
        tslog_close();
        return 1;
    }

    char log_buffer[100];
    snprintf(log_buffer, sizeof(log_buffer), "Iniciando servidor na porta %d com limite de fila %d...", port, backlog);
    tslog_info(log_buffer);
    
    start_server(port, backlog);

    tslog_info("Servidor encerrado de forma limpa.");
    tslog_close();
    
    return 0;
}