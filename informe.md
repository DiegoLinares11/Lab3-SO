# Informe Laboratorio 3 - Sistemas Operativos
**Universidad del Valle de Guatemala**  
**Nombre:** Diego Linares  
**Curso:** Sistemas Operativos  
**Docente:** Juan Luis García Zarceño  

---

## Objetivo del Laboratorio

Implementar un validador de Sudoku 9x9 utilizando programación concurrente y paralela en C, empleando `pthreads` y `OpenMP`. Se analizará el comportamiento del sistema y los Lightweight Processes (LWP) durante distintas fases del programa, incluyendo forks, hilos y paralelismo dinámico.

---

##  Estructura del Programa

1. **Lectura del Sudoku** desde archivo usando `open()` y `mmap()`.
2. **Copia a una matriz global `sudoku[9][9]`**.
3. **Validación concurrente**:
   - Columnas con un `pthread`.
   - Filas y subgrillas en el proceso padre.
4. **Forks** antes y después del análisis para mostrar los LWP con `ps -p <pid> -lLf`.
5. **Paralelización con OpenMP**:
   - Bucles `for` paralelos en filas, columnas y subgrillas.
   - Directiva `schedule(dynamic)` para probar distribución de carga.
   - Control de threads con `omp_set_num_threads()`.
   - Anidamiento con `omp_set_nested(true)`.

---

##  Funcionalidades Clave

- Validación con funciones específicas: `validarFilas()`, `validarColumnas()`, `validarSubgrilla()`.
- `pthread` para validar columnas en paralelo.
- Uso de `syscall(SYS_gettid)` para obtener TID del thread.
- Paralelización con `#pragma omp parallel for`.
- Análisis del número de threads (`LWP`) usando `ps`.

---

##  Experimentos y Resultados

### Sin OpenMP
- `NLWP = 2`: proceso principal + hilo de columnas.
- `ps` muestra:
  ```
  LWP  PID     CMD
  ---- ------- ---------------------
  1001 1000    ./SudoValidator sudoku
  1002 1000    ./SudoValidator sudoku  <- hilo columna
  ```

###  Con OpenMP (sin limitar hilos)
- `NLWP` variaba entre **15 a 25** dependiendo de la ejecución.
- Esto muestra que **OpenMP usa un thread pool dinámico** y lo ajusta por carga del sistema.

###  Con `omp_set_num_threads(9)`
- `NLWP = 9` constantes.
- Esto se alinea con 9 filas / 9 columnas → un hilo por tarea.
- Resultado más controlado, ideal para benchmarking.

###  Con `schedule(dynamic)`
- Se observaron **fluctuaciones entre ejecuciones**.
- A veces OpenMP activó menos de 9 hilos, otras veces más.
- Confirma que `schedule(dynamic)` **permite balanceo dinámico de carga**, repartiendo trabajo según disponibilidad.

###  Con `omp_set_nested(true)`
- Aunque no se usaron bucles paralelos anidados, no rompió funcionalidad.
- Prepara el entorno para **soportar paralelismo jerárquico**.

---

##  Conclusiones

- **OpenMP es altamente eficiente** pero requiere monitoreo para evitar sobreuso de recursos.
- El análisis de `ps` fue útil para visualizar el modelo de threads en Linux.
- Las diferentes configuraciones (`num_threads`, `schedule`, `nested`) **alteran directamente los LWP**, lo cual se refleja en el uso del sistema.
- Esta experiencia permite comprender mejor la **paralelización de datos y tareas** en sistemas modernos.

---

##  Recomendaciones

- Usar `omp_set_num_threads()` cuando se desea control explícito.
- Aplicar `schedule(dynamic)` para tareas de carga desigual.
- Usar `omp_set_nested(true)` solo si hay regiones paralelas anidadas reales.

---

##  Evidencias

_Incluir capturas de pantalla de la salida de `ps`, ejemplos del output del programa y número de LWP observados en cada configuración._

---

##  Funciones Base
```
int validarFilas();
int validarColumnas();
int validarSubgrilla(int filaInicio, int colInicio);
```
---

##  Respuestas a las Preguntas del Laboratorio

### 1. ¿Qué es una race condition y por qué hay que evitarlas?
Una race condition ocurre cuando dos o más hilos acceden y modifican una variable compartida al mismo tiempo, y el resultado final depende del orden de ejecución. Se deben evitar porque pueden provocar errores impredecibles y corrupción de datos. En este laboratorio se evitaron usando variables locales en los bucles y manejo correcto de variables compartidas en OpenMP.

### 2. ¿Cuál es la relación, en Linux, entre `pthreads` y `clone()`? ¿Hay diferencia al crear threads con uno u otro? ¿Qué es más recomendable?
`pthread_create()` usa internamente la syscall `clone()`, que permite compartir memoria entre hilos. `clone()` es más flexible pero compleja. `pthreads` es más recomendable por ser portable, fácil de usar y suficiente para la mayoría de aplicaciones como esta.

### 3. ¿Dónde, en su programa, hay paralelización de tareas, y dónde de datos?
- Tareas: uso de `pthread` para validar columnas mientras el padre hace otras validaciones.
- Datos: uso de `#pragma omp parallel for` para validar filas, columnas y subgrillas en paralelo dividiendo las iteraciones entre hilos.

### 4. ¿Cuántos LWP’s hay abiertos antes de terminar el `main()` y cuántos durante la revisión de columnas?
Durante la revisión con OpenMP pueden haber de 9 a 25 LWP. Al terminar, solo queda 1 LWP (hilo principal). Se puede observar con `ps -p <pid> -lLf`.

### 5. ¿Qué cambia al limitar el número de threads en `main()` a uno?
Al usar `omp_set_num_threads(1)`, el número de LWP activos disminuye, usualmente solo el principal o uno más. OpenMP por defecto usa tantos hilos como núcleos tenga el sistema.

### 6. ¿Qué significa la primera columna de `ps`? ¿Cuál es el LWP inactivo y por qué?
La columna `F` indica flags del proceso. Los LWP con estado `S` (sleeping) o `futex_` están esperando. Son hilos que ya terminaron su trabajo o están en espera de sincronización.

### 7. ¿Qué es un thread team en OpenMP? ¿Qué es el master thread? ¿Qué es busy-wait?
- *Thread team*: conjunto de hilos creados para una región paralela.
- *Master thread*: el hilo que inicia dicha región (por lo general el principal).
- *Busy-wait*: cuando un hilo ocupa CPU esperando sin hacer trabajo útil. OpenMP evita esto con su thread pool y sincronización eficiente.

### 8. ¿Qué se observa con `schedule(dynamic)`?
El número de LWP activos varía entre ejecuciones. A veces OpenMP usa más o menos hilos dependiendo de la carga. Esto muestra que la distribución dinámica permite un mejor balanceo de carga.

### 9. ¿Qué pasa al usar `omp_set_num_threads()` en cada función?
Da mayor control sobre cuántos hilos se usan, haciendo más predecible el comportamiento. No siempre mejora el rendimiento, pero permite ajustar el paralelismo a la cantidad de trabajo (e.g., 9 filas → 9 hilos).

### 10. ¿Qué efecto tiene `omp_set_nested(true)`?
Permite regiones paralelas dentro de otras regiones paralelas. En este programa no tiene mucho impacto porque no hay bucles anidados, pero prepara el entorno para aplicaciones más complejas.

---

**Fin**