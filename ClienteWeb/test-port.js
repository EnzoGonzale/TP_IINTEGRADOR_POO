const express = require('express');
const app = express();
const PORT = 8081;  // Puerto diferente

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
        <title>Test Connection</title>
        <style>
            body { 
                font-family: Arial, sans-serif; 
                margin: 20px; 
                background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
                color: white;
                text-align: center;
                padding: 50px;
            }
            h1 { font-size: 2.5em; }
            .info { 
                background: rgba(255,255,255,0.2); 
                padding: 20px; 
                border-radius: 15px; 
                margin: 20px 0; 
            }
        </style>
    </head>
    <body>
        <h1>✅ ¡Conexión Exitosa!</h1>
        <div class="info">
            <p>🎯 <strong>Puerto 8081 funciona correctamente</strong></p>
            <p>📱 <strong>Acceso desde:</strong> ${req.headers['user-agent']}</p>
            <p>🌐 <strong>IP cliente:</strong> ${req.connection.remoteAddress}</p>
            <p>⏰ <strong>Hora:</strong> ${new Date().toLocaleString()}</p>
        </div>
        <p>Si ves esto, la red funciona. Probaremos el puerto 3000 después.</p>
    </body>
    </html>
  `);
});

// Iniciar servidor en todas las interfaces
app.listen(PORT, '0.0.0.0', () => {
  console.log(`
╔════════════════════════════════════════════╗
║  🔍 PRUEBA DE CONECTIVIDAD - PUERTO 8081   ║
╚════════════════════════════════════════════╝

  📱 Intenta desde tu celular:
  🔗 http://192.168.159.128:8081
  
  Si funciona este puerto, el problema es específico del puerto 3000
  `);
});