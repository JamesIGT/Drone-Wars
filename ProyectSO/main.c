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
    int N;
    int velocidad;
    int distancia_alerta;
    int prob_defensa;
    int prob_perdida_com;
    int tiempo_espera;
} Parametros;

// Leer parámetros del archivo
Parametros leerParametros(const char *filename) {
    Parametros p;
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("No se pudo abrir el archivo de parámetros");
        exit(EXIT_FAILURE);
    }

    fscanf(f, "N=%d\n", &p.N);
    fscanf(f, "B=%*d\n"); // Saltamos B, no lo usamos por ahora
    fscanf(f, "VELOCIDAD=%d\n", &p.velocidad);
    fscanf(f, "Z=%d\n", &p.distancia_alerta);
    fscanf(f, "W=%d\n", &p.prob_defensa);
    fscanf(f, "Q=%d\n", &p.prob_perdida_com);
    fscanf(f, "R=%d\n", &p.tiempo_espera);
    fclose(f);
    return p;
}

// Generar blancos aleatorios entre 100 y 300
void generar_blancos(Coordenada blancos[], int n) {
    for (int i = 0; i < n; i++) {
        blancos[i].x = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
        blancos[i].y = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
    }
}

// Simular movimiento paso a paso y comunicación
void simular_drone(int id, Coordenada blanco, Parametros p) {
    float x = 0, y = 0;
    int comunicacion_perdida = 0;
    int tiempo_sin_com = 0;

    int dx = blanco.x;
    int dy = blanco.y;
    int distancia = abs(dx) + abs(dy); // Evitamos sqrt()
    int pasos = distancia / p.velocidad;
    if (pasos < 1) pasos = 1;

    float paso_x = (float)dx / pasos;
    float paso_y = (float)dy / pasos;

    printf("🚁 Drone %d despegando hacia blanco (%d, %d)\n", id, blanco.x, blanco.y);

    for (int i = 0; i < pasos; i++) {
        x += paso_x;
        y += paso_y;

        int xi = (int)x;
        int yi = (int)y;

        // Simular pérdida de comunicación
        if (!comunicacion_perdida && rand() % 100 < p.prob_perdida_co) {
            comunicacion_perdida = 1;
            printf("⚠️  Drone %d perdió comunicación\n", id);
        }

        if (comunicacion_perdida) {
            tiempo_sin_com++;
            if (tiempo_sin_com >= p.tiempo_espera) {
                comunicacion_perdida = 0;
                tiempo_sin_com = 0;
                printf("🔁 Drone %d recuperó comunicación\n", id);
            }
        } else {
            printf("📡 Drone %d en (%d, %d)\n", id, xi, yi);
        }

        sleep(1); // Simula tiempo entre movimientos
    }

    printf("✅ Drone %d alcanzó su blanco en (%d, %d)\n", id, blanco.x, blanco.y);
    exit(0); // Fin del proceso drone
}

int main() {
    srand(time(NULL));

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
            simular_drone(i, blancos[i], p);
        }
    }

    // Esperar a que todos los drones terminen
    for (int i = 0; i < p.N; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("🎯 Simulación completada\n");
    return 0;
}
