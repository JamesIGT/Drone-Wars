// simulador_drones.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_DRONES 100

typedef struct {
    int N; // número de drones
    int X; // número de blancos
    int Y; // velocidad m/s
    int Z; // distancia a partir de la cual actúan defensas
    int W; // % de probabilidad de ser derribado
    int Q; // % de probabilidad de perder comunicación
    int R; // segundos para reintentos
} ParametrosGlobales;

pid_t drones[MAX_DRONES];
pid_t defensa_pid;

ParametrosGlobales parametros;

// Función del proceso drone (hijosssss)
void drone_proceso(int id) {
    printf("🛸 Drone %d despegando (PID: %d)\n", id, getpid());
    // Aquí irá la lógica del vuelo, comunicación y ataque en siguientes partes
    pause(); // Pausa hasta recibir señales o ser terminado
}

// Función del proceso defensa anti-drone
void defensa_proceso() {
    printf("🛡️ Defensa anti-drone activada (PID: %d)\n", getpid());
    // Lógica de monitoreo y derribo irá aquí
    pause();
}

int main() {
    // Leer parámetros desde consola
    printf("Ingrese número de drones (N): "); scanf("%d", &parametros.N);
    printf("Ingrese número de blancos (X): "); scanf("%d", &parametros.X);
    printf("Ingrese velocidad de drones (Y m/s): "); scanf("%d", &parametros.Y);
    printf("Ingrese distancia de activación de defensas (Z): "); scanf("%d", &parametros.Z);
    printf("Ingrese %% de probabilidad de ser derribado (W): "); scanf("%d", &parametros.W);
    printf("Ingrese %% de pérdida de comunicación (Q): "); scanf("%d", &parametros.Q);
    printf("Ingrese tiempo máximo de reintento (R seg): "); scanf("%d", &parametros.R);

    // Crear drones
    for (int i = 0; i < parametros.N; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Proceso hijo
            drone_proceso(i);
            exit(0);
        } else {
            drones[i] = pid;
        }
    }

    // Crear proceso de defensa
    defensa_pid = fork();
    if (defensa_pid == 0) {
        defensa_proceso();
        exit(0);
    }

    // Proceso padre espera
    for (int i = 0; i < parametros.N; i++) {
        waitpid(drones[i], NULL, 0);
    }
    waitpid(defensa_pid, NULL, 0);

    printf("🎯 Simulación finalizada\n");
    return 0;
}
