#!/usr/bin/env python3
"""
Tkinter GUI client for robot RPC.
Provides a login screen and role-based panels with buttons mapping to the CLI commands.
"""
import os
import threading
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog

import sys
import os

# Ensure repository root is on sys.path so we can import client_api when
# executing this file directly (python3 gui_client/main.py)
repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if repo_root not in sys.path:
    sys.path.insert(0, repo_root)
from client_api import RobotRpcClient


class RobotGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Robot Client GUI")
        self.geometry("1500x800")
        self.client = RobotRpcClient()
        self.user = None
        self.password = None
        self.role = None
        self.learning = False
        self.token = None
        self.learning_commands = []
        self.create_login()
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def create_login(self):
        for w in self.winfo_children():
            w.destroy()
        frm = ttk.Frame(self, padding=20)
        frm.pack(fill=tk.BOTH, expand=True)

        ttk.Label(frm, text="Usuario:").grid(row=0, column=0, sticky=tk.W)
        self.user_entry = ttk.Entry(frm)
        self.user_entry.grid(row=0, column=1)

        ttk.Label(frm, text="Clave:").grid(row=1, column=0, sticky=tk.W)
        self.pass_entry = ttk.Entry(frm, show="*")
        self.pass_entry.grid(row=1, column=1)

        # prefill from env
        env_user = os.environ.get("ROBOT_USER")
        env_pass = os.environ.get("ROBOT_PASS")
        if env_user:
            self.user_entry.insert(0, env_user)
        if env_pass:
            self.pass_entry.insert(0, env_pass)

        login_btn = ttk.Button(frm, text="Login", command=self.do_login)
        login_btn.grid(row=2, column=0, columnspan=2, pady=10)

    def do_login(self):
        user = self.user_entry.get().strip()
        pwd = self.pass_entry.get().strip()
        try:
            res = self.client.proxy.user.login(user, pwd)
            if isinstance(res, dict):
                self.user = user
                self.token = res.get("token")
                self.role = int(res.get("role"))
                messagebox.showinfo("Login", f"Bienvenido {user}")
                self.create_main_panel()
            else:
                messagebox.showerror("Login", "Usuario o clave incorrectos")
        except Exception as e:
            messagebox.showerror("Error", f"Error de conexión: {e}")

    def create_main_panel(self):
        for w in self.winfo_children():
            w.destroy()
        top = ttk.Frame(self, padding=12)
        top.pack(side=tk.TOP, fill=tk.X)
        ttk.Label(top, text=f"Usuario: {self.user} | Rol: {'ADMIN' if self.role==0 else 'OPERATOR'}").pack(side=tk.LEFT)
        logout_btn = ttk.Button(top, text="Logout", command=self.logout)
        logout_btn.pack(side=tk.RIGHT)

        body = ttk.Frame(self, padding=12)
        body.pack(fill=tk.BOTH, expand=True)

        controls = ttk.LabelFrame(body, text="Controles")
        controls.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=4, pady=4)

        ttk.Button(controls, text="Conectar", command=self.rpc_connect).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Desconectar", command=self.rpc_disconnect).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Motores ON", command=self.rpc_motors_on).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Motores OFF", command=self.rpc_motors_off).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Efector ON", command=self.rpc_effector_on).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Efector OFF", command=self.rpc_effector_off).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Modo Absoluto", command=lambda: self.rpc_set_coord(True)).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Modo Relativo", command=lambda: self.rpc_set_coord(False)).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Estado", command=self.rpc_get_status).pack(fill=tk.X, pady=2)
        ttk.Button(controls, text="Ayuda", command=self.rpc_help).pack(fill=tk.X, pady=2)

        reportsOrTables = ttk.LabelFrame(body, text="Reportes y Usuarios")
        reportsOrTables.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=4, pady=4)
        ttk.Label(reportsOrTables, text="Reportes", font=("", 10)).pack(pady=(0, 5), anchor='w', padx=5)
        ttk.Button(reportsOrTables, text="Reporte", command=self.rpc_report).pack(fill=tk.X, pady=2)
        ttk.Button(reportsOrTables, text="Reporte Admin", command=self.report_admin).pack(fill=tk.X, pady=2)
        ttk.Button(reportsOrTables, text="Reporte Log", command=self.rpc_report_log).pack(fill=tk.X, pady=2)
        # Separador y título para la sección de usuarios
        ttk.Separator(reportsOrTables, orient='horizontal').pack(fill='x', pady=10, padx=5)
        ttk.Label(reportsOrTables, text="Usuarios", font=("", 10)).pack(pady=(0, 5), anchor='w', padx=5)
        ttk.Button(reportsOrTables, text="Agregar Usuario", command=self.rpc_add_user).pack(fill=tk.X, pady=2)
        ttk.Button(reportsOrTables, text="Usuarios Conectados", command=self.rpc_connected_users).pack(fill=tk.X, pady=2)


        taskbox = ttk.LabelFrame(body, text="Tareas")
        taskbox.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=4, pady=4)
        ttk.Button(taskbox, text="Listar Tareas", command=self.rpc_list_tasks).pack(fill=tk.X, pady=2)
        ttk.Button(taskbox, text="Ejecutar Tarea", command=self.ui_execute_task).pack(fill=tk.X, pady=2)
        ttk.Button(taskbox, text="Aprender Inicio", command=self.ui_learning_start).pack(fill=tk.X, pady=2)
        ttk.Button(taskbox, text="Aprender Fin", command=self.ui_learning_end).pack(fill=tk.X, pady=2)

        movebox = ttk.LabelFrame(self, text="Mover")
        movebox.pack(side=tk.BOTTOM, fill=tk.X, padx=5, pady=5)
        ttk.Label(movebox, text="X").pack(side=tk.LEFT)
        self.x_entry = ttk.Entry(movebox, width=8)
        self.x_entry.pack(side=tk.LEFT, padx=2)
        ttk.Label(movebox, text="Y").pack(side=tk.LEFT)
        self.y_entry = ttk.Entry(movebox, width=8)
        self.y_entry.pack(side=tk.LEFT, padx=2)
        ttk.Label(movebox, text="Z").pack(side=tk.LEFT)
        self.z_entry = ttk.Entry(movebox, width=8)
        self.z_entry.pack(side=tk.LEFT, padx=2)
        ttk.Label(movebox, text="Vel").pack(side=tk.LEFT)
        self.v_entry = ttk.Entry(movebox, width=8)
        self.v_entry.pack(side=tk.LEFT, padx=2)
        ttk.Button(movebox, text="Mover", command=self.ui_move).pack(side=tk.LEFT, padx=6)

        # log area
        self.log = tk.Text(self, height=25)
        self.log.pack(side=tk.BOTTOM, fill=tk.X)

    def logout(self):
        self.client.proxy.user.logout(self.token)
        self.user = None
        self.password = None
        self.role = None
        self.create_login()

    def append_log(self, msg: str):
        self.log.insert(tk.END, msg + "\n")
        self.log.see(tk.END)

    def on_closing(self):
        if self.token:
            try:
                self.append_log("Cerrando sesión antes de salir...")
                self.client.proxy.user.logout(self.token)
            except Exception as e:
                self.append_log(f"Error durante el logout automático: {e}")
        self.destroy()

