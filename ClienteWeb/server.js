const express = require('express');
const xmlrpc = require('xmlrpc');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 3000;

// Configuración del servidor XML-RPC
const RPC_HOST = '127.0.0.1';  // Cambiado a localhost para pruebas locales
const RPC_PORT = 8080;
const RPC_PATH = '/RPC2';

// Crear cliente XML-RPC
const xmlrpcClient = xmlrpc.createClient({
  host: RPC_HOST,
  port: RPC_PORT,
  path: RPC_PATH
});

// Middleware
app.use(cors({
  origin: '*',
  methods: ['GET', 'POST', 'OPTIONS'],
  allowedHeaders: ['Content-Type', 'Authorization']
}));
app.use(express.json());
app.use(express.static(__dirname));

// Middleware para cabeceras adicionales
app.use((req, res, next) => {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET,PUT,POST,DELETE,OPTIONS');
  res.header('Access-Control-Allow-Headers', 'Content-Type, Authorization, Content-Length, X-Requested-With');
  next();
});

// Ruta principal
app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

// Endpoint proxy para XML-RPC
app.post('/rpc', async (req, res) => {
  try {
    const { method, params } = req.body;
    
    if (!method) {
      return res.status(400).json({
        error: 'El campo "method" es requerido'
      });
    }

    // Procesar parámetros para asegurar tipos correctos para XML-RPC
    const processedParams = (params || []).map(param => {
      if (typeof param === 'number') {
        // XML-RPC es muy estricto: cualquier número entero se marca como <i4> no como <double>
        // Necesitamos asegurar que TODOS los números tengan decimales para ser tratados como <double>
        let floatValue = parseFloat(param);
        
        // Si es un número entero (sin decimales), agregar una pequeña fracción decimal
        if (floatValue === Math.floor(floatValue)) {
          floatValue = floatValue + 0.01; // Agregar .01 para forzar tipo double
        }
        
        return floatValue;
      }
      return param;
    });

    console.log(`📨 Llamada XML-RPC: ${method}`, processedParams);

    // Llamar al servidor XML-RPC
    xmlrpcClient.methodCall(method, processedParams, (error, value) => {
      if (error) {
        console.error('❌ Error RPC:', error);
        
        if (error.code === 'ECONNREFUSED') {
          return res.status(503).json({
            error: {
              code: -32001,
              message: `No se puede conectar al servidor RPC en ${RPC_HOST}:${RPC_PORT}`,
              details: 'Verifica que el servidor RPC esté funcionando'
            }
          });
        } else if (error.code === 'ETIMEDOUT') {
          return res.status(504).json({
            error: {
              code: -32002,
              message: 'Timeout al conectar con el servidor RPC'
            }
          });
        } else {
          return res.status(500).json({
            error: {
              code: error.code || -32000,
              message: error.message || 'Error al ejecutar el método RPC',
              faultCode: error.faultCode,
              faultString: error.faultString
            }
          });
        }
      }

      console.log('✅ Respuesta RPC:', value);
      res.json({
        success: true,
        result: value
      });
    });

  } catch (error) {
    console.error('❌ Error en proxy:', error.message);
    res.status(500).json({
      error: {
        code: -32000,
        message: error.message
      }
    });
  }
});

// Endpoint de salud
app.get('/health', (req, res) => {
  console.log('🔍 Verificando conexión con servidor XML-RPC...');
  
  xmlrpcClient.methodCall('system.listMethods', [], (error, value) => {
    if (error) {
      console.error('❌ Error de conexión:', error.code || error.message);
      
      if (error.code === 'ECONNREFUSED') {
        return res.status(503).json({
          status: 'error',
          rpcServer: `${RPC_HOST}:${RPC_PORT}${RPC_PATH}`,
          message: 'No se puede conectar con el servidor RPC',
          hint: 'Verifica que el servidor esté corriendo en 127.0.0.1:8080'
        });
      } else if (error.faultCode) {
        // El servidor respondió pero el método no existe (esto es bueno)
        return res.json({
          status: 'ok',
          rpcServer: `${RPC_HOST}:${RPC_PORT}${RPC_PATH}`,
          message: 'Servidor XML-RPC alcanzable y funcionando',
          note: 'Usa robot.help para ver métodos disponibles',
          protocol: 'XML-RPC'
        });
      } else {
        return res.status(503).json({
          status: 'error',
          rpcServer: `${RPC_HOST}:${RPC_PORT}${RPC_PATH}`,
          message: error.message || 'Error desconocido'
        });
      }
    }

    console.log('✅ Servidor XML-RPC alcanzable');
    res.json({
      status: 'ok',
      rpcServer: `${RPC_HOST}:${RPC_PORT}${RPC_PATH}`,
      message: 'Servidor XML-RPC alcanzable y funcionando',
      methods: value || 'Métodos disponibles mediante robot.*',
      protocol: 'XML-RPC'
    });
  });
});

// Iniciar servidor
app.listen(PORT, '0.0.0.0', () => {
  console.log('╔════════════════════════════════════════════════════════╗');
  console.log('║  🚀 SERVIDOR PROXY XML-RPC INICIADO                   ║');
  console.log('╚════════════════════════════════════════════════════════╝');
  console.log('');
  console.log(`  📍 Servidor local:     http://localhost:${PORT}`);
  console.log(`  🔗 Servidor RPC:       http://${RPC_HOST}:${RPC_PORT}${RPC_PATH}`);
  console.log(`  📡 Protocolo:          XML-RPC`);
  console.log('');
  console.log('  Endpoints:');
  console.log('  ├─ GET  /           - Interfaz web');
  console.log('  ├─ POST /rpc        - Proxy XML-RPC');
  console.log('  └─ GET  /health     - Verificar conexión');
  console.log('');
  console.log('  📋 Métodos RPC disponibles:');
  console.log('  ├─ user.login(username, password)');
  console.log('  ├─ user.logout(username, password)');
  console.log('  ├─ robot.connect(username, password)');
  console.log('  ├─ robot.getStatus(username, password)');
  console.log('  ├─ robot.move(username, password, x, y, z, speed)');
  console.log('  ├─ robot.help(username, password)');
  console.log('  └─ ... y más (ver código fuente del servidor)');
  console.log('');
  console.log('  👉 Abre http://localhost:3000 en tu navegador');
  console.log('');
});
