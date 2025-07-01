#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
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

// Leer parámetros desde archivo
Parametros leerParametros(const char *filename) {
    Parametros p;
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("No se pudo abrir el archivo de parámetros");
        exit(EXIT_FAILURE);
    }

    fscanf(f, "N=%d\n", &p.N);
    fscanf(f, "B=%*d\n"); // Ignorar B por ahora
    fscanf(f, "VELOCIDAD=%d\n", &p.velocidad);
    fscanf(f, "Z=%d\n", &p.distancia_alerta);
    fscanf(f, "W=%d\n", &p.prob_defensa);
    fscanf(f, "Q=%d\n", &p.prob_perdida_com);
    fscanf(f, "R=%d\n", &p.tiempo_espera);
    fclose(f);
    return p;
}

// Generar blancos aleatorios
void generar_blancos(Coordenada blancos[], int n) {
    for (int i = 0; i < n; i++) {
        blancos[i].x = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
        blancos[i].y = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
    }
}

// Simular drone
void simular_drone(int id, Coordenada blanco, Parametros p) {
    float x = 0, y = 0;
    int dx = blanco.x;
    int dy = blanco.y;
    int distancia = abs(dx) + abs(dy); // Evita sqrt
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
        if (rand() % 100 < p.prob_perdida_com) {
            printf("⚠️  Drone %d ha perdido comunicación\n", id);

            // Centro intenta reconectar durante R segundos, reintentando cada R/3
            int intentos = 3;
            int sleep_interval = p.tiempo_espera / intentos;

            for (int j = 0; j < intentos; j++) {
                printf("📡 Comando intenta reconectar con Drone %d (intento %d)\n", id, j + 1);
                sleep(sleep_interval);
            }

            // El dron responde con 50% probabilidad
            int respuesta = rand() % 2; // 0 = OK, 1 = Falla
            if (respuesta == 1) {
                printf("💥 Drone %d respondió: FALLA CATASTRÓFICA. Autodestrucción.\n", id);

                // Enviar estado al centro de comando
                char fifo_nombre[64];
                snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", id);
                int fifo_fd = open(fifo_nombre, O_WRONLY);
                if (fifo_fd != -1) {
                    char mensaje[64];
                    snprintf(mensaje, sizeof(mensaje), "Drone %d: FALLA CATASTROFICA", id);
                    write(fifo_fd, mensaje, strlen(mensaje) + 1);
                    close(fifo_fd);
                }

                exit(1); // El dron termina su proceso por autodestrucción
            } else {
                printf("✅ Drone %d respondió: OK. Continúa misión.\n", id);
            }
        }

        printf("📡 Drone %d en (%d, %d)\n", id, xi, yi);
        sleep(1);
    }

    // IPC con named pipe
    char fifo_nombre[64];
    snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", id);

    int fifo_fd = open(fifo_nombre, O_WRONLY);
    if (fifo_fd != -1) {
        char mensaje[64];
        snprintf(mensaje, sizeof(mensaje), "Drone %d: DETONACION EN BLANCO", id);
        write(fifo_fd, mensaje, strlen(mensaje) + 1);
        close(fifo_fd);
    }

    printf("💥 Drone %d alcanzó su blanco en (%d, %d) y se detonó\n", id, blanco.x, blanco.y);
    exit(0);
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

    // Crear FIFOs antes de forking
    for (int i = 0; i < p.N; i++) {
        char fifo_nombre[64];
        snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", i);
        mkfifo(fifo_nombre, 0666);
    }

    // Lanzar drones
    for (int i = 0; i < p.N; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            simular_drone(i, blancos[i], p);
        }
    }

    // Leer mensajes de los FIFOs
    for (int i = 0; i < p.N; i++) {
        char fifo_nombre[64];
        snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", i);
        int fd = open(fifo_nombre, O_RDONLY);
        if (fd != -1) {
            char buffer[128];
            read(fd, buffer, sizeof(buffer));
            printf("📩 C2 recibió: %s\n", buffer);
            close(fd);
            unlink(fifo_nombre); // Limpieza del FIFO
        }
    }

    // Esperar a los procesos hijos
    for (int i = 0; i < p.N; i++) {
        waitpid(pids[i], NULL, 0);
    }

    printf("🎯 Simulación completada\n");
    return 0;
}