# --- Controles ---
    def rpc_connect(self):
        threading.Thread(target=self._rpc_call, args=("connect",)).start()

    def rpc_disconnect(self):
        threading.Thread(target=self._rpc_call, args=("disconnect",)).start()

    def rpc_motors_on(self):
        threading.Thread(target=self._rpc_call, args=("enableMotors",)).start()

    def rpc_motors_off(self):
        threading.Thread(target=self._rpc_call, args=("disableMotors",)).start()

    def rpc_effector_on(self):
        threading.Thread(target=self._rpc_call, args=("setEffector", True)).start()
        if self.learning:
            self.learning_commands.append("M3")

    def rpc_effector_off(self):
        threading.Thread(target=self._rpc_call, args=("setEffector", False)).start()
        if self.learning:
            self.learning_commands.append("M5")

    def rpc_set_coord(self, absolute: bool):
        threading.Thread(target=self._rpc_call, args=("setCoordinateMode", absolute)).start()
        if self.learning:
            mode_cmd = "G90" if absolute else "G91"
            self.learning_commands.append(mode_cmd)

    def rpc_get_status(self):
        threading.Thread(target=self._rpc_call, args=("getStatus",)).start()

    def rpc_help(self):
        threading.Thread(target=self._rpc_call, args=("help",)).start()

    def ui_move(self):
        try:
            x = float(self.x_entry.get())
            y = float(self.y_entry.get())
            z = float(self.z_entry.get())
        except ValueError:
            messagebox.showerror("Mover", "Coordenadas inválidas")
            return
        vel = None
        try:
            vel = float(self.v_entry.get()) if self.v_entry.get().strip() else None
        except ValueError:
            messagebox.showerror("Mover", "Velocidad inválida")
            return
        if vel is None:
            threading.Thread(target=self._rpc_call, args=("moveDefaultSpeed", x, y, z)).start()
        else:
            threading.Thread(target=self._rpc_call, args=("move", x, y, z, vel)).start()
        if self.learning:
            # record a simple G1
            self.learning_commands.append(f"G1 X{x} Y{y} Z{z} E{vel if vel else 0}")

