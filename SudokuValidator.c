#define SIZE 9

int sudoku[SIZE][SIZE];

// estas son las funcionees que se haran
int validarFilas();
int validarColumnas();
int validarSubgrilla(int filaInicio, int colInicio);