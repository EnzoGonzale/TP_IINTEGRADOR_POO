# --- Variables ---
CXX = g++

# --- Directorios del Proyecto ---
SERVER_DIR = server
CLIENT_DIR = client
LIBRARY_DIR = library
TEST_DIR = test
OBJ_DIR = obj
BIN_DIR = bin

# --- Library specific directories ---
Bcrypt_DIR = $(LIBRARY_DIR)/Bcrypt
Json_DIR = $(LIBRARY_DIR)/Json
DocTest_DIR = $(LIBRARY_DIR)/DocTest
SQLITE_DIR = $(LIBRARY_DIR)/sqlite3
XMLRPC_DIR = $(LIBRARY_DIR)/xmlrpc-c

# --- Flags de Compilación y Enlazado ---
CXXFLAGS = -std=c++17 \
           -I$(SERVER_DIR)/include \
           -I$(CLIENT_DIR)/include \
           -I$(Bcrypt_DIR)/include \
           -I$(Json_DIR) \
           -I$(DocTest_DIR) \
           -I$(SQLITE_DIR)/include \
           -I$(XMLRPC_DIR)/include

# --- Bibliotecas Estáticas de XML-RPC ---
# El orden es crucial: de más alto nivel (más dependiente) a más bajo nivel (menos dependiente).
# Las bibliotecas C++ dependen de las bibliotecas C.

# Bibliotecas C++
XMLRPC_LIBS_SERVER_CPP = \
    $(XMLRPC_DIR)/lib/libxmlrpc_server_abyss++.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_server++.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_client++.a \
    $(XMLRPC_DIR)/lib/libxmlrpc++.a

XMLRPC_LIBS_CLIENT_CPP = \
    $(XMLRPC_DIR)/lib/libxmlrpc_client++.a \
    $(XMLRPC_DIR)/lib/libxmlrpc++.a

# Bibliotecas C (dependencias de las de C++)
XMLRPC_LIBS_C = \
    $(XMLRPC_DIR)/lib/libxmlrpc_server_abyss.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_server.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_client.a \
    $(XMLRPC_DIR)/lib/libxmlrpc.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_xmlparse.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_xmltok.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_util.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_abyss.a \
    $(XMLRPC_DIR)/lib/libxmlrpc_util.a

XMLRPC_LIBS_CLIENT_SPECIFIC = \
    $(XMLRPC_DIR)/lib/libxmlrpc_client++.a

# SQLite3 static library
SQLite3_LIB_STATIC = $(SQLITE_DIR)/lib/libsqlite3.a

# Dependencias de sistema comunes para XML-RPC (y otras librerías si las hubiera)
SYSTEM_DEPS = -lpthread -lcurl

LDFLAGS_SERVER = $(XMLRPC_LIBS_SERVER_CPP) $(XMLRPC_LIBS_C) $(SQLite3_LIB_STATIC) $(SYSTEM_DEPS)
LDFLAGS_CLIENT = $(XMLRPC_LIBS_CLIENT_CPP) $(XMLRPC_LIBS_C) $(SQLite3_LIB_STATIC) $(SYSTEM_DEPS)

# --- Bcrypt sources / objects ---
Bcrypt_SOURCES = $(wildcard $(Bcrypt_DIR)/src/*.cpp)
Bcrypt_OBJECTS = $(patsubst $(Bcrypt_DIR)/src/%.cpp, $(OBJ_DIR)/%.o, $(Bcrypt_SOURCES))

# --- Archivos Fuente y Objeto (Nueva Estructura) ---
# Fuentes y objetos del Servidor
# Fuentes y objetos del Cliente
CLIENT_SOURCES = $(wildcard $(CLIENT_DIR)/src/*.cpp)
CLIENT_OBJECTS = $(patsubst $(CLIENT_DIR)/src/%.cpp, $(OBJ_DIR)/%.o, $(CLIENT_SOURCES))

# Fuentes y objetos del Servidor (excluyendo mainServer.cpp para tests)
SERVER_SOURCES = $(wildcard $(SERVER_DIR)/src/*.cpp)
SERVER_OBJECTS = $(patsubst $(SERVER_DIR)/src/%.cpp, $(OBJ_DIR)/%.o, $(SERVER_SOURCES))

# Fuentes y objetos para los Tests
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SOURCES))

# --- Reglas de Compilación ---
all:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)
	$(MAKE) $(BIN_DIR)/mainServer
	$(MAKE) $(BIN_DIR)/cli_main
	$(MAKE) $(BIN_DIR)/serial_comunicator_test
	$(MAKE) $(BIN_DIR)/array_rpc_test
	$(MAKE) $(BIN_DIR)/status_arduino_test

# Regla para enlazar el servidor (depende de todos los objetos del servidor)
$(BIN_DIR)/mainServer: $(SERVER_OBJECTS) $(Bcrypt_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_SERVER)

# Regla para enlazar el cliente (depende de los objetos del cliente y algunos compartidos del servidor)
$(BIN_DIR)/cli_main: $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

# Regla para enlazar el test de SerialComunicator
$(BIN_DIR)/serial_comunicator_test: $(OBJ_DIR)/serial_comunicator_test.o $(OBJ_DIR)/SerialComunicator.o $(OBJ_DIR)/SerialPortConfiguration.o $(OBJ_DIR)/Logger.o $(OBJ_DIR)/FileManager.o $(OBJ_DIR)/GCode.o $(OBJ_DIR)/User.o $(Bcrypt_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_SERVER)

# Regla para enlazar el test de ArrayRPC
$(BIN_DIR)/array_rpc_test: $(OBJ_DIR)/array_rpc_test.o $(filter-out $(OBJ_DIR)/mainServer.o, $(SERVER_OBJECTS)) $(Bcrypt_OBJECTS) $(OBJ_DIR)/Logger.o $(OBJ_DIR)/FileManager.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_SERVER)

# Regla para enlazar el test de StatusArduino
$(BIN_DIR)/status_arduino_test: $(OBJ_DIR)/status_arduino_test.o $(OBJ_DIR)/Robot.o $(OBJ_DIR)/SerialComunicator.o $(OBJ_DIR)/SerialPortConfiguration.o $(OBJ_DIR)/GCode.o $(OBJ_DIR)/User.o $(Bcrypt_OBJECTS) $(OBJ_DIR)/Logger.o $(OBJ_DIR)/FileManager.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_SERVER)

# Regla genérica para compilar archivos .cpp a .o
$(OBJ_DIR)/%.o: $(SERVER_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(CLIENT_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para compilar fuentes de bcrypt a objetos
$(OBJ_DIR)/%.o: $(Bcrypt_DIR)/src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Visualiza el archivo de log generado por el Logger
visualize:
	tail -f application.csv

# Ejecuta el modo interactivo
# Ahora, 'interactive' ejecuta un cliente CLI separado que se conecta al servidor RPC.
run_client:
	sudo ./$(BIN_DIR)/cli_main

# Inicia el servidor XML-RPC
run_server:
	sudo ./$(BIN_DIR)/mainServer

# Evaluacion de tests
test_comunicator:
	sudo ./$(BIN_DIR)/serial_comunicator_test

test_array_rpc:
	sudo ./$(BIN_DIR)/array_rpc_test

test_status_arduino:
	sudo ./$(BIN_DIR)/status_arduino_test

# Limpia los archivos binarios y objetos generados
clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	rm -f application.csv # Limpiamos el archivo de log generado por el Logger en los tests

.PHONY: all clean run_server run_client visualize
