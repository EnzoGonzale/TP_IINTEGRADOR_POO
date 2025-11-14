GUI client for robot RPC

This folder contains a simple Tkinter GUI wrapper for the existing Python RPC client.

Usage:

- From the repository root run:

```bash
python3 gui_client/main.py
```

- The GUI reads `ROBOT_USER` and `ROBOT_PASS` env vars if present.
- The GUI provides buttons for: conectar, desconectar, motores on/off, efector on/off, mover, modo absoluto/relativo, listar/ejecutar tareas, iniciar/terminar aprendizaje.

Notes:
- The GUI uses the existing `client_api.RobotRpcClient` to call RPC methods.
- This is a minimal proof-of-concept UI; adapt layout and styles as needed.
