cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wolfssl-tls-hello-server)

# Check if the symbolic link __repo__ exists
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/__repo__")
    # Create a symbolic link from __repo__ to the ZEPHYR_BASE environment variable
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink $ENV{ZEPHYR_BASE}/.. ${CMAKE_SOURCE_DIR}/__repo__
        COMMAND_ECHO STDOUT
    )
endif()

target_sources(app PRIVATE src/main.c)
target_sources(app PRIVATE src/system_init_ns.c)
target_sources(app PRIVATE __repo__/modules/crypto/wolfssl/wolfcrypt/test/test.c)
target_sources(app PRIVATE __repo__/modules/crypto/wolfssl/wolfcrypt/benchmark/benchmark.c)
target_include_directories(app PRIVATE src)

# wolfMQTT
target_sources(app PRIVATE src/mqttClient/fwclient.c)
target_sources(app PRIVATE src/mqttClient/mqttexample.c)
target_sources(app PRIVATE src/mqttClient/mqttnet.c)
target_sources(app PRIVATE src/mqttClient/mqttport.c)
target_include_directories(app PRIVATE src/mqttClient)
# Enable wolfMQTT user settings to adjust the configuration as same as wolfSSL
target_compile_definitions(app PRIVATE WOLFMQTT_USER_SETTINGS)

# wolfBoot (libwolfboot)
set(WOLFBOOT_ROOT "/Users/yshimizu/work/wolfBoot" CACHE PATH "Path to wolfBoot checkout")
set(WOLFBOOT_NSC_OBJECT "${WOLFBOOT_ROOT}/src/wolfboot_tz_nsc.o" CACHE FILEPATH "Path to wolfBoot NSC prebuilt object")

if(NOT EXISTS "${WOLFBOOT_NSC_OBJECT}")
    message(FATAL_ERROR "wolfBoot NSC object not found: ${WOLFBOOT_NSC_OBJECT}")
endif()

if(NOT EXISTS "${WOLFBOOT_ROOT}/include/wolfboot/wolfboot.h")
    message(FATAL_ERROR "wolfBoot headers not found under: ${WOLFBOOT_ROOT}/include")
endif()

target_link_libraries(app PRIVATE "${WOLFBOOT_NSC_OBJECT}")
target_include_directories(app PRIVATE "${WOLFBOOT_ROOT}/include")
target_compile_definitions(app PRIVATE TZEN)

add_definitions(-DWOLFSSL_USER_SETTINGS)


