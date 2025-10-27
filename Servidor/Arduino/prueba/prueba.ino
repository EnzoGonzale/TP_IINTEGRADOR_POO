/*
  prueba.ino
  Sketch de prueba para el servidor del brazo robótico.

  Este sketch escucha comandos a través del puerto serie (a 115200 baudios)
  y responde con un mensaje descriptivo seguido de "OK\n".
  Está diseñado para funcionar con la lógica de `sendCommandAndWaitForResponse`
  del servidor C++.
*/

String inputString = "";      // Una cadena para almacenar los datos entrantes
bool stringComplete = false;  // Una bandera para indicar si se ha recibido una línea completa

void setup() {
  // Inicializar el puerto serie a la misma velocidad que el servidor C++
  Serial.begin(115200);

  // Limpiar cualquier dato residual en el buffer de entrada
  while (Serial.available() > 0) {
    Serial.read();
  }
  
  inputString.reserve(200); // Reservar memoria para la cadena de entrada
  
  // Enviar un mensaje de bienvenida para confirmar que el sketch se ha iniciado
  Serial.println("Arduino Test Sketch Ready.");
  Serial.println("OK");
}

void loop() {
  // Si se ha recibido una línea completa, procesarla
  if (stringComplete) {
    inputString.trim(); // Eliminar espacios en blanco y saltos de línea
    processCommand(inputString);

    // Limpiar la cadena para el próximo comando
    inputString = "";
    stringComplete = false;
  }
}

// La función serialEvent se ejecuta automáticamente cuando hay datos disponibles en el puerto serie.
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    // Si el carácter es un salto de línea, hemos recibido un comando completo.
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void processCommand(String command) {
  if (command.equals("M17"))      { Serial.println("Motores ACTIVADOS"); }
  else if (command.equals("M18")) { Serial.println("Motores DESACTIVADOS"); }
  else if (command.equals("M3"))  { Serial.println("Efector ACTIVADO"); }
  else if (command.equals("M5"))  { Serial.println("Efector DESACTIVADO"); }
  else if (command.startsWith("G1")) { Serial.print("Moviendo a: " + command); }
  else if (command.equals("G24")) { Serial.println("Moviendo a ORIGEN"); }
  else { Serial.print("Comando no reconocido: " + command); }

  // Enviar "OK" para indicar al servidor que el comando ha finalizado.
  Serial.println("OK");
}