#!/usr/bin/env python3
"""
Script interactivo para probar el login al servidor RPC.
Solicita credenciales, intenta autenticarse, muestra el rol del usuario,
y finalmente cierra la sesión.
"""

import sys
import getpass
import xmlrpc.client
from enum import Enum

# --- Definiciones que antes estaban en archivos separados ---

RPC_ENDPOINT = "http://localhost:8080/RPC2"

class UserRole(Enum):
    """Enumera los roles de usuario posibles."""
    OPERATOR = 0
    ADMIN = 1

class RobotRpcClient:
    """
    Cliente RPC autocontenido para interactuar con el servidor del robot.
    """
    def __init__(self, uri=RPC_ENDPOINT):
        self.proxy = xmlrpc.client.ServerProxy(uri, verbose=False)
        self._session_token = None
        self.user_role = None

    def login(self, username, password):
        """Autentica al usuario y guarda el token de sesión."""
        try:
            result = self.proxy.user.login(username, password)
            if isinstance(result, dict):
                self._session_token = result.get("token")
                self.user_role = UserRole(result.get("role"))
                if self._session_token:
                    return True
        except xmlrpc.client.Fault:
            # El servidor lanza una excepción si las credenciales son incorrectas
            return False
        return False

    def logout(self):
        """Cierra la sesión en el servidor."""
        if self._session_token:
            self.proxy.user.logout(self._session_token)
            self._session_token = None
            self.user_role = None

def main():
    """
    Función principal para ejecutar la prueba de login interactiva.
    """
    print("="*60)
    print("PRUEBA DE LOGIN INTERACTIVO")
    print("="*60)
    print(f"Conectando al servidor: {RPC_ENDPOINT}\n")

    # Solicitar credenciales al usuario
    username = input("Ingrese su nombre de usuario: ")
    password = getpass.getpass("Ingrese su contraseña: ")

    client = RobotRpcClient()
    logged_in = False

    try:
        # Intentar hacer login
        if client.login(username, password):
            logged_in = True
            role_str = "Administrador" if client.user_role == UserRole.ADMIN else "Operador"
            print(f"\n✓ ¡Login exitoso! Bienvenido, {username}.")
            print(f"  Rol: {role_str}")
            # El token de sesión se gestiona internamente en el objeto 'client'
            # para las siguientes llamadas.
            # Aquí podrías añadir más llamadas a métodos del robot que requieran autenticación.
        else:
            print("\n✗ Login falló. Usuario o contraseña incorrectos.")
            return 1 # Salir con código de error

    except xmlrpc.client.Fault as f:
        print(f"\n✗ Error RPC: {f.faultString}")
        return 1
    except (ConnectionRefusedError, OSError) as e:
        print(f"\n✗ Error de conexión: {e}")
        print("  Asegúrese de que el servidor C++ esté corriendo.")
        return 1
    except Exception as e:
        print(f"\n✗ Error inesperado: {type(e).__name__}: {e}")
        return 1
    finally:
        # Asegurarse de hacer logout si el login fue exitoso
        if logged_in:
            print("\nCerrando sesión...")
            client.logout()
            print("✓ Logout exitoso.")

if __name__ == "__main__":
    sys.exit(main())
