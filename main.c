// main.c (VERSÃO FINAL E CORRIGIDA)

#include <stdio.h>
#include <stdlib.h> // Essencial para atoi()
#include "libtslog.h"
#include "server.h"

int main(int argc, char *argv[]) {
    
    // Passo 1: VERIFICAR o número de argumentos PRIMEIRO!
    // Esta é a correção para o erro "Segmentation Fault".
    if (argc < 2) {
        // Usamos fprintf(stderr, ...) para mensagens de erro, pois é o padrão.
        fprintf(stderr, "Erro: Porta não especificada.\n");
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        return 1; // Encerra o programa com um código de erro.
    }

    // Passo 2: Agora que sabemos que argv[1] existe, podemos inicializar o logger.
    if (tslog_init("server.log", TSLOG_DEBUG) != 0) {
        perror("Falha ao inicializar o logger");
        return 1;
    }

    // Passo 3: Agora é seguro converter e validar argv[1].
    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535) {
        tslog_fatal("Porta inválida fornecida: %s", argv[1]);
        fprintf(stderr, "Erro: Porta inválida: '%s'. Deve ser um número entre 1 e 65535.\n", argv[1]);
        tslog_close();
        return 1;
    }

    // Passo 4: Registrar o início e iniciar o servidor de forma segura.
    // Esta correção com snprintf resolve o bug do log mostrando a porta errada.
    char log_buffer[100];
    snprintf(log_buffer, sizeof(log_buffer), "Iniciando servidor na porta %d...", port);
    tslog_info(log_buffer);
    
    start_server(port);

    // Este código geralmente não é alcançado, mas é uma boa prática fechar o log.
    tslog_close();
    
    return 0;
}