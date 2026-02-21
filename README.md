# luauv
LuaUV  Biblioteca que integra Lua com libuv, permitindo executar tarefas Lua de forma assíncrona em threads separadas usando o loop de eventos do libuv. Suporta envio seguro de funções Lua, gerenciamento de deadlocks e coleta de lixo automática.


Uso e Testes do LuaUV

Este guia mostra como configurar, usar e testar a biblioteca LuaUV para integração de Lua com libuv em C.


---

Estrutura do Projeto

luauv/
├── luauv.h
├── luauv.c
├── lua/           # Código-fonte do Lua
├── libuv/         # Código-fonte do libuv
├── tests/         # Exemplos de teste
├── CMakeLists.txt
├── build/         # Diretório de compilação
└── compile        # Script de compilação


---

Inicialização

Para usar LuaUV, primeiro inicialize a estrutura luauv_t:

#include "luauv.h"

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);

    // Enviar uma task para a thread Lua
    luauv_post(&uv, [](lua_State* L){
        luaL_dostring(L, "print('Olá do Lua!')");
    });

    luauv_stop(&uv);
    luauv_destroy(&uv);
    return 0;
}


---

Funções Principais

Função	Descrição

luauv_init(luauv_t* self)	Inicializa a estrutura luauv_t.
luauv_start(luauv_t* self)	Inicia a thread do loop libuv e Lua.
luauv_post(luauv_t* self, void (*func)(lua_State*))	Envia uma função para execução na thread Lua. Bloqueia até completar.
luauv_stop(luauv_t* self)	Para o loop e aguarda o término da thread.
luauv_destroy(luauv_t* self)	Limpa recursos alocados.



---

Testes Básicos

Crie um arquivo em tests/test_basic.c:

#include "luauv.h"
#include <stdio.h>

void print_task(lua_State* L) {
    luaL_dostring(L, "print('Executando tarefa Lua!')");
}

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);

    luauv_post(&uv, print_task);

    luauv_stop(&uv);
    luauv_destroy(&uv);
    return 0;
}

Compilação Rápida

Use o script compile para compilar automaticamente LuaUV e os testes, baixando as versões apropriadas de Lua e libuv:

./compile tests/test_basic.c -o test_basic

Execução

./test_basic
# Saída esperada:
# Executando tarefa Lua!


---

Dicas

Evite chamar luauv_post de dentro da thread Lua para não criar deadlocks.

Você pode usar luaL_dostring ou luaL_loadfile dentro das tasks enviadas.

O loop de libuv cuida de async e idle, permitindo tasks assíncronas e coleta de lixo do Lua.
