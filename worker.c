// worker.c (Versão HTTP)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> // Para stat()
#include <fcntl.h>    // Para open()

#include "libtslog.h"
#include "server.h" 
#include "worker.h"

#define WEB_ROOT "./www" // Define o diretório raiz para os arquivos web

// --- Funções Auxiliares ---

// Retorna o tipo MIME de um arquivo com base na sua extensão
const char* get_mime_type(const char* path) {
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".jpg")) return "image/jpeg";
    if (strstr(path, ".jpeg")) return "image/jpeg";
    if (strstr(path, ".png")) return "image/png";
    return "application/octet-stream"; // Tipo genérico para arquivos binários
}

// Envia uma resposta de erro para o cliente (ex: 404 Not Found)
void send_error_response(int socket, const char* status_code, const char* message) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.0 %s\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "<h1>%s</h1><p>%s</p>",
             status_code, status_code, message);
    write(socket, response, strlen(response));
}


// --- Função Principal da Thread ---

// worker.c (função handle_client_thread CORRIGIDA)

// worker.c (VERSÃO COMPLETA E CORRIGIDA)

void *handle_client_thread(void *arg) {
    client_context_t *context = (client_context_t*)arg;
    int client_socket = context->socket;
    char client_ip_str[16];
    snprintf(client_ip_str, sizeof(client_ip_str), "%s", context->ip_addr);
    free(context);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Ler a requisição do cliente
    bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return NULL;
    }
    buffer[bytes_read] = '\0';

    // --- PONTO CRÍTICO DA CORREÇÃO ---

    // 1. INICIALIZAR as variáveis para que nunca contenham lixo.
    char method[16] = "";
    char path[256] = "";
    char version[16] = "";
    
    // 2. VERIFICAR se o sscanf conseguiu ler os 3 itens esperados.
    int items_scanned = sscanf(buffer, "%s %s %s", method, path, version);
    if (items_scanned < 3) {
        // Se não conseguiu, a requisição é inválida. Registramos o erro e encerramos.
        tslog_warn("Requisição mal formada recebida de %s. Conteúdo: %s", client_ip_str, buffer);
        send_error_response(client_socket, "400 Bad Request", "A requisição HTTP está mal formada.");
        close(client_socket);
        return NULL; // Encerra a thread com segurança.
    }

    // Se o programa chegou até aqui, é 100% SEGURO logar as variáveis.
    tslog_info("Recebido de %s: %s %s %s", client_ip_str, method, path, version);

    // --- FIM DA CORREÇÃO ---

    // Validar o método
    if (strcmp(method, "GET") != 0) {
        send_error_response(client_socket, "501 Not Implemented", "O servidor suporta apenas o método GET.");
        close(client_socket);
        return NULL;
    }

    // Se o caminho for "/", sirva o "index.html" por padrão
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }

    // Construir o caminho completo e seguro para o arquivo
    char file_path[512];
    snprintf(file_path, sizeof(file_path), "%s%s", WEB_ROOT, path);
    
    if (strstr(file_path, "..")) {
        send_error_response(client_socket, "400 Bad Request", "Caminho de arquivo inválido.");
        close(client_socket);
        return NULL;
    }
    
    // Tentar abrir e servir o arquivo...
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        tslog_warn("Arquivo não encontrado: %s", file_path);
        send_error_response(client_socket, "404 Not Found", "O arquivo solicitado não foi encontrado no servidor.");
    } else {
        struct stat file_stat;
        fstat(file_fd, &file_stat);
        long file_size = file_stat.st_size;
        const char* mime_type = get_mime_type(file_path);

        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header),
                 "HTTP/1.0 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n",
                 mime_type, file_size);
        write(client_socket, header, strlen(header));

        char file_buffer[BUFFER_SIZE];
        ssize_t bytes_read_from_file;
        while ((bytes_read_from_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
            write(client_socket, file_buffer, bytes_read_from_file);
        }
        tslog_info("Arquivo enviado: %s (%ld bytes)", file_path, file_size);
        close(file_fd);
    }
    
    close(client_socket);
    return NULL;
}