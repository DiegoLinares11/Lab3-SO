#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <sys/syscall.h>   // Para SYS_gettid
#include <omp.h>


#define SIZE 9

int sudoku[SIZE][SIZE];

// funciones que se haran
int validarFilas();
int validarColumnas();
int validarSubgrilla(int filaInicio, int colInicio);

// pthread
void* hiloValidarColumnas(void* arg) {
    printf("[Thread] TID: %ld\n", syscall(SYS_gettid));
    int valido = validarColumnas();
    pthread_exit((void*)(size_t)valido);
}

// arreglo sudoku
void cargarSudokuDesdeArchivo(char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    char* data = mmap(NULL, 81, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    // Copiar al arreglo global
    for (int i = 0; i < 81; i++) {
        sudoku[i / SIZE][i % SIZE] = data[i] - '0';
    }

    munmap(data, 81);
    close(fd);
}



int validarFilas() {
    //omp_set_num_threads(9);
    int valido = 1;
    int i, j, k;

    #pragma omp parallel for schedule(dynamic) private(j, k) shared(valido)
    for (i = 0; i < SIZE; i++) {
        int contador[SIZE] = {0};
        for (j = 0; j < SIZE; j++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9) {
                valido = 0;
                break;
            }
            contador[num - 1]++;
        }
        for (k = 0; k < SIZE; k++) {
            if (contador[k] != 1) {
                valido = 0;
                break;
            }
        }
    }
    return valido;
}



int validarColumnas() {
    //omp_set_num_threads(9);
    int valido = 1;
    int i, j, k;

    #pragma omp parallel for schedule(dynamic) private(i, k) shared(valido)
    for (j = 0; j < SIZE; j++) {
        int contador[SIZE] = {0};
        for (i = 0; i < SIZE; i++) {
            int num = sudoku[i][j];
            if (num < 1 || num > 9) {
                valido = 0;
                break;
            }
            contador[num - 1]++;
        }
        for (k = 0; k < SIZE; k++) {
            if (contador[k] != 1) {
                valido = 0;
                break;
            }
        }
    }
    return valido;
}




int validarSubgrilla(int filaInicio, int colInicio) {
    int contador[SIZE] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int num = sudoku[filaInicio + i][colInicio + j];
            if (num < 1 || num > 9) return 0;
            contador[num - 1]++;
        }
    }
    for (int k = 0; k < SIZE; k++) {
        if (contador[k] != 1) return 0;
    }
    return 1;
}


//  Main
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo_sudoku>\n", argv[0]);
        return 1;
    }
    omp_set_num_threads(1);

    // Leer sudoku
    cargarSudokuDesdeArchivo(argv[1]);

    pid_t pid = fork();
    if (pid == 0) {
        //  Proceso hijo: ejecutar ps
        char padrePid[16];
        snprintf(padrePid, sizeof(padrePid), "%d", getppid());
        execlp("ps", "ps", "-p", padrePid, "-lLf", NULL);
        perror("execlp");
        exit(1);
    }

    //  Crear pthread para columnas
    pthread_t thread;
    void* resultado;
    pthread_create(&thread, NULL, hiloValidarColumnas, NULL);
    pthread_join(thread, &resultado);
    printf("[Main] Resultado columnas: %s\n", ((int)(size_t)resultado) ? "válido" : "inválido");

    // Esperar al hijo del fork
    wait(NULL);

    // Validar filas
    int filasValidas = validarFilas();

    //  Validar subgrillas (3x3)
    int subgrillasValidas = 1;

    #pragma omp parallel for collapse(2) schedule(dynamic) shared(subgrillasValidas)
    for (int i = 0; i < SIZE; i += 3) {
        for (int j = 0; j < SIZE; j += 3) {
            if (!validarSubgrilla(i, j)) {
                subgrillasValidas = 0;
            }
        }
    }


    // Resultado final
    if ((int)(size_t)resultado && filasValidas && subgrillasValidas) {
        printf(" Solución de Sudoku válida.\n");
    } else {
        printf(" Solución de Sudoku inválida.\n");
    }

    // Segundo fork para comparar LWP al final
    pid_t pid2 = fork();
    if (pid2 == 0) {
        char padrePid[16];
        snprintf(padrePid, sizeof(padrePid), "%d", getppid());
        execlp("ps", "ps", "-p", padrePid, "-lLf", NULL);
        perror("execlp");
        exit(1);
    }

    wait(NULL);
    return 0;
}
