LuaUV
LuaUV é uma biblioteca que integra Lua com libuv, permitindo a execução de tarefas Lua de forma assíncrona em threads separadas. Ela utiliza o loop de eventos do libuv para garantir o envio seguro de funções, gerenciamento de deadlocks e coleta de lixo automática.
Estrutura do Projeto
A organização de diretórios do projeto segue o padrão abaixo:
luauv/
├── luauv.h           # Interface da biblioteca
├── luauv.c           # Implementação principal
├── lua/              # Código-fonte do Lua (Dependência)
├── libuv/            # Código-fonte do libuv (Dependência)
├── tests/            # Exemplos e suítes de teste
├── CMakeLists.txt    # Configuração de build
├── build/            # Binários gerados
└── compile           # Script auxiliar de compilação

Inicialização Rápida
Para usar a LuaUV, você deve inicializar a estrutura, iniciar a thread de processamento e então postar suas tarefas.
#include "luauv.h"

int main() {
    luauv_t uv;
    
    // 1. Inicializa e inicia a thread do loop
    luauv_init(&uv);
    luauv_start(&uv);

    // 2. Envia uma tarefa (task) para a thread Lua
    // Exemplo usando uma função auxiliar:
    luauv_post(&uv, [](lua_State* L){
        luaL_dostring(L, "print('Olá do Lua!')");
    });

    // 3. Finaliza a execução
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}

API Reference (Funções Principais)
| Função | Descrição |
|---|---|
| luauv_init(luauv_t* self) | Inicializa a estrutura interna e o estado da biblioteca. |
| luauv_start(luauv_t* self) | Cria a thread dedicada e inicia o loop libuv junto ao lua_State. |
| luauv_post(luauv_t* self, void (*func)(lua_State*)) | Agenda uma função C para execução na thread Lua. Esta chamada é bloqueante até o término da tarefa. |
| luauv_stop(luauv_t* self) | Solicita a parada do loop de eventos e aguarda o fechamento da thread. |
| luauv_destroy(luauv_t* self) | Libera todos os recursos, fecha o estado Lua e limpa a memória. |
Testes e Compilação
Criando um Teste Básico
Crie o arquivo em tests/test_basic.c:
#include "luauv.h"
#include <stdio.h>

void print_task(lua_State* L) {
    luaL_dostring(L, "print('Executando tarefa Lua via Thread!')");
}

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);

    // Posta a tarefa para a thread secundária
    luauv_post(&uv, print_task);

    luauv_stop(&uv);
    luauv_destroy(&uv);
    return 0;
}

Compilando e Executando
O script compile gerencia o download automático das versões corretas das dependências.
# Dar permissão de execução
chmod +x compile

# Compilar o teste
./compile tests/test_basic.c -o test_basic

# Executar
./test_basic

Saída Esperada:
> Executando tarefa Lua via Thread!
> 
Dicas Importantes
> [!CAUTION]
> Aviso de Deadlock: Jamais chame luauv_post de dentro de uma função que já está rodando na thread Lua. Isso causará um travamento infinito (Deadlock), pois a thread tentará esperar por si mesma.
> 
 * Scripts Externos: Você pode utilizar luaL_loadfile dentro do luauv_post para carregar arquivos .lua complexos em vez de strings curtas.
 * Coleta de Lixo: O uso do loop idle do libuv permite que o coletor de lixo (GC) do Lua funcione de forma eficiente sem interromper o fluxo principal do programa C.
Gostaria que eu criasse um script de exemplo em Lua para interagir com essa integração via C?
