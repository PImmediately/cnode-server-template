set(CMAKE_SYSTEM_NAME Generic)

cmake_minimum_required(VERSION 3.16)

project(cnode-server-template)
include_directories(..)

set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "em++")

set(WASM_FLAGS 
	"-s" "WASM=1"
	"-s" "FILESYSTEM=0"
	"-s" "EXPORTED_FUNCTIONS=['_main','_malloc','_free']"
	"-s" "DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=$stringToNewUTF8"
)

add_custom_target(server
	COMMAND emcc
	${CMAKE_SOURCE_DIR}/Sources/Server/Main.cpp
	${CMAKE_SOURCE_DIR}/Sources/Server/Server.cpp
	${CMAKE_SOURCE_DIR}/Sources/Server/WebSocket/WebSocketServerHandler.cpp
	${CMAKE_SOURCE_DIR}/Sources/Server/WebSocket/WebSocketClientHandler.cpp

	-s ENVIRONMENT='node'
	-s MODULARIZE=1
	${WASM_FLAGS}
	--pre-js=${CMAKE_SOURCE_DIR}/Sources/Server/Main.pre.js
	-o ${CMAKE_SOURCE_DIR}/build/server_cpp.js
)

add_custom_target(client
	COMMAND emcc
	${CMAKE_SOURCE_DIR}/Sources/Client/Main.cpp
	${CMAKE_SOURCE_DIR}/Sources/Client/Client.cpp
	${CMAKE_SOURCE_DIR}/Sources/Client/WebSocket/WebSocketClientHandler.cpp

	-s ENVIRONMENT='web'
	${WASM_FLAGS}
	--pre-js=${CMAKE_SOURCE_DIR}/Sources/Client/Main.pre.js
	-o ${CMAKE_SOURCE_DIR}/build/client_cpp.js
)

add_dependencies(server client)