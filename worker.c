// worker.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "libtslog.h"
#include "server.h" // Precisa para broadcast_message e MAX_CLIENTS
#include "worker.h"

// A lógica que estava em handle_client() antes
void *handle_client_thread(void *arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    char client_ip_str[INET_ADDRSTRLEN];
    
    // Pega o IP do cliente para logs mais descritivos
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);

    tslog_info("Worker thread iniciada para o cliente %s", client_ip_str);

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        tslog_debug("Recebido de %s: %s", client_ip_str, buffer);

        char broadcast_buffer[BUFFER_SIZE + 50];
        snprintf(broadcast_buffer, sizeof(broadcast_buffer), "[%s]: %s", client_ip_str, buffer);
        
        broadcast_message(broadcast_buffer, client_socket);
    }
    
    // Lógica de desconexão (não mostrada aqui para simplicidade, mas seria bom adicionar)
    // ... remover o socket da lista global ...
    tslog_info("Cliente %s desconectado.", client_ip_str);
    close(client_socket);

    return NULL;
}