# Makefile para o projeto MINI-SERVIDOR-WEB-HTTP

# --- Variáveis de Compilação ---
CC = gcc
# Flags de compilação: 
# -g para informações de debug (essencial para usar o gdb)
# -Wall para mostrar todos os avisos (boa prática)
# -I. para o compilador procurar arquivos de cabeçalho (.h) no diretório atual
# -pthread é necessário para compilar e linkar código que usa a biblioteca Pthreads
CFLAGS = -g -Wall -I. -pthread
LDFLAGS = -pthread

# --- Definição dos Arquivos ---

# Nome dos executáveis que serão gerados na pasta 'output'
SERVER_EXEC = output/server
CLIENT_EXEC = output/client

# Lista de todos os arquivos fonte (.c) necessários para o SERVIDOR
SRCS_SERVER = main.c serve.c worker.c interface.c libtslog.c

# Lista de todos os arquivos fonte (.c) necessários para o CLIENTE
# (Assumindo que você terá client.c e usará a libtslog)
SRCS_CLIENT = client.c libtslog.c

# Gera automaticamente a lista de arquivos objeto (.o) a partir das listas de fontes
OBJS_SERVER = $(SRCS_SERVER:.c=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)


# --- Regras (Targets) ---

# A regra 'all' é a padrão. Será executada quando você digitar apenas 'make'
# Ela depende das regras 'server' e 'client'
all: server client

# Regra para criar o executável do SERVIDOR
server: $(OBJS_SERVER)
	@mkdir -p output  # Garante que o diretório 'output' exista
	@echo "Linkando o executável do servidor..."
	$(CC) $(LDFLAGS) $^ -o $(SERVER_EXEC)
	@echo "Servidor '$(SERVER_EXEC)' pronto."

# Regra para criar o executável do CLIENTE
client: $(OBJS_CLIENT)
	@mkdir -p output # Garante que o diretório 'output' exista
	@echo "Linkando o executável do cliente..."
	$(CC) $(LDFLAGS) $^ -o $(CLIENT_EXEC)
	@echo "Cliente '$(CLIENT_EXEC)' pronto."

# Regra genérica para compilar qualquer arquivo .c em um arquivo .o
# O make usará esta regra para todos os .o que ele precisar criar
%.o: %.c
	@echo "Compilando $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Regra 'clean' para limpar todos os arquivos gerados (objetos e executáveis)
clean:
	@echo "Limpando arquivos de build..."
	@rm -f *.o $(SERVER_EXEC) $(CLIENT_EXEC)
	@echo "Limpeza concluída."

# Declara que 'all', 'clean', 'server', 'client' não são nomes de arquivos
.PHONY: all clean server client