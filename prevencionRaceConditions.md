#  Prevención de Race Conditions en el Laboratorio

##  ¿Qué es una Race Condition?

Una *race condition* ocurre cuando dos o más hilos acceden y modifican una variable compartida al mismo tiempo, y el resultado final depende del orden en que se ejecutan los hilos. Esto puede causar errores impredecibles, corrupción de datos y resultados incorrectos.

---

##  ¿Cómo se evitaron en este laboratorio?

### 1. Uso de variables locales en bucles paralelos

```c
int contador[SIZE] = {0}; // esto esta dentro del for
```

Cada hilo que ejecuta una iteración del `for` tiene su **propia copia privada** del arreglo `contador`, evitando interferencia entre hilos.

---

### 2. Escritura controlada de variables compartidas

```c
int valido = 1;

#pragma omp parallel for private(j, k) shared(valido)
```

- Todos los hilos pueden leer `valido`.
- Solo lo escriben si encuentran un error (`valido = 0`).
- Como nunca se vuelve a poner en `1`, **no importa si varios hilos escriben 0**.

---

### 3. No se usaron secciones críticas porque no fueron necesarias

El diseño evitó compartir variables entre hilos más allá de `valido`, y no hubo acumulaciones o estructuras compartidas modificadas simultáneamente.

---

##  ¿Qué se podría usar en casos más complejos?

- `#pragma omp critical`: para proteger secciones sensibles.
- `#pragma omp atomic`: para operaciones atómicas simples.
- `reduction(+:suma)`: si se necesita acumular resultados entre hilos.
- `mutex` o `semaphores`: en programación con `pthreads`.

---

## Conclusión

Las race conditions fueron evitadas mediante:

| Estrategia                          | Beneficio                                  |
|-------------------------------------|---------------------------------------------|
| Variables locales por hilo          | Cada hilo tiene su propio espacio seguro    |
| Solo lectura compartida             | Evita conflictos al leer datos              |
| Escritura única no reversible (valido=0) | Resultado consistente, sin interferencia    |
| Diseño sin acumulaciones paralelas  | No fue necesario usar sincronización        |

Este enfoque permitió una ejecución paralela **segura y eficiente** del validador de Sudoku.

---

**Fin del análisis de race conditions**
