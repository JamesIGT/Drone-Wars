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

## ⚙️ Compilación

Requiere `gcc`:

```bash
gcc main.c -o masda
