#!/bin/bash

# --- Configurações ---
HOST="127.0.0.1"
PORT="8080"
BACKLOG="10"
SERVER_EXEC="./output/server"
WEB_ROOT="./www"
INDEX_FILE="index.html"

# Cores para a saída
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # Sem Cor

# Variáveis para contar os testes
total_tests=0
passed_tests=0

# Função para rodar um teste
run_test() {
    local test_name="$1"
    local path="$2"
    local expected_status="$3"
    
    ((total_tests++))
    
    echo -n "Rodando teste: '$test_name'... "
    
    # -s: Modo silencioso (não mostra barra de progresso)
    # -o /dev/null: Joga o corpo da resposta fora (só nos importamos com o cabeçalho)
    # -w "%{http_code}": Escreve o código de status HTTP na saída padrão
    # --max-time 2: Define um timeout de 2 segundos para evitar que o script trave
    local http_status=$(curl -s -o /dev/null -w "%{http_code}" --max-time 2 http://${HOST}:${PORT}${path})
    
    if [ "$http_status" == "$expected_status" ]; then
        echo -e "${GREEN}PASSOU${NC} (Esperado: $expected_status, Recebido: $http_status)"
        ((passed_tests++))
    else
        echo -e "${RED}FALHOU${NC} (Esperado: $expected_status, Recebido: $http_status)"
    fi
}

# --- Execução do Script ---

echo "--- Iniciando Testes do Servidor HTTP ---"

# 1. Preparação: Verificar se o executável e o diretório www existem
if [ ! -f "$SERVER_EXEC" ]; then
    echo "Erro: Executável do servidor '$SERVER_EXEC' não encontrado. Compile o projeto primeiro com 'make'."
    exit 1
fi
if [ ! -d "$WEB_ROOT" ]; then
    echo "Erro: Diretório web '$WEB_ROOT' não encontrado. Crie-o com um arquivo '$INDEX_FILE'."
    exit 1
fi

# 2. Iniciar o servidor em segundo plano com os DOIS argumentos necessários
$SERVER_EXEC $PORT $BACKLOG & 
SERVER_PID=$!
sleep 1 # Dar um tempo para o servidor iniciar

# Verificar se o servidor realmente iniciou
if ! ps -p $SERVER_PID > /dev/null; then
    echo "Erro: Falha ao iniciar o servidor. Verifique os logs ('cat server.log')."
    exit 1
fi
echo "Servidor iniciado com PID: $SERVER_PID"
echo ""

# 3. Rodar os testes
run_test "Requisição GET para arquivo existente" "/$INDEX_FILE" "200"
run_test "Requisição GET para a raiz" "/" "200"
run_test "Requisição GET para arquivo inexistente" "/pagina_que_nao_existe.html" "404"
run_test "Requisição com Directory Traversal" "/../../../../etc/passwd" "400"

echo ""

# 4. Encerrar o servidor
echo "Encerrando o servidor (PID: $SERVER_PID)..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null 

# 5. Mostrar o resumo
echo "--- Resumo dos Testes ---"
if [ "$passed_tests" == "$total_tests" ]; then
    echo -e "${GREEN}TODOS OS $total_tests TESTES PASSARAM!${NC}"
else
    echo -e "${RED}$((total_tests - passed_tests)) de $total_tests testes FALHARAM.${NC}"
fi
echo "--------------------------"