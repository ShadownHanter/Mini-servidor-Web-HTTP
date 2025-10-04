// client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "client.h"   // Inclui nossa própria interface
#include "libtslog.h" // Inclui a biblioteca de log

#define BUFFER_SIZE 1024

// --- Funções Privadas (static) ---
// Estas funções só serão usadas dentro deste arquivo.

// Thread para RECEBER mensagens do servidor
static void *receive_thread(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char server_reply[BUFFER_SIZE];
    ssize_t read_size;

    while ((read_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0)) > 0) {
        server_reply[read_size] = '\0';
        printf("%s", server_reply);
        fflush(stdout); // Garante que a mensagem apareça imediatamente
    }

    if (read_size == 0) {
        tslog_info("Servidor encerrou a conexao.");
        printf("\nServidor desconectado. Pressione Enter para sair.\n");
    } else {
        tslog_error("Falha ao receber dados (recv).");
    }

    // Força o programa principal a encerrar caso a conexão caia
    exit(0);
    return NULL;
}

// Thread para ENVIAR mensagens para o servidor
static void *send_thread(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char message[BUFFER_SIZE];

    // Lê continuamente do teclado (stdin)
    while (fgets(message, BUFFER_SIZE, stdin)) {
        if (write(sock, message, strlen(message)) < 0) {
            tslog_error("Falha ao enviar mensagem. A conexão pode ter sido encerrada.");
            break;
        }
    }
    return NULL;
}


// --- Funções Públicas (definidas em client.h) ---

int connect_to_server(const char *ip, int port) {
    int sock;
    struct sockaddr_in server;

    // 1. Criar o socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        tslog_fatal("Nao foi possivel criar o socket");
        return -1;
    }
    tslog_info("Socket do cliente criado.");

    // 2. Configurar o endereço do servidor
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // 3. Conectar
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        tslog_error("Conexao falhou com o servidor %s na porta %d", ip, port);
        close(sock);
        return -1;
    }

    tslog_info("Conectado ao servidor %s:%d", ip, port);
    return sock;
}

void start_client_loops(int sock) {
    pthread_t recv_thread_id;
    pthread_t send_thread_id;

    // Cria a thread que fica escutando as mensagens do servidor
    if (pthread_create(&recv_thread_id, NULL, receive_thread, (void*) &sock) < 0) {
        tslog_error("Nao foi possivel criar a thread de recebimento");
        return;
    }

    // Cria a thread que fica lendo o teclado do usuário e enviando
    if (pthread_create(&send_thread_id, NULL, send_thread, (void*) &sock) < 0) {
        tslog_error("Nao foi possivel criar a thread de envio");
        return;
    }

    // Espera as threads terminarem. Na prática, elas rodam "para sempre"
    // até que a conexão seja fechada e o programa encerrado.
    pthread_join(recv_thread_id, NULL);
    pthread_join(send_thread_id, NULL);
}

void shutdown_client(int sock) {
    tslog_info("Encerrando cliente.");
    close(sock);
}

// O main fica muito mais limpo!
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <ip_servidor> <porta>\n", argv[0]);
        return 1;
    }

    if (tslog_init("client.log", TSLOG_DEBUG) != 0) {
        perror("Falha ao inicializar o logger");
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int client_socket = connect_to_server(ip, port);
    if (client_socket < 0) {
        tslog_fatal("Nao foi possivel conectar ao servidor.");
        tslog_close();
        return 1;
    }
    
    printf("Conectado ao chat. Digite suas mensagens e pressione Enter.\n");

    // Esta função agora contém o loop principal das threads
    start_client_loops(client_socket);

    // Este código só será alcançado se as threads terminarem (ex: desconexão)
    shutdown_client(client_socket);
    tslog_close();
    
    return 0;
}
