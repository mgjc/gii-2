#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>
#include <unistd.h>
#include <sys/types.h>

int main (void){

	int tiempo;
	int hijo;
	srand(time(NULL));

	tiempo=(rand()%(5-2+1))+2;

	printf("Proceso P1 empezando sentencia 1\n");
	sleep(tiempo);
	printf("Proceso P1 acabando sentencia 1\n");

	switch(fork()){

		case 0: //P2


			printf("Proceso P2 empezando sentencia 2\n");
			tiempo=(rand()%(5-2+1))+2;
			sleep(tiempo);
			printf("Proceso P2 acabando sentencia 2\n");

			switch(fork()){
				case 0: //P3
					printf("Proceso P3 empezando sentencia 4\n");
					tiempo=(rand()%(5-2+1))+2;
					sleep(tiempo);
					printf("Proceso P3 acabando sentencia 4\n");
					break;
				default:
					printf("Proceso P2 empezando sentencia 4\n");
					tiempo=(rand()%(5-2+1))+2;
					sleep(tiempo);
					printf("Proceso P2 acabando sentencia 4\n");
					wait(&hijo);
					printf("Proceso P2 empezando sentencia 5\n");
					tiempo=(rand()%(5-2+1))+2;
					sleep(tiempo);
					printf("Proceso P2 acabando sentencia 5\n");
			}
			break;
		default:
			printf("Proceso P1 empezando sentencia 3\n");
			tiempo=(rand()%(5-2+1))+2;
			sleep(tiempo);
			printf("Proceso P1 acabando sentencia 3\n");
			wait(&hijo);
			printf("Proceso P1 empezando sentencia 6\n");
			tiempo=(rand()%(5-2+1))+2;
			sleep(tiempo);
			printf("Proceso P1 acabando sentencia 6\n");
	}

	return 0;
}
