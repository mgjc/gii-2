#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int x=10;
pthread_mutex_t mutex;
FILE * fich;
void *fun1(void){
	int i;
	for(i=0;i<10000;i++){
		pthread_mutex_lock(&mutex);
		x--;
		fprintf(fich,"Hilo 1 - decrementa - valor contador: %d\n", x);
		pthread_mutex_unlock(&mutex);
	}
};
void *fun2(void){
	int j;
	for(j=0;j<10000;j++){
		pthread_mutex_lock(&mutex);
		x--;
		x--;
		fprintf(fich,"Hilo 2 - decrementa - valor contador: %d\n", x);
		pthread_mutex_unlock(&mutex);
	}
};
void *fun3(void){
	int k;
	for(k=0;k<10000;k++){
		pthread_mutex_lock(&mutex);
		x++;
		fprintf(fich,"Hilo 3 - incrementa - valor contador: %d\n", x);
		pthread_mutex_unlock(&mutex);
	}
};
void *fun4(void){
	int l;
	for(l=0;l<10000;l++){
		pthread_mutex_lock(&mutex);
		x++;
		x++;
		fprintf(fich,"Hilo 4 - incrementa - valor contador: %d\n", x);
		pthread_mutex_unlock(&mutex);
	}
};
int main (void){


	pthread_t hilo1, hilo2, hilo3, hilo4;
	pthread_mutex_init(&mutex,NULL);
	fich=fopen("Solucion_Ejercicio1.txt","w");

	if(pthread_create(&hilo1,NULL,(void *)&fun1,NULL))
		puts("Hilo mal creado");
	if(pthread_create(&hilo2,NULL,(void *)&fun2,NULL))
		puts("Hilo mal creado");
	if(pthread_create(&hilo3,NULL,(void *)&fun3,NULL))
		puts("Hilo mal creado");
	if(pthread_create(&hilo4,NULL,(void *)&fun4,NULL))
		puts("Hilo mal crerrado");
	if(pthread_join(hilo1,NULL))
		puts("Hilo mal crerrado");
	if(pthread_join(hilo2,NULL))
		puts("Hilo mal crerrado");
	if(pthread_join(hilo3,NULL))
		puts("Hilo mal crerrado");
	if(pthread_join(hilo4,NULL))
		puts("Hilo mal creado");

	fprintf(fich,"Valor final del contador: %d\n", x);
	pthread_mutex_destroy(&mutex);
	fclose(fich);
	return 0;

}
