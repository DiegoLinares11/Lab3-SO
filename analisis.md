#  Análisis de Cambios en el Código y su Impacto con OpenMP

Este documento describe únicamente los cambios realizados al código del validador de Sudoku en C conforme se fueron integrando herramientas de paralelismo (`OpenMP`) y cómo afectaron el comportamiento del programa a nivel de hilos (`LWP`) observables con `ps`.

---

##  Fase Base: Código sin OpenMP

- Solo existía un `pthread` para validar columnas.
- Validación de filas y subgrillas era secuencial.
- **Resultado en `ps`:**  
  - 2 LWP: proceso principal + 1 hilo creado con `pthread`.

---

##  Integración de OpenMP (sin configuración adicional)

### Cambio:
Se agregaron directivas `#pragma omp parallel for` a:
- `validarFilas()`
- `validarColumnas()`
- Validación de subgrillas en `main()`.

### Resultado:
- OpenMP creó dinámicamente varios hilos.
- **`NLWP` variaba entre 15 a 25**.
- Hilos aparecían en `ps` en estado `futex_`, mostrando que existían pero estaban esperando.

---

##  Control con `omp_set_num_threads(9)`

### Cambio:
Se agregó `omp_set_num_threads(9);` al inicio de cada función paralela.

### Resultado:
- Número de hilos usados pasó a ser **exactamente 9**, uno por cada fila, columna o subgrilla.
- **`NLWP` constante = 9** durante ejecución paralela.
- Mejora la predictibilidad y control del comportamiento.

---

##  Cambio de estrategia con `schedule(dynamic)`

### Cambio:
Se añadió `schedule(dynamic)` a las directivas `#pragma omp`.

### Resultado:
- OpenMP repartió las iteraciones dinámicamente.
- El número de `LWP` activos **varió entre ejecuciones** (algunas 9, otras 17, 25...).
- Distribución de trabajo más eficiente en algunos casos, aunque menos predecible.

---

##  Activación de `omp_set_nested(true)`

### Cambio:
Se añadió `omp_set_nested(true);` al inicio de cada función paralela.

### Resultado:
- No cambió el número de `LWP` porque no había regiones anidadas paralelas reales.
- Pero el sistema **quedó preparado** para manejar paralelismo jerárquico si fuese necesario.

---

##  Conclusiones Técnicas

- Cada directiva o función de OpenMP afecta directamente la **cantidad y comportamiento de los hilos (`LWP`)** del sistema.
- Usar `omp_set_num_threads()` da control fino y reproduce resultados consistentes.
- `schedule(dynamic)` mejora balance de carga pero genera variabilidad.
- `omp_set_nested()` es más útil en arquitecturas paralelas complejas.

---