// main.c

#include <stdio.h>
#include <stdlib.h>
#include "libtslog.h"
#include "server.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        return 1;
    }
    
    // 1. Inicializa o logger
    if (tslog_init("server.log", TSLOG_DEBUG) != 0) {
        perror("Falha ao inicializar o logger");
        return 1;
    }

    // 2. Converte a porta de string para inteiro
    int port = atoi(argv[1]);

    // 3. Inicia o servidor (que contém o loop infinito)
    start_server(port);

    // Este código nunca será alcançado a menos que start_server retorne
    tslog_close();
    
    return 0;
}