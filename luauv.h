#ifndef LUAUV_H
#define LUAUV_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <uv.h>
#include <pthread.h>

typedef struct luauv_s {
    lua_State* L;
    uv_loop_t* loop;
    uv_async_t* async;
    uv_idle_t* idle;
    
    pthread_t thread;
    pthread_t lua_thread_id;
    pthread_mutex_t* mtx;
    pthread_cond_t* cv;
    void (*task)(lua_State*);
} luauv_t;

void luauv_async_cb(uv_async_t* handle);
void luauv_idle_cb(uv_idle_t* handle);
void* luauv_loop_cb(void* arg);

int luauv_init(luauv_t* self);
void luauv_destroy(luauv_t* self);
int luauv_start(luauv_t* self);
void luauv_stop(luauv_t* self);
void luauv_post(luauv_t* self, void (*func)(lua_State*));

#endif
