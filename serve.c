// serve.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libtslog.h"
#include "server.h" // Inclui nosso próprio header
#include "worker.h" // Inclui o header do worker

// Array global para manter os sockets dos clientes e um mutex para protegê-lo
static int client_sockets[MAX_CLIENTS] = {0};
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Implementação da função de broadcast
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0 && client_sockets[i] != sender_socket) {
            if (write(client_sockets[i], message, strlen(message)) < 0) {
                tslog_error("Erro ao enviar mensagem para o cliente socket %d", client_sockets[i]);
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}


int start_server(int port) {
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        tslog_fatal("Nao foi possivel criar o socket");
        return -1;
    }
    tslog_info("Socket do servidor criado.");
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        tslog_fatal("Bind falhou na porta %d", port);
        return -1;
    }
    tslog_info("Bind realizado na porta %d", port);

    listen(server_socket, 3);
    tslog_info("Servidor escutando na porta %d...", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            tslog_error("accept falhou");
            continue;
        }

        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        tslog_info("Conexao aceita de %s:%d", client_ip_str, ntohs(client_addr.sin_port));

        // Adiciona cliente ao array e dispara a thread do worker
        pthread_mutex_lock(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        
        if (i == MAX_CLIENTS) {
            tslog_warn("Numero maximo de clientes atingido. Conexao recusada.");
            close(client_socket);
        } else {
            pthread_t thread_id;
            int *new_sock = malloc(sizeof(int));
            *new_sock = client_socket;
            if (pthread_create(&thread_id, NULL, handle_client_thread, (void*) new_sock) != 0) {
                tslog_error("Nao foi possivel criar a thread do worker");
                free(new_sock);
            } else {
                pthread_detach(thread_id);
            }
        }
    }
    close(server_socket);
    return 0;
}