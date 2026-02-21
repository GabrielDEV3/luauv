#include "luauv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void luauv_async_cb(uv_async_t* handle) {
    luauv_t* self = (luauv_t*)handle->data;
    void (*local)(lua_State*) = NULL;
    
    pthread_mutex_lock(self->mtx);
    local = self->task;
    pthread_mutex_unlock(self->mtx);
    
    if (local && self->L) {
        local(self->L);
        
        pthread_mutex_lock(self->mtx);
        self->task = NULL;
        pthread_mutex_unlock(self->mtx);
        
        pthread_cond_signal(self->cv);
    }
}

void luauv_idle_cb(uv_idle_t* handle) {
    luauv_t* self = (luauv_t*)handle->data;
    if (self->L) {
        lua_gc(self->L, LUA_GCSTEP, 200);
    }
}

void* luauv_loop_cb(void* arg) {
    luauv_t* self = (luauv_t*)arg;
    
    self->lua_thread_id = pthread_self();
    
    self->mtx = malloc(sizeof(pthread_mutex_t));
    self->cv = malloc(sizeof(pthread_cond_t));
    self->loop = malloc(sizeof(uv_loop_t));
    self->async = malloc(sizeof(uv_async_t));
    self->idle = malloc(sizeof(uv_idle_t));
    
    pthread_mutex_init(self->mtx, NULL);
    pthread_cond_init(self->cv, NULL);
    uv_loop_init(self->loop);
    
    self->L = luaL_newstate();
    if (!self->L) return NULL;
    luaL_openlibs(self->L);
    lua_gc(self->L, LUA_GCSTOP, 0);
    
    self->async->data = self;
    uv_async_init(self->loop, self->async, luauv_async_cb);
    
    self->idle->data = self;
    uv_idle_init(self->loop, self->idle);
    uv_idle_start(self->idle, luauv_idle_cb);
    
    uv_run(self->loop, UV_RUN_DEFAULT);
    
    uv_idle_stop(self->idle);
    uv_close((uv_handle_t*)self->async, NULL);
    uv_close((uv_handle_t*)self->idle, NULL);
    
    uv_run(self->loop, UV_RUN_DEFAULT);
    uv_loop_close(self->loop);
    
    if (self->L) {
        lua_close(self->L);
        self->L = NULL;
    }
    
    pthread_mutex_destroy(self->mtx);
    pthread_cond_destroy(self->cv);
    
    free(self->loop);
    free(self->async);
    free(self->idle);
    free(self->mtx);
    free(self->cv);
    
    self->loop = NULL;
    self->async = NULL;
    self->idle = NULL;
    self->mtx = NULL;
    self->cv = NULL;
    
    return NULL;
}

int luauv_init(luauv_t* self) {
    memset(self, 0, sizeof(luauv_t));
    return 0;
}

void luauv_destroy(luauv_t* self) {
    luauv_stop(self);
}

int luauv_start(luauv_t* self) {
    if (self->thread) return 0;
    return pthread_create(&self->thread, NULL, luauv_loop_cb, self) == 0;
}

void luauv_stop(luauv_t* self) {
    if (!self->thread) return;
    
    if (self->loop) uv_stop(self->loop);
    if (self->async) uv_async_send(self->async);
    
    pthread_join(self->thread, NULL);
    self->thread = 0;
}

void luauv_post(luauv_t* self, void (*func)(lua_State*)) {
    if (!self->thread || !self->mtx) return;
    
    pthread_t current = pthread_self();
    if (pthread_equal(current, self->lua_thread_id)) {
        fprintf(stderr, "Deadlock detected\n");
        return;
    }
    
    pthread_mutex_lock(self->mtx);
    
    while (self->task != NULL) {
        pthread_cond_wait(self->cv, self->mtx);
    }
    
    self->task = func;
    
    uv_async_send(self->async);
    
    while (self->task != NULL) {
        pthread_cond_wait(self->cv, self->mtx);
    }
    
    pthread_mutex_unlock(self->mtx);
}
