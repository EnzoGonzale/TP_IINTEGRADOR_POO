# Cliente Web XML-RPC para Robot

Cliente web que se comunica con un servidor XML-RPC ubicado en `192.168.1.125:8080/RPC2`.

## 🚀 Instalación

```bash
npm install
npm start
```

Abre `http://localhost:3000` en tu navegador.

## 🤖 Métodos RPC Disponibles

### Autenticación
- `user.login(username, password)` - Retorna `{role: int, token: string}` donde role: 0=ADMIN, 1=OPERATOR
- `user.logout(username, password)` - Cierra sesión del servidor

### Control del Robot (requieren token)
- `robot.connect(token)`
- `robot.disconnect(token)`
- `robot.getStatus(token)`
- `robot.move(token, x, y, z, speed)`
- `robot.moveDefaultSpeed(token, x, y, z)`
- `robot.enableMotors(token)`
- `robot.disableMotors(token)`
- `robot.help(token)`

### Reportes y Tareas
- `robot.getReport(token)`
- `robot.listTasks(token)`
- `robot.executeTask(token, taskId)`

### Administración (solo ADMIN)
- `robot.user_add(admin_token, new_user, new_pass, role)`

## 📝 Ejemplo de Uso

1. **Autenticarse:**
   - Método: `user.login`
   - Parámetros: `["principalAdmin", "1234"]`
   - Retorna: `{role: int, token: string}` donde role: 0=ADMIN, 1=OPERATOR

2. **Obtener estado:**
   - Método: `robot.getStatus`  
   - Parámetros: `["token_del_login"]`

3. **Mover robot:**
   - Método: `robot.move`
   - Parámetros: `["token_del_login", 100.0, 50.0, 25.0, 100.0]` (x, y, z, speed)
   
4. **Mover robot (velocidad por defecto):**
   - Método: `robot.moveDefaultSpeed`
   - Parámetros: `["token_del_login", 100.0, 50.0, 25.0]` (x, y, z)

## 🔧 Configuración

Edita en `server.js`:
```javascript
const RPC_HOST = '192.168.1.125';
const RPC_PORT = 8080;
```

## 📡 Protocolo

Utiliza XML-RPC (no JSON-RPC). El servidor proxy convierte automáticamente.
