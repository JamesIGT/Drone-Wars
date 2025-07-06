# 🛰️ Simulador de Control y Defensa de Drones

Este proyecto simula una operación militar con **drones autónomos** que se dirigen a objetivos enemigos y pueden ser interceptados por **defensas antiaéreas**. Está desarrollado en **C**, usando **procesos independientes** y comunicación mediante **FIFOs (named pipes)**, diseñado para prácticas de concurrencia, IPC (inter-process communication) y señales en sistemas operativos.

---

## 🧠 Objetivo

Simular un entorno donde:

- Drones autónomos navegan hacia un blanco.
- Se comunican su posición en tiempo real.
- Pueden perder comunicación con el centro de mando.
- Son derribados si entran en el radio de una defensa.
- El sistema se puede reiniciar sin perder el monitoreo.

---

## 🗂️ Estructura del Proyecto

```bash
ProyectSO/
├── main.c                 # Código fuente principal
├── parametros.txt         # Archivo de configuración
├── README.md              # Documentación del proyecto
```
## ⚙️ Compilación

Requiere `gcc`:

```bash
gcc main.c -o masda
```

# 🚀 Ejecución

./masda

Puedes terminar el proceso con Ctrl+C. Si reinicias el programa, detectará los drones activos y recuperará el monitoreo sin reiniciarlos.
⚒️ Configuración (parametros.txt)

Este archivo contiene los parámetros de simulación:

N=5             # Número de drones (y defensas)
B=0             # (Ignorado en esta versión)
VELOCIDAD=5     # Velocidad de movimiento
Z=20            # Radio de acción de las defensas
W=50            # Probabilidad (%) de que una defensa derribe un dron
Q=10            # Probabilidad (%) de que un dron pierda comunicación
R=6             # Tiempo total de reconexión antes de autodestrucción

# 🛰️ Lógica del Dron

    Se mueve desde (0, 0) hasta su objetivo.

    Escribe su posición en fifo_pos_drone_X.

    Revisa constantemente si fue derribado (mensaje por fifo_drone_X).

    Tiene una probabilidad Q% de perder comunicación.

    Si pierde comunicación, intenta reconectarse hasta R segundos.

    Si no se reconecta, se autodestruye.

# 🛡️ Lógica de la Defensa

    Cada defensa escucha la posición del dron correspondiente.

    Si el dron entra en el radio Z, hay una probabilidad W% de que sea derribado.

    Si derriba al dron, envía el mensaje por fifo_drone_X y finaliza.

# 📡 Comunicación

Se utilizan FIFOs (named pipes) para IPC:

    fifo_drone_X: mensajes del dron al centro de comando.

    fifo_pos_drone_X: posición del dron para monitoreo y defensas.

    fifo_estado: FIFO compartido para notificaciones globales (opcional).

♻️ Persistencia

Si el centro de comando (main) termina abruptamente:

    Puedes volver a ejecutarlo.

    Verificará los FIFOs existentes.

    No lanza nuevos procesos para drones activos.

    Retoma el monitoreo de drones ya en vuelo.

📋 Ejemplo de Salida

🛰️ Drone 1 despegando hacia blanco (203, 149)
📡 Drone 1 en (100, 100)
📡 Drone 1 en (110, 110)
🛡️ Defensa 1 derribó al drone en (120, 120)
💥 Drone 1: fue derribado por defensa en (120, 120)
📩 C2 recibió: Drone 1: DERRIBADO POR DEFENSA

🧹 Limpieza

Para limpiar los FIFOs después de ejecutar el programa:

rm fifo_drone_* fifo_pos_drone_* fifo_estado

📦 Requisitos

    Sistema Linux/Unix

    gcc

    make (opcional)