# --- Reportes y Usuarios ---
    def rpc_report(self):
        threading.Thread(target=self._rpc_call, args=("getReport",)).start()

    def report_admin(self):
        filter = simpledialog.askstring("Filtro Reporte Admin", "Filtrar por 'usuario' o 'resultado' (dejar en blanco para todos):", parent=self)
        if filter is None:
            threading.Thread(target=self._rpc_call, args=("getAdminReport", "", "")).start()
        elif filter.lower() == 'usuario':
            user_filter = simpledialog.askstring("Filtro Reporte Admin", "Filtrar por usuario:", parent=self)
            if user_filter is None:
                return
            threading.Thread(target=self._rpc_call, args=("getAdminReport", "username", user_filter)).start()
        elif filter.lower() == 'resultado':
            result_filter = simpledialog.askstring("Filtro Reporte Admin", "Filtrar por resultado (ej: 'success', 'Error'):", parent=self)
            if result_filter is None:
                return
            threading.Thread(target=self._rpc_call, args=("getAdminReport", filter, result_filter)).start()
        else:
            messagebox.showerror("Filtro Reporte Admin", "Filtro inválido. Use 'usuario' o 'resultado'.")
            return

    def rpc_report_log(self):
        filter = simpledialog.askstring("Filtro Reporte por Logger", "Filtrar por 'usuario' o 'nivel' (dejar en blanco para todos):", parent=self)
        if filter is None:
            threading.Thread(target=self._rpc_call, args=("getLogReport", "", "")).start()
        elif filter.lower() == 'usuario':
            user_filter = simpledialog.askstring("Filtro Reporte por Logger", "Filtrar por usuario:", parent=self)
            if user_filter is None:
                return
            threading.Thread(target=self._rpc_call, args=("getLogReport", "username", user_filter)).start()
        elif filter.lower() == 'nivel':
            level_filter = simpledialog.askstring("Filtro Reporte por Logger", "Filtrar por nivel (ej: 'INFO', 'ERROR', 'DEBUG', 'WARNING' y 'CRITICAL'):", parent=self)
            if level_filter is None:
                return
            threading.Thread(target=self._rpc_call, args=("getLogReport", "level", level_filter)).start()
        else:
            messagebox.showerror("Filtro Reporte por Logger", "Filtro inválido. Use 'usuario' o 'nivel'.")
            return

    def rpc_add_user(self):
        user = simpledialog.askstring("Agregar Usuario", "Usuario:")
        if not user:
            return
        password = simpledialog.askstring("Agregar Usuario", "Clave:")
        if not password:
            return
        role = simpledialog.askinteger("Agregar Usuario", "Rol (0 para administrador, 1 para operador):")
        if role is None:
            return
        success = self.client.proxy.user.add(self.token, user, password, role)
        if success:
            self.append_log(f"Usuario {user} agregado como  {'Administrador' if role == 0 else 'Operador'}")
        else:
            self.append_log(f"Error al agregar usuario {user}")

    def rpc_connected_users(self):
        users = self.client.proxy.user.list(self.token)
        if isinstance(users, list):
            report_lines = ["\n--- Usuarios Conectados ---"]
            if not users:
                report_lines.append("No hay usuarios conectados en este momento.")
            else:
                # Cabecera de la tabla
                header = f"| {'Usuario':<20} | {'Rol':<15} | {'Dirección IP':<25} |"
                separator = "+" + "-"*22 + "+" + "-"*17 + "+" + "-"*27 + "+"
                report_lines.append(separator)
                report_lines.append(header)
                report_lines.append(separator)
                # Filas de la tabla
            for user_data in users:
                username = user_data.get('username', 'N/A')
                role = 'ADMIN' if user_data.get('role') == 0 else 'OPERATOR'
                ip = user_data.get('ip_address', 'N/A')
                report_lines.append(f"| {username:<20} | {role:<15} | {ip:<25} |")
            report_lines.append(separator)
            self.append_log("\n".join(report_lines))
            self.append_log("\n")
        else:
            self.append_log(f"Error al obtener la lista de usuarios conectados: {users}")

