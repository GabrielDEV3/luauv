cmake_minimum_required(VERSION 3.10)
project(luauv VERSION 0.1.0 LANGUAGES C)

# Arquivo de propriedades (obrigatório)
set(PROPERTIES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/build.properties")
if(NOT EXISTS ${PROPERTIES_FILE})
    message(FATAL_ERROR "build.properties not found. Please create it.")
endif()

# Carregar propriedades
message(STATUS "Loading properties from ${PROPERTIES_FILE}")
file(STRINGS ${PROPERTIES_FILE} PROPERTIES)
foreach(line ${PROPERTIES})
    if(NOT line MATCHES "^#")
        string(REGEX REPLACE "=" ";" line_split ${line})
        list(GET line_split 0 key)
        string(STRIP ${key} key)
        list(GET line_split 1 value)
        string(STRIP ${value} value)
        set(${key} ${value})
        message(STATUS "  ${key} = ${value}")
    endif()
endforeach()

# Verificar propriedades obrigatórias
set(REQUIRED_VARS
    LUA_DIR
    UV_DIR
)

foreach(var ${REQUIRED_VARS})
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "${var} not defined in build.properties")
    endif()
endforeach()

# Definir includes baseados nos diretórios
set(LUA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${LUA_DIR}/src")
set(LUA_INCLUDE_DIR "${LUA_SRC_DIR}")
set(UV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${UV_DIR}/include")
set(UV_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${UV_DIR}/src")
set(UV_UNIX_DIR "${UV_SRC_DIR}/unix")

message(STATUS "LUA_SRC_DIR: ${LUA_SRC_DIR}")
message(STATUS "LUA_INCLUDE_DIR: ${LUA_INCLUDE_DIR}")
message(STATUS "UV_INCLUDE_DIR: ${UV_INCLUDE_DIR}")
message(STATUS "UV_SRC_DIR: ${UV_SRC_DIR}")

# Verificar se os diretórios existem
if(NOT EXISTS ${LUA_INCLUDE_DIR})
    message(FATAL_ERROR "Lua include directory not found: ${LUA_INCLUDE_DIR}")
endif()
if(NOT EXISTS ${UV_INCLUDE_DIR})
    message(FATAL_ERROR "UV include directory not found: ${UV_INCLUDE_DIR}")
endif()

# Opções
option(INSTALL_HEADERS "Install headers" ON)

# Configurações de saída
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -fPIC -D_GNU_SOURCE ${CFLAGS_EXTRA}")

# Biblioteca Lua - lista manual de arquivos (excluindo lua.c e luac.c)
set(LUA_SOURCES
    ${LUA_SRC_DIR}/lapi.c
    ${LUA_SRC_DIR}/lauxlib.c
    ${LUA_SRC_DIR}/lbaselib.c
    ${LUA_SRC_DIR}/lcode.c
    ${LUA_SRC_DIR}/lcorolib.c
    ${LUA_SRC_DIR}/lctype.c
    ${LUA_SRC_DIR}/ldblib.c
    ${LUA_SRC_DIR}/ldebug.c
    ${LUA_SRC_DIR}/ldo.c
    ${LUA_SRC_DIR}/ldump.c
    ${LUA_SRC_DIR}/lfunc.c
    ${LUA_SRC_DIR}/lgc.c
    ${LUA_SRC_DIR}/linit.c
    ${LUA_SRC_DIR}/liolib.c
    ${LUA_SRC_DIR}/llex.c
    ${LUA_SRC_DIR}/lmathlib.c
    ${LUA_SRC_DIR}/lmem.c
    ${LUA_SRC_DIR}/loadlib.c
    ${LUA_SRC_DIR}/lobject.c
    ${LUA_SRC_DIR}/lopcodes.c
    ${LUA_SRC_DIR}/loslib.c
    ${LUA_SRC_DIR}/lparser.c
    ${LUA_SRC_DIR}/lstate.c
    ${LUA_SRC_DIR}/lstring.c
    ${LUA_SRC_DIR}/lstrlib.c
    ${LUA_SRC_DIR}/ltable.c
    ${LUA_SRC_DIR}/ltablib.c
    ${LUA_SRC_DIR}/ltm.c
    ${LUA_SRC_DIR}/lundump.c
    ${LUA_SRC_DIR}/lutf8lib.c
    ${LUA_SRC_DIR}/lvm.c
    ${LUA_SRC_DIR}/lzio.c
)

add_library(lua_static STATIC ${LUA_SOURCES})
target_include_directories(lua_static PRIVATE 
    ${LUA_INCLUDE_DIR}
)
set_target_properties(lua_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Biblioteca libuv - APENAS ARQUIVOS UNIX/LINUX (sem Windows)
set(UV_SOURCES
    # Arquivos comuns
    ${UV_SRC_DIR}/fs-poll.c
    ${UV_SRC_DIR}/idna.c
    ${UV_SRC_DIR}/inet.c
    ${UV_SRC_DIR}/random.c
    ${UV_SRC_DIR}/strscpy.c
    ${UV_SRC_DIR}/strtok.c
    ${UV_SRC_DIR}/thread-common.c
    ${UV_SRC_DIR}/threadpool.c
    ${UV_SRC_DIR}/timer.c
    ${UV_SRC_DIR}/uv-common.c
    ${UV_SRC_DIR}/uv-data-getter-setters.c
    ${UV_SRC_DIR}/version.c
    
    # Arquivos Unix/Linux
    ${UV_UNIX_DIR}/async.c
    ${UV_UNIX_DIR}/core.c
    ${UV_UNIX_DIR}/dl.c
    ${UV_UNIX_DIR}/fs.c
    ${UV_UNIX_DIR}/getaddrinfo.c
    ${UV_UNIX_DIR}/getnameinfo.c
    ${UV_UNIX_DIR}/linux.c
    ${UV_UNIX_DIR}/loop.c
    ${UV_UNIX_DIR}/loop-watcher.c
    ${UV_UNIX_DIR}/pipe.c
    ${UV_UNIX_DIR}/poll.c
    ${UV_UNIX_DIR}/process.c
    ${UV_UNIX_DIR}/random-devurandom.c
    ${UV_UNIX_DIR}/random-getrandom.c
    ${UV_UNIX_DIR}/random-sysctl-linux.c
    ${UV_UNIX_DIR}/signal.c
    ${UV_UNIX_DIR}/stream.c
    ${UV_UNIX_DIR}/tcp.c
    ${UV_UNIX_DIR}/thread.c
    ${UV_UNIX_DIR}/tty.c
    ${UV_UNIX_DIR}/udp.c
    ${UV_UNIX_DIR}/proctitle.c
    ${UV_UNIX_DIR}/posix-hrtime.c
    ${UV_UNIX_DIR}/sysinfo-loadavg.c
    ${UV_UNIX_DIR}/sysinfo-memory.c
)

add_library(uv_static STATIC ${UV_SOURCES})
target_include_directories(uv_static PRIVATE
    ${UV_INCLUDE_DIR}
    ${UV_SRC_DIR}
    ${UV_UNIX_DIR}
)
set_target_properties(uv_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Biblioteca principal luauv
add_library(luauv SHARED luauv.c)

target_include_directories(luauv PRIVATE
    ${LUA_INCLUDE_DIR}
    ${UV_INCLUDE_DIR}
    ${UV_SRC_DIR}
    ${UV_UNIX_DIR}
)

# Verificar se o lua.h existe
if(EXISTS ${LUA_INCLUDE_DIR}/lua.h)
    message(STATUS "Found lua.h in ${LUA_INCLUDE_DIR}")
else()
    message(FATAL_ERROR "lua.h not found in ${LUA_INCLUDE_DIR}")
endif()

target_link_libraries(luauv lua_static uv_static pthread dl m ${LDFLAGS_EXTRA})
set_target_properties(luauv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ============= COPIAR HEADERS DURANTE O BUILD =============
set(LUAV_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include/luauv")

# Target para copiar headers
add_custom_target(copy_headers ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LUAV_INCLUDE_DIR}/lua
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LUAV_INCLUDE_DIR}/uv
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/luauv.h ${LUAV_INCLUDE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${LUA_INCLUDE_DIR} ${LUAV_INCLUDE_DIR}/lua
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${UV_INCLUDE_DIR} ${LUAV_INCLUDE_DIR}/uv
    COMMENT "Copying headers to ${LUAV_INCLUDE_DIR}"
)

# Dependência para copiar headers antes do luauv
add_dependencies(luauv copy_headers)

# Opcional: criar link simbólico para versão sem soversion
add_custom_command(TARGET luauv POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "libluauv.so.${PROJECT_VERSION}"
        "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libluauv.so"
    COMMENT "Creating symlink libluauv.so -> libluauv.so.${PROJECT_VERSION}"
)

# Instalação
install(TARGETS luauv
    LIBRARY DESTINATION lib
)

install(DIRECTORY ${LUAV_INCLUDE_DIR}/
    DESTINATION include/luauv
)

message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Library will be installed to: lib/")
message(STATUS "Headers will be copied to: ${LUAV_INCLUDE_DIR} during build")
message(STATUS "Headers will be installed to: include/luauv/")