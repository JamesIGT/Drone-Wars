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

void generar_blancos(Coordenada blancos[], int n) {
    for (int i = 0; i < n; i++) {
        blancos[i].x = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
        blancos[i].y = MIN_COORD + rand() % (MAX_COORD - MIN_COORD + 1);
    }
}

void simular_defensa(int id, Coordenada blanco, Parametros p) {
    char fifo_pos[64];
    snprintf(fifo_pos, sizeof(fifo_pos), "fifo_pos_drone_%d", id);

    int fd = open(fifo_pos, O_RDONLY);
    if (fd == -1) {
        perror("No se pudo abrir FIFO de posición del dron");
        exit(1);
    }

    while (1) {
        int pos[2];
        int bytes = read(fd, pos, sizeof(pos));
        if (bytes <= 0) break;

        int dx = pos[0] - blanco.x;
        int dy = pos[1] - blanco.y;
        int dist2 = dx*dx + dy*dy;

        if (dist2 <= p.distancia_alerta * p.distancia_alerta) {
            if (rand() % 100 < p.prob_defensa) {
                printf("\U0001F6E1\uFE0F  Defensa %d derribó al drone en (%d, %d)\n", id, pos[0], pos[1]);

                // Enviar mensaje de destrucción
                char fifo_estado[64];
                snprintf(fifo_estado, sizeof(fifo_estado), "fifo_drone_%d", id);
                int fd2 = open(fifo_estado, O_WRONLY);
                if (fd2 != -1) {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Drone %d: DERRIBADO POR DEFENSA", id);
                    write(fd2, msg, strlen(msg) + 1);
                    close(fd2);
                }
                close(fd);
                exit(0);
            }
        }
    }
    close(fd);
    exit(0);
}

void simular_drone(int id, Coordenada blanco, Parametros p) {
    float x = 0, y = 0;
    int dx = blanco.x;
    int dy = blanco.y;
    int distancia = abs(dx) + abs(dy);
    int pasos = distancia / p.velocidad;
    if (pasos < 1) pasos = 1;

    float paso_x = (float)dx / pasos;
    float paso_y = (float)dy / pasos;

    printf("\U0001F681 Drone %d despegando hacia blanco (%d, %d)\n", id, blanco.x, blanco.y);

    char fifo_pos[64];
    snprintf(fifo_pos, sizeof(fifo_pos), "fifo_pos_drone_%d", id);
    int pos_fd = open(fifo_pos, O_WRONLY);

    for (int i = 0; i < pasos; i++) {
        x += paso_x;
        y += paso_y;

        int xi = (int)x;
        int yi = (int)y;

        if (pos_fd != -1) {
            int pos[2] = {xi, yi};
            write(pos_fd, pos, sizeof(pos));
        }

        if (rand() % 100 < p.prob_perdida_com) {
            printf("\u26A0\uFE0F  Drone %d ha perdido comunicación\n", id);
            int intentos = 3;
            int sleep_interval = p.tiempo_espera / intentos;

            for (int j = 0; j < intentos; j++) {
                printf("\U0001F4E1 Comando intenta reconectar con Drone %d (intento %d)\n", id, j + 1);
                sleep(sleep_interval);
            }

            int respuesta = rand() % 2;
            if (respuesta == 1) {
                printf("\U0001F4A5 Drone %d respondió: FALLA CATASTRÓFICA. Autodestrucción.\n", id);

                char fifo_nombre[64];
                snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", id);
                int fifo_fd = open(fifo_nombre, O_WRONLY);
                if (fifo_fd != -1) {
                    char mensaje[64];
                    snprintf(mensaje, sizeof(mensaje), "Drone %d: FALLA CATASTROFICA", id);
                    write(fifo_fd, mensaje, strlen(mensaje) + 1);
                    close(fifo_fd);
                }
                if (pos_fd != -1) close(pos_fd);
                exit(1);
            } else {
                printf("\u2705 Drone %d respondió: OK. Continúa misión.\n", id);
            }
        }

        printf("\U0001F4E1 Drone %d en (%d, %d)\n", id, xi, yi);
        sleep(1);
    }

    if (pos_fd != -1) close(pos_fd);

    char fifo_nombre[64];
    snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", id);
    int fifo_fd = open(fifo_nombre, O_WRONLY);
    if (fifo_fd != -1) {
        char mensaje[64];
        snprintf(mensaje, sizeof(mensaje), "Drone %d: DETONACION EN BLANCO", id);
        write(fifo_fd, mensaje, strlen(mensaje) + 1);
        close(fifo_fd);
    }

    printf("\U0001F4A5 Drone %d alcanzó su blanco en (%d, %d) y se detonó\n", id, blanco.x, blanco.y);
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

    printf("\U0001F5FA\uFE0F  Campo de batalla generado con %d drones/blancos\n", p.N);

    for (int i = 0; i < p.N; i++) {
        char fifo_estado[64];
        snprintf(fifo_estado, sizeof(fifo_estado), "fifo_drone_%d", i);
        mkfifo(fifo_estado, 0666);

        char fifo_pos[64];
        snprintf(fifo_pos, sizeof(fifo_pos), "fifo_pos_drone_%d", i);
        mkfifo(fifo_pos, 0666);
    }

    for (int i = 0; i < p.N; i++) {
        if (fork() == 0) {
            simular_drone(i, blancos[i], p);
        }
        if (fork() == 0) {
            simular_defensa(i, blancos[i], p);
        }
    }

    for (int i = 0; i < p.N; i++) {
        char fifo_nombre[64];
        snprintf(fifo_nombre, sizeof(fifo_nombre), "fifo_drone_%d", i);
        int fd = open(fifo_nombre, O_RDONLY);
        if (fd != -1) {
            char buffer[128];
            read(fd, buffer, sizeof(buffer));
            printf("\U0001F4E9 C2 recibió: %s\n", buffer);
            close(fd);
            unlink(fifo_nombre);
        }
        char fifo_pos[64];
        snprintf(fifo_pos, sizeof(fifo_pos), "fifo_pos_drone_%d", i);
        unlink(fifo_pos);
    }

    for (int i = 0; i < 2 * p.N; i++) {
        wait(NULL);
    }

    printf("\U0001F3AF Simulación completada\n");
    return 0;
}
