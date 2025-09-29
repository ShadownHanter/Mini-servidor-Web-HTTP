#include "libtslog.h"

int main() {
    if (tslog_init("meu_log.txt", TSLOG_DEBUG) != 0) {
        printf("Erro ao inicializar logger\n");
        return 1;
    }

    tslog_info("Servidor iniciado na porta %d", 8080);
    tslog_debug("Carregando configuração...");
    tslog_warn("Uso de memória alto");
    tslog_error("Falha ao abrir arquivo %s", "dados.txt");
    tslog_fatal("Erro crítico: encerrando");

    tslog_close();
    return 0;
}
