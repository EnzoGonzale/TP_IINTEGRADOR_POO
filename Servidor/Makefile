# --- Variables ---
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -I$(LIBRARY_DIR)
LDFLAGS_SERVER = -lxmlrpc_server_abyss++ -lxmlrpc_server++ -lxmlrpc_client++ -lxmlrpc++ -lxmlrpc_abyss -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_util -lsqlite3
LDFLAGS_CLIENT = -lxmlrpc_client++ -lxmlrpc++ -lxmlrpc_xmlparse -lxmlrpc_xmltok -lxmlrpc_util -lsqlite3

SRC_DIR = src
LIBRARY_DIR = library
TEST_DIR = test
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# --- Archivos Fuente y Objeto ---
# Fuentes compartidas por ambos ejecutables (excluyendo los 'main')
SHARED_SOURCES = $(wildcard $(INCLUDE_DIR)/*.cpp)
SHARED_OBJECTS = $(patsubst $(INCLUDE_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SHARED_SOURCES))

# Fuentes y objetos específicos para el servidor
SERVER_MAIN_SRC = $(SRC_DIR)/mainServer.cpp
SERVER_MAIN_OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SERVER_MAIN_SRC))

# Fuentes y objetos específicos para el cliente
CLIENT_MAIN_SRC = $(SRC_DIR)/cli_main.cpp
CLIENT_MAIN_OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CLIENT_MAIN_SRC))

# Fuentes y objetos para los tests de CLI
CLI_TEST_SRC = $(TEST_DIR)/cli_handler_test.cpp
CLI_TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(CLI_TEST_SRC))

# Fuentes y objetos para los tests de Serial
SERIAL_TEST_SRC = $(TEST_DIR)/serial_comunicator_test.cpp
SERIAL_TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SERIAL_TEST_SRC))

# Fuentes y objetos para el test de array RPC
ARRAY_RPC_TEST_SRC = $(TEST_DIR)/array_rpc_test.cpp
ARRAY_RPC_TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(ARRAY_RPC_TEST_SRC))

# Fuentes y objetos para el test de estado del Arduino
STATUS_TEST_SRC = $(TEST_DIR)/status_arduino_test.cpp
STATUS_TEST_OBJ = $(patsubst $(TEST_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(STATUS_TEST_SRC))


# --- Reglas de Compilación ---
all:
	mkdir -p $(BIN_DIR) $(OBJ_DIR)
	$(MAKE) $(BIN_DIR)/cli_main
	$(MAKE) $(BIN_DIR)/mainServer
	$(MAKE) $(BIN_DIR)/serial_tests
	$(MAKE) $(BIN_DIR)/array_rpc_test
	$(MAKE) $(BIN_DIR)/status_arduino_test

# Regla para enlazar el servidor
$(BIN_DIR)/mainServer: $(SHARED_OBJECTS) $(SERVER_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_SERVER)

# Regla para enlazar el cliente
$(BIN_DIR)/cli_main: $(filter-out $(OBJ_DIR)/Server.o $(OBJ_DIR)/RpcServiceHandler.o, $(SHARED_OBJECTS)) $(CLIENT_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

# Regla genérica para compilar archivos .cpp a .o
$(OBJ_DIR)/%.o: $(INCLUDE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para compilar y enlazar los tests de CLI
$(BIN_DIR)/cli_tests: $(filter-out $(OBJ_DIR)/Server.o $(OBJ_DIR)/RpcServiceHandler.o, $(SHARED_OBJECTS)) $(CLI_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

$(BIN_DIR)/serial_tests: $(OBJ_DIR)/SerialComunicator.o $(OBJ_DIR)/SerialPortConfiguration.o $(SERIAL_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Regla para compilar y enlazar el test de array RPC
$(BIN_DIR)/array_rpc_test: $(ARRAY_RPC_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

# Regla para compilar y enlazar el test de estado del Arduino
$(BIN_DIR)/status_arduino_test: $(OBJ_DIR)/Robot.o $(OBJ_DIR)/GCode.o $(OBJ_DIR)/SerialComunicator.o $(OBJ_DIR)/SerialPortConfiguration.o $(STATUS_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS_CLIENT)

# Inicia el servidor XML-RPC
run:
	sudo ./$(BIN_DIR)/mainServer

# Ejecuta el modo interactivo
# Ahora, 'interactive' ejecuta un cliente CLI separado que se conecta al servidor RPC.
interactive:
	sudo ./$(BIN_DIR)/cli_main

# Ejecuta los tests de CLI
test: $(BIN_DIR)/cli_tests
	./$(BIN_DIR)/cli_tests

# Ejecuta los tests de comunicación serial (requiere hardware y sudo)
serial_test: $(BIN_DIR)/serial_tests
	sudo ./$(BIN_DIR)/serial_tests

# Ejecuta el test de array RPC (requiere que el servidor esté corriendo)
array_rpc_test: $(BIN_DIR)/array_rpc_test
	./$(BIN_DIR)/array_rpc_test

# Ejecuta el test de parseo de estado del Arduino
status_test: $(BIN_DIR)/status_arduino_test
	./$(BIN_DIR)/status_arduino_test

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)
	rm -f application.csv # Limpiamos el archivo de log generado por el Logger en los tests
