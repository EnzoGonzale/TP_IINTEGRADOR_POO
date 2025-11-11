 # Proyecto Servidor RPC de Robot

 Este proyecto implementa un servidor XML-RPC para controlar un brazo robótico, junto con un cliente de línea de comandos (CLI) para interactuar con él.

 ## Requisitos de Compilación

 Antes de compilar, asegúrate de tener instaladas las siguientes dependencias en tu sistema (los comandos son para Debian/Ubuntu):

 1.  **Compilador de C++ y herramientas de construcción (`build-essential`):**
     Necesitarás un compilador de C++ que soporte C++17 (como g++) y la herramienta `make`.
     ```bash
     sudo apt update
     sudo apt install build-essential g++ make
     ```

 2.  **Librería de desarrollo de cURL (`libcurl`):**
     Es una dependencia de la librería `xmlrpc-c` para la comunicación de red del cliente.
     ```bash
     sudo apt install libcurl4-openssl-dev
     ```

 ## Cómo Compilar

 Una vez que tengas todos los requisitos, simplemente ejecuta `make` en el directorio raíz del proyecto:

 ```bash
 make all
 ```

 Esto generará los siguientes ejecutables en el directorio `bin/`:
 *   `mainServer`: El servidor RPC.
 *   `cli_main`: El cliente de línea de comandos.
 *   Varios ejecutables de test.

 ## Cómo Ejecutar

 1.  **Iniciar el servidor:**
     ```bash
     make run_server
     ```

 2.  **Ejecutar el cliente en otra terminal:**
     ```bash
     make run_client
     ```

 3.  **Ejecutar los tests con:**
     ```bash
     make test_comunicator
     make test_array_rpc
     make test_status_arduino
     ```