# --- Tareas ---
    def rpc_list_tasks(self):
        threading.Thread(target=self._rpc_call, args=("listTasks",)).start()

    def ui_execute_task(self):
        tid = simpledialog.askstring("Ejecutar Tarea", "ID de tarea:")
        if not tid:
            return
        threading.Thread(target=self._rpc_call, args=("executeTask", tid)).start()

    def ui_learning_start(self):
        if self.learning:
            messagebox.showinfo("Aprendizaje", "Ya en modo aprendizaje")
            return
        tid = simpledialog.askstring("Aprender Inicio", "ID de la tarea:")
        name = simpledialog.askstring("Aprender Inicio", "Nombre de la tarea:")
        if not tid or not name:
            return
        self.learning = True
        self.learning_commands = []
        self.learning_id = tid
        self.learning_name = name
        self.append_log(f"[APRENDIZAJE] Inicio {tid} - {name}")

    def ui_learning_end(self):
        if not self.learning:
            messagebox.showinfo("Aprendizaje", "No hay aprendizaje en curso")
            return
        try:
            self.client.proxy.robot.addTask(self.token, self.learning_id, self.learning_name, self.learning_commands)
            self.append_log(f"[APRENDIZAJE] Tarea guardada: {self.learning_id} ({len(self.learning_commands)} comandos)")
        except Exception as e:
            self.append_log(f"[APRENDIZAJE] Error al guardar: {e}")
        finally:
            self.learning = False
            self.learning_commands = []


    def _rpc_call(self, method, *args):
        try:
            # map method name to proxy call
            fn = getattr(self.client.proxy.robot, method)
            res = fn(self.token, *args)
            # --- Controles ---
            if method == 'enableMotors':
                self.append_log('Los Motores fueron Activados')
            elif method == 'disableMotors':
                self.append_log('Los Motores fueron Desactivados')
            elif method == 'setEffector':
                state = args[0] if args else None
                if state:
                    self.append_log('El Efector fue Activado')
                else:
                    self.append_log('El Efector fue Desactivado')
            elif method == 'setCoordinateMode':
                absolute = args[0] if args else None
                if absolute:
                    self.append_log('Modo de Coordenadas Absoluto activado')
                else:
                    self.append_log('Modo de Coordenadas Relativo activado')
            elif method == 'getStatus':
                if isinstance(res, dict):
                    self.append_log("\n--- Estado del Robot ---")
                    for state in res:
                        activityState = res.get('activityState', 'N/A')
                        areMotorsEnabled = res.get('areMotorsEnabled', 'N/A')
                        cordinateMode = res.get('coordinateMode', 'N/A')
                        isConnected = res.get('isConnected', 'N/A')
                        position = res.get('position', 'N/A')
                    self.append_log(f"Conectado: {'Si' if isConnected else 'No'}")
                    self.append_log(f"Motores Habilitados: {'Si' if areMotorsEnabled else 'No'}")
                    self.append_log(f"Modo de Coordenadas: {cordinateMode}")
                    self.append_log(f"Estado de Actividad: {activityState}")
                    self.append_log(f"Posición Actual: {position}")
                    self.append_log("------------------------\n")
                else:
                    self.append_log("Respuesta inválida al obtener el estado.")
            elif method == 'help':
                if isinstance(res, dict):
                    self.append_log("\n--- Comandos Disponibles ---")
                    for cmd, desc in res.items():
                        cmd = checksHelps(cmd)
                        self.append_log(f"  {cmd:<30} - {desc}")
                    self.append_log("---------------------------\n")
                else:
                    self.append_log("Respuesta inválida al obtener la ayuda.")
            elif method in ('move', 'moveDefaultSpeed'):
                self.append_log('Movimiento enviado al robot')
            elif method == 'connect':
                self.append_log('Conectado al robot')
            elif method == 'disconnect':
                self.append_log('Desconectado del robot')

            # --- Reportes y Usuarios ---
            elif method == 'getReport':
                if isinstance(res, dict):
                    report_lines = ["\n--- Reporte de Actividad ---"]
                    
                    # Resumen del estado
                    report_lines.append("Estado General:")
                    report_lines.append(f"  - Conectado: {'Sí' if res.get('isConnected') else 'No'}")
                    report_lines.append(f"  - Motores Habilitados: {'Sí' if res.get('areMotorsEnabled') else 'No'}")
                    report_lines.append(f"  - Estado de Actividad: {res.get('activityState', 'N/A')}")
                    report_lines.append(f"  - Posición: {res.get('position', 'N/A')}")
                    report_lines.append(f"  - Total de Órdenes: {res.get('orderCount', 0)}")
                    report_lines.append(f"  - Órdenes con Error: {res.get('errorOrderCount', 0)}")
                    report_lines.append("") # Espacio

                    # Tabla de órdenes
                    orders = res.get('orders', [])
                    if orders:
                        report_lines.append("Historial de Órdenes:")
                        # Cabecera de la tabla
                        header = f"| {'Hora':<10} | {'Comando':<15} | {'Detalles':<60} | {'Resultado':<80} |"
                        separator = "+" + "-"*12 + "+" + "-"*17 + "+" + "-"*62 + "+" + "-"*82 + "+"
                        report_lines.append(separator)
                        report_lines.append(header)
                        report_lines.append(separator)
                        # Filas de la tabla
                        for order in orders:
                            ts = order.get('timestamp', 'N/A')
                            cmd = str(order.get('command', 'N/A'))
                            details = str(order.get('details', 'N/A'))
                            success = str(order.get('success', 'N/A'))
                            report_lines.append(f"| {ts:<10} | {cmd:<15.15} | {str(details):<60.60} | {str(success):<80.80} |")
                        report_lines.append(separator)
                    self.append_log("\n".join(report_lines))
                else:
                    self.append_log(f"Respuesta inválida al obtener el reporte: {res}")
            elif method == 'getAdminReport':
                # El servidor devuelve un diccionario: {'orders': [lista_de_ordenes]}
                if isinstance(res, dict) and 'orders' in res:
                    orders = res.get('orders', [])
                    report_lines = ["\n--- Reporte de Administrador ---"]
                    if not orders:
                        report_lines.append("No se encontraron registros con los filtros aplicados.")
                    else:
                        # Cabecera de la tabla
                        header = f"| {'Hora':<10} | {'Usuario':<15} | {'Comando':<20} | {'Detalles':<60} | {'Resultado':<80} |"
                        separator = "+" + "-"*12 + "+" + "-"*17 + "+" + "-"*22 + "+" + "-"*62 + "+" + "-"*82 + "+"
                        report_lines.append(separator)
                        report_lines.append(header)
                        report_lines.append(separator)
                        # Filas de la tabla
                        for entry in orders:
                            ts = entry.get('timestamp', 'N/A')
                            user = str(entry.get('username', 'N/A'))
                            cmd = str(entry.get('command', 'N/A')) # 'commandName' en C++ se mapea a 'command'
                            details = str(entry.get('details', 'N/A'))
                            result = str(entry.get('success', 'N/A'))
                            report_lines.append(f"| {ts:<10} | {user:<15.15} | {cmd:<20.20} | {details:<60.60} | {result:<80.80} |")
                        report_lines.append(separator)
                    self.append_log("\n".join(report_lines))
                else:
                    self.append_log(f"Respuesta inválida al obtener el reporte admin: {res}")

            elif method == 'getLogReport':
                # El servidor devuelve un diccionario: {'logs': [lista_de_logs]}
                if isinstance(res, dict) and 'logs' in res:
                    logs = res.get('logs', [])
                    report_lines = ["\n--- Reporte de Logger ---"]
                    if not logs:
                        report_lines.append("No se encontraron registros con los filtros aplicados.")
                    else:
                        # Cabecera de la tabla
                        header = f"| {'Hora':<20} | {'Usuario':<15} | {'Nivel':<10} | {'Mensaje':<100} | {'Nodo':<15} |"
                        separator = "+" + "-"*22 + "+" + "-"*17 + "+" + "-"*12 + "+" + "-"*102 + "+" + "-"*17 + "+"
                        report_lines.append(separator)
                        report_lines.append(header)
                        report_lines.append(separator)
                        # Filas de la tabla
                        for entry in logs:
                            ts = entry.get('timestamp', 'N/A')
                            user = str(entry.get('user', 'N/A'))
                            level = str(entry.get('level', 'N/A'))
                            message = str(entry.get('message', 'N/A'))
                            node = str(entry.get('node', 'N/A'))
                            report_lines.append(f"| {ts:<20.20} | {user:<15.15} | {level:<10.10} | {message:<100.100} | {node:<15.15} |")
                        report_lines.append(separator)
                    self.append_log("\n".join(report_lines))
                else:
                    self.append_log(f"Respuesta inválida al obtener el reporte de logger: {res}")

            # --- Tareas ---
            elif method == 'addTask':
                self.append_log('La tarea fue guardada en el robot')
            elif method == 'executeTask':
                self.append_log('La tarea fue ejecutada')
            elif method == 'listTasks':
                if isinstance(res, list) and res:
                    self.append_log("\n--- Tareas Disponibles ---")
                    for task in res:
                        task_id = task.get('id', 'N/A')
                        task_name = task.get('name', 'Sin nombre')
                        task_desc = task.get('description', 'Sin descripción.')
                        self.append_log(f"ID: {task_id} - {task_name}")
                        self.append_log(f"  -> {task_desc}\n")
                    self.append_log("-------------------------\n")
                else:
                    self.append_log("No hay tareas disponibles o la respuesta es inválida.")
            else:
                self.append_log(f"[{method}] -> {res}")
        except Exception as e:
            self.append_log(f"Error RPC {method}: {e}")

