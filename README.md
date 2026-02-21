# LuaUV - Integração Lua com libuv

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Lua](https://img.shields.io/badge/Lua-5.5-blue.svg)](http://www.lua.org/)
[![libuv](https://img.shields.io/badge/libuv-1.52-green.svg)](https://libuv.org/)

LuaUV é uma biblioteca em C que integra a máquina virtual Lua com o loop de eventos libuv, permitindo execução assíncrona de scripts Lua em uma thread dedicada.

## 📋 Índice

- [Características](#-características)
- [Pré-requisitos](#-pré-requisitos)
- [Compilação](#-compilação)
  - [Configuração inicial](#configuração-inicial)
  - [Compilando a biblioteca](#compilando-a-biblioteca)
  - [Compilando programas com LuaUV](#compilando-programas-com-luauv)
- [Uso Básico](#-uso-básico)
- [API Reference](#-api-reference)
- [Exemplos](#-exemplos)
- [Testes](#-testes)
- [Limitações](#-limitações)
- [Licenças](#-licenças)

## ✨ Características

- Thread dedicada para execução Lua com loop libuv
- Postagem segura de tarefas para execução na thread Lua
- Coleta de lixo automática via idle callback
- Prevenção contra deadlocks
- Interface simples e intuitiva

## 🔧 Pré-requisitos

- GCC ou Clang
- CMake (3.10+)
- pthreads
- Git (para clonar repositórios)

## 🚀 Compilação

### Configuração inicial

Antes de compilar, você precisa ter os diretórios do Lua e libuv no local esperado. Crie um arquivo `build.properties` na raiz do projeto com o seguinte conteúdo:

```properties
LUA_DIR = lua-5.5.0
UV_DIR = libuv-1.52.0
```

Estes diretórios devem conter os códigos-fonte do Lua 5.5.0 e libuv 1.52.0 respectivamente.

Compilando a biblioteca

```bash
# Criar diretório de build e compilar
mkdir -p build
cd build
cmake ..
make
```

Após a compilação, você encontrará:

· Biblioteca compartilhada: build/lib/libluauv.so
· Headers organizados: build/include/luauv/ (contém luauv.h e subpastas lua/ e uv/ com os headers originais)

Compilando programas com LuaUV

O projeto inclui um script compile que simplifica a compilação de programas que usam a biblioteca:

```bash
# Compilar um programa
./compile tests/test_basic.c

# O executável será gerado em tests/test_basic
```

O script automaticamente:

· Localiza a biblioteca compilada em build/lib/libluauv.so
· Configura os paths de inclusão (build/include/luauv, build/include/luauv/lua, build/include/luauv/uv)
· Linka com a biblioteca e configura o rpath para execução imediata

💻 Uso Básico

Exemplo mínimo

```c
#include "luauv.h"
#include <stdio.h>

void minha_task(lua_State* L) {
    luaL_dostring(L, "print('Olá do Lua!')");
}

int main() {
    luauv_t uv;
    
    // Inicializar e iniciar
    luauv_init(&uv);
    luauv_start(&uv);
    
    // Enviar tarefa para execução
    luauv_post(&uv, minha_task);
    
    // Parar e limpar
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}
```

Executando código Lua de arquivos

```c
void executar_arquivo(lua_State* L) {
    if (luaL_loadfile(L, "script.lua") == LUA_OK) {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    } else {
        fprintf(stderr, "Erro: %s\n", lua_tostring(L, -1));
    }
}
```

📚 API Reference

Estruturas

luauv_t

Estrutura principal que mantém o estado da instância LuaUV.

```c
typedef struct luauv_s {
    lua_State* L;           // Estado Lua
    uv_loop_t* loop;        // Loop libuv
    uv_async_t* async;      // Handler async
    uv_idle_t* idle;        // Handler idle
    pthread_t thread;       // Thread principal
    pthread_t lua_thread_id; // ID da thread Lua
    pthread_mutex_t* mtx;   // Mutex para sincronização
    pthread_cond_t* cv;     // Condition variable
    void (*task)(lua_State*); // Tarefa atual
} luauv_t;
```

Funções

Função Descrição
int luauv_init(luauv_t* self) Inicializa a estrutura
int luauv_start(luauv_t* self) Inicia a thread Lua/libuv
void luauv_post(luauv_t* self, void (*func)(lua_State*)) Envia tarefa para execução (bloqueante)
void luauv_stop(luauv_t* self) Para o loop e aguarda thread
void luauv_destroy(luauv_t* self) Libera recursos alocados

📝 Exemplos

Exemplo 1: Múltiplas tarefas

```c
#include "luauv.h"
#include <stdio.h>
#include <unistd.h>

void tarefa1(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 1: ' .. os.date())");
}

void tarefa2(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 2: Calculando...')");
    luaL_dostring(L, "local s = 0; for i=1,1000000 do s = s + i end; print('Soma:', s)");
}

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);
    
    printf("Enviando tarefas...\n");
    
    luauv_post(&uv, tarefa1);
    luauv_post(&uv, tarefa2);
    luauv_post(&uv, tarefa1);
    
    sleep(1); // Aguardar execução
    
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}
```

Exemplo 2: Estado persistente

```c
void inicializar_estado(lua_State* L) {
    // Criar tabela global compartilhada
    luaL_dostring(L, 
        "compartilhado = {"
        "   contador = 0,"
        "   dados = {}"
        "}"
    );
}

void incrementar_contador(lua_State* L) {
    luaL_dostring(L,
        "compartilhado.contador = compartilhado.contador + 1;"
        "print('Contador:', compartilhado.contador);"
    );
}

// Uso: luauv_post(&uv, incrementar_contador);
```

🧪 Testes

Teste básico (tests/test_basic.c)

```c
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
```

Compilar e executar:

```bash
./compile tests/test_basic.c
./tests/test_basic
# Saída esperada:
# Executando tarefa Lua!
```

⚠️ Limitações

· Deadlock prevention: Não chamar luauv_post de dentro da thread Lua
· Tarefas bloqueantes: A thread principal espera a conclusão da tarefa via condition variable
· Single-threaded Lua: O estado Lua é usado apenas na thread dedicada
· Sem suporte a múltiplos estados: Uma instância gerencia um único estado Lua
· Plataformas: Atualmente suporta apenas sistemas Unix/Linux (sem suporte Windows)

📄 Licenças

· luauv: Este código é licenciado sob a MIT License.
· Lua 5.5.0: Incluído no diretório especificado em build.properties, licenciado sob a MIT License.
    Copyright © 1994–2025 Lua.org, PUC-Rio.
    Repositório oficial de releases do Lua
· libuv 1.52.0: Incluído no diretório especificado em build.properties, licenciado sob a MIT License.
    Repositório oficial de releases do libuv

---

Tags: lua, libuv, async, threading, c, event-loop

```

Este README agora:
- ✅ Não referencia Lua e libuv na estrutura do projeto
- ✅ Explica claramente o processo de build com CMake e o script `compile`
- ✅ Mantém todos os links oficiais solicitados
- ✅ Inclui a seção de licenças completa
- ✅ Documenta a API e exemplos de uso}
```

Executando código Lua de arquivos

```c
void executar_arquivo(lua_State* L) {
    if (luaL_loadfile(L, "script.lua") == LUA_OK) {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    } else {
        fprintf(stderr, "Erro: %s\n", lua_tostring(L, -1));
    }
}
```

📚 API Reference

Estruturas

luauv_t

Estrutura principal que mantém o estado da instância LuaUV.

```c
typedef struct luauv_s {
    lua_State* L;           // Estado Lua
    uv_loop_t* loop;        // Loop libuv
    uv_async_t* async;      // Handler async
    uv_idle_t* idle;        // Handler idle
    pthread_t thread;       // Thread principal
    pthread_t lua_thread_id; // ID da thread Lua
    pthread_mutex_t* mtx;   // Mutex para sincronização
    pthread_cond_t* cv;     // Condition variable
    void (*task)(lua_State*); // Tarefa atual
} luauv_t;
```

Funções

Função Descrição
int luauv_init(luauv_t* self) Inicializa a estrutura
int luauv_start(luauv_t* self) Inicia a thread Lua/libuv
void luauv_post(luauv_t* self, void (*func)(lua_State*)) Envia tarefa para execução (bloqueante)
void luauv_stop(luauv_t* self) Para o loop e aguarda thread
void luauv_destroy(luauv_t* self) Libera recursos alocados

📝 Exemplos

Exemplo 1: Múltiplas tarefas

```c
#include "luauv.h"
#include <stdio.h>
#include <unistd.h>

void tarefa1(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 1: ' .. os.date())");
}

void tarefa2(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 2: Calculando...')");
    luaL_dostring(L, "local s = 0; for i=1,1000000 do s = s + i end; print('Soma:', s)");
}

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);
    
    printf("Enviando tarefas...\n");
    
    luauv_post(&uv, tarefa1);
    luauv_post(&uv, tarefa2);
    luauv_post(&uv, tarefa1);
    
    sleep(1); // Aguardar execução
    
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}
```

Exemplo 2: Estado persistente

```c
void inicializar_estado(lua_State* L) {
    // Criar tabela global compartilhada
    luaL_dostring(L, 
        "compartilhado = {"
        "   contador = 0,"
        "   dados = {}"
        "}"
    );
}

void incrementar_contador(lua_State* L) {
    luaL_dostring(L,
        "compartilhado.contador = compartilhado.contador + 1;"
        "print('Contador:', compartilhado.contador);"
    );
}

// Uso: luauv_post(&uv, incrementar_contador);
```

🧪 Testes

Teste básico (tests/test_basic.c)

```c
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
```

Compilar e executar:

```bash
./compile tests/test_basic.c -o test_basic
./test_basic
# Saída esperada:
# Executando tarefa Lua!
```

⚠️ Limitações

· Deadlock prevention: Não chamar luauv_post de dentro da thread Lua
· Tarefas bloqueantes: A thread principal espera a conclusão da tarefa via condition variable
· Single-threaded Lua: O estado Lua é usado apenas na thread dedicada
· Sem suporte a múltiplos estados: Uma instância gerencia um único estado Lua

📄 Licenças

· luauv: Este código é licenciado sob a MIT License.
· Lua 5.5.0: Incluído na pasta lua-5.5.0/, licenciado sob a MIT License.
    Copyright © 1994–2025 Lua.org, PUC-Rio.
    Repositório oficial de releases do Lua
· libuv 1.52.0: Incluído na pasta libuv-1.52.0/, licenciado sob a MIT License.
    Repositório oficial de releases do libuv

---

Tags: lua, libuv, async, threading, c, event-loop

```

Este `README.md` agora contém:
- ✅ Todos os links oficiais solicitados
- ✅ Seção de licenças completa e correta
- ✅ Documentação técnica completa do projeto
- ✅ Sem informações de contato pessoal

O arquivo está pronto para ser commitado no seu repositório!cmake ..
make
```

💻 Uso Básico

Exemplo mínimo

```c
#include "luauv.h"
#include <stdio.h>

void minha_task(lua_State* L) {
    luaL_dostring(L, "print('Olá do Lua!')");
}

int main() {
    luauv_t uv;
    
    // Inicializar e iniciar
    luauv_init(&uv);
    luauv_start(&uv);
    
    // Enviar tarefa para execução
    luauv_post(&uv, minha_task);
    
    // Parar e limpar
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}
```

Executando código Lua de arquivos

```c
void executar_arquivo(lua_State* L) {
    if (luaL_loadfile(L, "script.lua") == LUA_OK) {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    } else {
        fprintf(stderr, "Erro: %s\n", lua_tostring(L, -1));
    }
}
```

📚 API Reference

Estruturas

luauv_t

Estrutura principal que mantém o estado da instância LuaUV.

```c
typedef struct luauv_s {
    lua_State* L;           // Estado Lua
    uv_loop_t* loop;        // Loop libuv
    uv_async_t* async;      // Handler async
    uv_idle_t* idle;        // Handler idle
    pthread_t thread;       // Thread principal
    pthread_t lua_thread_id; // ID da thread Lua
    pthread_mutex_t* mtx;   // Mutex para sincronização
    pthread_cond_t* cv;     // Condition variable
    void (*task)(lua_State*); // Tarefa atual
} luauv_t;
```

Funções

Função Descrição
int luauv_init(luauv_t* self) Inicializa a estrutura
int luauv_start(luauv_t* self) Inicia a thread Lua/libuv
void luauv_post(luauv_t* self, void (*func)(lua_State*)) Envia tarefa para execução (bloqueante)
void luauv_stop(luauv_t* self) Para o loop e aguarda thread
void luauv_destroy(luauv_t* self) Libera recursos alocados

📝 Exemplos

Exemplo 1: Múltiplas tarefas

```c
#include "luauv.h"
#include <stdio.h>
#include <unistd.h>

void tarefa1(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 1: ' .. os.date())");
}

void tarefa2(lua_State* L) {
    luaL_dostring(L, "print('Tarefa 2: Calculando...')");
    luaL_dostring(L, "local s = 0; for i=1,1000000 do s = s + i end; print('Soma:', s)");
}

int main() {
    luauv_t uv;
    luauv_init(&uv);
    luauv_start(&uv);
    
    printf("Enviando tarefas...\n");
    
    luauv_post(&uv, tarefa1);
    luauv_post(&uv, tarefa2);
    luauv_post(&uv, tarefa1);
    
    sleep(1); // Aguardar execução
    
    luauv_stop(&uv);
    luauv_destroy(&uv);
    
    return 0;
}
```

Exemplo 2: Estado persistente

```c
void inicializar_estado(lua_State* L) {
    // Criar tabela global compartilhada
    luaL_dostring(L, 
        "compartilhado = {"
        "   contador = 0,"
        "   dados = {}"
        "}"
    );
}

void incrementar_contador(lua_State* L) {
    luaL_dostring(L,
        "compartilhado.contador = compartilhado.contador + 1;"
        "print('Contador:', compartilhado.contador);"
    );
}

// Uso: luauv_post(&uv, incrementar_contador);
```

🧪 Testes

Teste básico (tests/test_basic.c)

```c
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
```

Compilar e executar:

```bash
./compile tests/test_basic.c -o test_basic
./test_basic
# Saída esperada:
# Executando tarefa Lua!
```

⚠️ Limitações

· Deadlock prevention: Não chamar luauv_post de dentro da thread Lua
· Tarefas bloqueantes: A thread principal espera a conclusão da tarefa via condition variable
· Single-threaded Lua: O estado Lua é usado apenas na thread dedicada
· Sem suporte a múltiplos estados: Uma instância gerencia um único estado Lua

🤝 Contribuindo

Contribuições são bem-vindas! Por favor, siga estes passos:

1. Fork o projeto
2. Crie uma branch (git checkout -b feature/nova-feature)
3. Commit suas mudanças (git commit -am 'Adiciona nova feature')
4. Push para a branch (git push origin feature/nova-feature)
5. Abra um Pull Request

📄 Licença

Este projeto está licenciado sob a licença MIT - veja o arquivo LICENSE para detalhes.

✉️ Contato

· Issues: GitHub Issues
· Email: seu-email@exemplo.com

---

Tags: lua, libuv, async, threading, c, event-loop
