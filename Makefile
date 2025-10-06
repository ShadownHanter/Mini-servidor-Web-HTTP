CC = gcc
CFLAGS = -g -Wall -I. -pthread
LDFLAGS = -pthread

# --- Alvos Executáveis ---
SERVER_EXEC = output/server
CLIENT_EXEC = output/client

# --- Listas de Arquivos Objeto ---
OBJS_SERVER = main.o serve.o worker.o interface.o libtslog.o
OBJS_CLIENT = client.o libtslog.o

# --- Regras de Compilação ---

all: $(SERVER_EXEC) $(CLIENT_EXEC)

# Linka o SERVIDOR
$(SERVER_EXEC): $(OBJS_SERVER)
	@mkdir -p output
	$(CC) $(LDFLAGS) $(OBJS_SERVER) -o $(SERVER_EXEC)
	@echo "Servidor '$(SERVER_EXEC)' pronto."

# Linka o CLIENTE
$(CLIENT_EXEC): $(OBJS_CLIENT)
	@mkdir -p output
	$(CC) $(LDFLAGS) $(OBJS_CLIENT) -o $(CLIENT_EXEC)
	@echo "Cliente '$(CLIENT_EXEC)' pronto."

# --- Regras de Dependência Explícitas ---
main.o: main.c server.h libtslog.h
	$(CC) $(CFLAGS) -c main.c -o main.o

serve.o: serve.c server.h worker.h libtslog.h
	$(CC) $(CFLAGS) -c serve.c -o serve.o

worker.o: worker.c worker.h server.h libtslog.h
	$(CC) $(CFLAGS) -c worker.c -o worker.o

client.o: client.c client.h libtslog.h
	$(CC) $(CFLAGS) -c client.c -o client.o

libtslog.o: libtslog.c libtslog.h
	$(CC) $(CFLAGS) -c libtslog.c -o libtslog.o

interface.o: interface.c interface.h
	$(CC) $(CFLAGS) -c interface.c -o interface.o

# --- Regra de Limpeza ---
clean:
	@rm -f *.o
	@rm -rf output
	@echo "Limpeza concluída."

.PHONY: all clean