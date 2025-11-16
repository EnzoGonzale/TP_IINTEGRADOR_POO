const express = require('express');
const app = express();
const PORT = 3000;

// Configuración básica para acceso móvil
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET,POST,OPTIONS');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  next();
});

// Página de prueba simple
app.get('/', (req, res) => {
  res.send(`
    <!DOCTYPE html>
    <html>
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Test Mobile Access</title>
        <style>
            body { 
                font-family: Arial, sans-serif; 
                margin: 20px; 
                background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
                color: white;
                text-align: center;
                padding: 50px;
            }
            h1 { font-size: 2em; }
            .info { 
                background: rgba(255,255,255,0.1); 
                padding: 20px; 
                border-radius: 10px; 
                margin: 20px 0; 
            }
        </style>
    </head>
    <body>
        <h1>🎉 ¡Acceso Móvil Exitoso!</h1>
        <div class="info">
            <p>Si puedes ver esta página desde tu celular, la conectividad funciona perfectamente.</p>
            <p><strong>IP del servidor:</strong> ${req.ip}</p>
            <p><strong>User Agent:</strong> ${req.headers['user-agent']}</p>
            <p><strong>Hora:</strong> ${new Date().toLocaleString()}</p>
        </div>
        <p>Ahora puedes usar la aplicación principal.</p>
    </body>
    </html>
  `);
});

// Iniciar servidor en todas las interfaces
app.listen(PORT, '0.0.0.0', () => {
  console.log(`
╔════════════════════════════════════════════╗
║  🧪 SERVIDOR DE PRUEBA MÓVIL INICIADO     ║
╚════════════════════════════════════════════╝

  📱 Prueba desde tu celular:
  🔗 http://192.168.159.128:3000
  
  Si funciona, presiona Ctrl+C y usa el servidor principal
  `);
});