def checksHelps(cmd: str):
    if cmd == "estado":
        cmd = "Estado"
    if cmd == "motores_on":
        cmd = "Motores ON"
    if cmd == "motores_off":
        cmd = "Motores OFF"
    if cmd == "mover <x> <y> <z> <vel>":
        cmd = "Mover"
    if cmd == "efector_on":
        cmd = "Efector ON"
    if cmd == "efector_off":
        cmd = "Efector OFF"
    if cmd == "modo_absoluto":
        cmd = "Modo Absoluto"
    if cmd == "modo_relativo":
        cmd = "Modo Relativo"
    if cmd == "lista_tareas":
        cmd = "Listar Tareas"
    if cmd == "ejecutar_tarea <id_tarea>":
        cmd = "Ejecutar Tarea"
    if cmd == "aprender_inicio <id> <nombre>":
        cmd = "Aprender Inicio"
    if cmd == "aprender_fin":
        cmd = "Aprender Fin"
    if cmd == "reporte":
        cmd = "Reporte"
    if cmd == "report_admin [filtro] [val]":
        cmd = "Reporte Admin"
    if cmd == "report_log [filtro] [val]":
        cmd = "Reporte Log"
    if cmd == "conectar":
        cmd = "Conectar"
    if cmd == "desconectar":
        cmd = "Desconectar"
    if cmd == "user.add <user> <pass> <role>":
        cmd = "Agregar Usuario"
    if cmd == "salir":
        cmd = "Logout"
    return cmd


def main():
    app = RobotGUI()
    app.mainloop()


if __name__ == '__main__':
    main()
