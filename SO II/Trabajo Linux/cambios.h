#define NEGRO    0
#define ROJO     1
#define VERDE    2
#define AMARILLO 3
#define AZUL     4
#define MAGENTA  5
#define CYAN     6
#define BLANCO   7

#define LETRAS "ABCDEFGHIJLMNOPRabcdefghijlmnopr"

int inicioCambios(int ret, int semAforos, char *z);
int aQuEGrupo(int grupoActual);
int refrescar(void);
void incrementarCuenta(void);
int finCambios(void);
void pon_error(char *mensaje);

