#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_DRONES 100
#define MIN_COORD 100
#define MAX_COORD 300

typedef struct {
    int x;
    int y;
} Coordenada;

typedef struct {
    int N;                  // Número de blancos y drones
    int velocidad;
    int distancia_alerta;
    int prob_defensa;
    int prob_perdida_com;
    int tiempo_espera;
} Parametros;

// Función para leer parámetros desde archivo
Parametros leerParametros(const char *filename) {
    Parametros p;
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("No se pudo abrir el archivo de parámetros");
        exit(EXIT_FAILURE);
    }

    fscanf(f, "N=%d\n", &p.N);
    fscanf(f, "VELOCIDAD=%d\n", &p.velocidad);
    fscanf(f, "Z=%d\n", &p.distancia_alerta);
    fscanf(f, "W=%d\n", &p.prob_defensa);
    fscanf(f, "Q=%d\n", &p.prob_perdida_com);
    fscanf(f, "R=%d\n", &p.tiempo_espera);

    fclose(f);
    return p;
}

// Genera coordenadas aleatorias para blancos entre 100 y 300
void generar_blancos(Coordenada blancos[], int n) {
    for (int i = 0; i < n; i++) {
        blancos[i].x = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
        blancos[i].y = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
    }
}

// Simulación individual de un drone
void simular_drone(int id, Coordenada blanco) {
    Coordenada posicion = {0, 0}; // Todos inician en (0, 0)

    printf("🚁 Drone %d despegando desde (0, 0) hacia blanco en (%d, %d)\n",
           id, blanco.x, blanco.y);

    sleep(1); // Aquí podría simularse movimiento

    printf("✅ Drone %d alcanzó el blanco en (%d, %d)\n", id, blanco.x, blanco.y);
    exit(0);
}

int main() {
    srand(time(NULL)); // Aleatoriedad

    Parametros p = leerParametros("parametros.txt");

    if (p.N > MAX_DRONES) {
        fprintf(stderr, "Número máximo de drones (%d) excedido\n", MAX_DRONES);
        exit(EXIT_FAILURE);
    }

    Coordenada blancos[MAX_DRONES];
    pid_t pids[MAX_DRONES];

    generar_blancos(blancos, p.N);

    printf("🗺️  Campo de batalla generado con %d drones/blancos\n", p.N);

    for (int i = 0; i < p.N; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            simular_drone(i, blancos[i]);
        }
    }

    // Espera a que todos los drones terminen
    for (int i = 0; i < p.N; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("🎯 Simulación completada\n");
    return 0;
}
