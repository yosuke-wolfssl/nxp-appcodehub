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
target_sources(app PRIVATE __repo__/modules/lib/wolfboot/src/libwolfboot.c)
target_include_directories(app PRIVATE __repo__/modules/lib/wolfboot/include)
target_sources(app PRIVATE src/hal/hal_zephyr.c)
target_include_directories(app PRIVATE src/hal)

add_definitions(-DWOLFSSL_USER_SETTINGS)


