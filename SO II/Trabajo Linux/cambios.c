#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include "cambios.h"

//estructura para los hijos
typedef struct tipoHijo{
	char nombre;
	int grupo;
	int direccion;
	int destino;
}tipoHijo;

//estructura del mensaje que envian los hijos
typedef struct tipoMensajeHijo{
	long tipo; //el tipo es 'z'=Zacarias
	tipoHijo datos;
}tipoMensajeHijo;

//estructura del mensaje que envia el padre
typedef struct tipoMensajePadre{
	long tipo; //el tipo es el nombre de los hijos
}tipoMensajePadre;

//Estructuras para colas
typedef char tipoElemento;

typedef struct tipoCelda { 
	tipoElemento elemento;
	struct tipoCelda * sig; 
} tipoCelda;

typedef struct {
        tipoCelda *frente, *fondo;
} Cola;

//FUNCIONES
void manejadoraPadre(int);
void manejadoraHijos(int);
void iniciarValores(int i, char * pMem, tipoHijo * hijo);
void operacionWait(int, int);
void operacionSignal(int, int );
void operacionEsperarCero(int );
void eliminarMecanismosIPC(void);
void hayCiclo(tipoMensajeHijo mensaje,Cola soy[][4]);

//Funciones para colas
int colaCreaVacia(Cola *c);
int colaVacia(Cola *c);
int colaInserta(Cola *c, tipoElemento elemento);
tipoElemento colaSuprime(Cola *c);

//Estructura con las variables globales para los mecanismos ipc
typedef struct {
	int shmid;
	int semid;
	int msgid;
} IPC;

//Variables globales
IPC ipc;	//Declaracion de la estructura que contendra las variables para los mecanismos ipc
pid_t pids[32]={-2};		// vector donde guardaremos los diferentes pids de los hijos
sigset_t backup;


//Comienza el main
int main (int argc, char *argv[]){

	char error[40]; 						// variable para depurar con pon_error
	int velocidad;							// velocidad rapida o lenta
	int tiempo;								// tiempo de la alarma (15 o 30)
	tipoHijo hijo; 							// informacion sobre los hijos
	int numGrupo;							// grupo al que se quiere cambiar un hijo
	char * pMem;							// puntero a la zona de memoria compartida
	int * pCont;							// puntero al contador de la memoria compartida tratado como entero
	tipoMensajeHijo mensaje;				// mensaje que va a enviar el hijo y recibira el padre
	tipoMensajePadre mensajeConfirmacion;	// mensaje que envia el padre y recibira el hijo
	key_t key;								// clave
	Cola soy[4][4];							// matriz de colas donde se almacenan los mensajes
	int estoy, voy;							// contadores para inicializar la matriz	
	int i;									// contador
	int cont;								// contador
	sigset_t conjunto;						// variables para la modificacion del comportamiento de las señales
	struct sigaction accion_SIGINT, accion_SIGALRM;

    union senum{
		int val;
		struct semid_ds *buf;
		ushort_t *array;
	}semval;

	//------------------------------
	//COMPROBACION DE PARAMETROS
	//------------------------------
	if (strcmp(argv[0],"./cambios")!=0 || argc>2 || argc<2){
		perror("Modo de uso: ./cambios num (con num>0)");
		exit(-1);
	}

	// convertir el primer parametro a entero
	velocidad=atoi(argv[1]);

	// comparar si el valor introducido es un numero negativo
	if(velocidad<0){
		perror("Modo de uso: ./cambios num (con num>0)");
		exit(-1);
	}
	if(velocidad==0 && strcmp(argv[1],"0")!=0){
		perror("Modo de uso: ./cambios num (siendo num un numero entero, no una cadena)");
		exit(-1);
	}
	if(velocidad==0) tiempo=15;
	else tiempo=30;


	//------------------------------
	//SEÑALES
	//------------------------------
	if(sigfillset(&conjunto)==-1){
		perror("Error:sigfillset");
		exit(-2);	
	}
	if(sigdelset(&conjunto,SIGINT)==-1){
		perror("Error:sigdelset:SIGINT");
		exit(-2);	
	}
	if(sigdelset(&conjunto,SIGALRM)==-1){
		perror("Error:sigdelset:SIGALRM");
		exit(-2);	
	}
	if(sigdelset(&conjunto,SIGTERM)==-1){
		perror("Error:sigdelset:SIGTERM");
		exit(-2);	
	}

	sigprocmask(SIG_SETMASK,&conjunto,&backup);


	//------------------------------
	//MECANISMOS IPC
	//------------------------------

	//inicializar los ipc a -1
	ipc.shmid=-1;
	ipc.semid=-1;
	ipc.msgid=-1;


	//----SEMAFORO----
	// creamos un lote de 3 semaforos
	if((ipc.semid=semget(IPC_PRIVATE,4,IPC_CREAT | 0600))==-1){
       	perror("Error:semget");
       	raise(SIGINT);
		exit(-2);	
   	}
	//El primer semaforo queda para uso de libcambios.a
	//inicializo el segundo semaforo, se usara para acceso memoria compartida
	semval.val=1;
	if (semctl(ipc.semid, 1, SETVAL, semval)==-1){
		perror("Error:semctl");
       	raise(SIGINT);
		exit(-2);	
	}	
	//inicializo el tercer semaforo, se usara para esperar por los hijos
   	semval.val=32;
	if (semctl(ipc.semid, 2, SETVAL, semval)==-1){
        perror("Error:semctl");
       	raise(SIGINT);
		exit(-2);	
    }

	//----BUZON---
	// creamos una clave para el buzon
	if ((key = ftok("cambios.c", 'C')) == -1) {
 		perror("Error:ftok");
       	raise(SIGINT);
		exit(-2);	
 	}
 	// creamos el buzon
	if((ipc.msgid=msgget(key, IPC_CREAT | 0644))==-1){
		perror("Error:msgget");
       	raise(SIGINT);
		exit(-2);	
	}

	//----MEMORIA COMPARTIDA----
	//zona de memoria compartida, 80 para las letras y los numeros, 4 libres y 4 para el contador
    if((ipc.shmid=shmget(IPC_PRIVATE,sizeof(char)*89,IPC_CREAT | 0600))==-1){
       	perror("Error:shmget");
    	raise(SIGINT);
		exit(-2);	
    }
	// puntero a la zona de memoria compartida
    pMem = (char *) shmat(ipc.shmid,(char *)0,0);
    if(pMem==NULL){
       	perror("Error:shmat");
    	raise(SIGINT);
		exit(-2);	
    }
	//inicializo la zona de memoria compartida
	//los nombres y grupos: nombre (espacio) y grupo (1,2,3 o 4)
	i=1;
	while(i<80){
		pMem[i-1]=' ';
		pMem[i]=(i/20)+1;
		i+=2;	
	}
	//el contador a 0
	for(i=0; i<4; i++){
		pMem[84+i]=0;
	}
	// puntero al contador tratado como entero
	pCont = (int *) (pMem+84);

    //inicializo las colas
	for(estoy=0; estoy<4; estoy++){
		for(voy=0; voy<4; voy++){
			colaCreaVacia(&(soy[estoy][voy]));
		}
	}

	//llamamos a la funcion inicioCambios para que empiece el programa
	if(inicioCambios(velocidad,ipc.semid,pMem)==-1){
		pon_error("Error:inicioCambios");
    	raise(SIGINT);
		exit(-2);	
	}

	//Creo los procesos
	for(i=0; i<32; i++){
		switch(pids[i]=fork()){
			case -1: 
				//Elimina todos los procesos creados hasta el momento 
				//y mecanismos IPC en caso de que no se pueda crear un proceso
				perror("Error:fork");
				if(kill(getpid(),SIGINT)==-1)
					perror("Error:kill:CreacionHijos");
				pause();
				break;
			case 0:

				if(sigaction(SIGINT,&accion_SIGINT,NULL)==-1){
					perror("Error:sigaction");
					if(kill(getppid(),SIGINT)==-1)
						perror("Error:kill:SIGACTIONHIJOS");
					pause();
				}

				//empezamos metiendo cada hijo en un grupo			
				iniciarValores(i,pMem,&hijo);
				if( refrescar()==-1 ){
					pon_error("Error:refrescar");
    				raise(SIGINT);
					exit(-2);	
				}
				//Wait para que pueda empezar el proceso padre cuando esten todos los hijos iniciados en un grupo
				operacionWait(2,1);

				for(;;){
					//elige el grupo de destino		
					numGrupo=aQuEGrupo(hijo.grupo);
					hijo.destino=numGrupo;
					
					//Envia un mensaje con su informacion a Zacarias
					mensaje.tipo= (long) 'z';
					mensaje.datos = hijo;
					msgsnd(ipc.msgid,&mensaje, sizeof(tipoMensajeHijo) - sizeof(long),0);
					//Recibe mensaje de Zacarias cuando se le concede el cambio				
					msgrcv(ipc.msgid,&mensajeConfirmacion,sizeof(tipoMensajeHijo) - sizeof(long), (long) hijo.nombre,0);

					//controla la escritura de la zona de memoria compartida
					operacionWait(1,1);

					//busca un blanco
					cont=(numGrupo-1)*20;
					while(cont<(numGrupo*20)){
						if(pMem[cont]==' '){
							//hay un hueco--> siempre tiene que haber uno
							pMem[hijo.direccion]=' ';
							hijo.direccion=cont;
							hijo.grupo=numGrupo;
							pMem[cont]=hijo.nombre;
							pMem[cont+1]=numGrupo;
							(*pCont)++;
							incrementarCuenta();
							break;
						}
						cont+=2;
					}//No deberia ocurrir la condicion fin ==> grupo lleno					
                    if(refrescar()==-1){
						pon_error("Error:refrescar");
    					raise(SIGINT);
						exit(-2);	
					}
					//controla la escritura de la zona de memoria compartida
					operacionSignal(1,1);
					//controla las permutas
					operacionWait(3,1);		
				}//fin for(;;)
				break;
			default:
				;
		}
	}

	//Aqui solo llega el padre, por lo que la manejadora para el SIGALRM se la vamos a poner solo a el
	//a igual que la funcion alarm(), para que solo le llegue a el la seNal SIGALRM

	accion_SIGINT.sa_handler=manejadoraPadre;
	accion_SIGINT.sa_flags=0;
	accion_SIGINT.sa_mask=conjunto;

	if(sigaction(SIGINT,&accion_SIGINT,NULL)==-1){
		perror("Error:sigaction");
		exit(-2);	
	}

	accion_SIGALRM.sa_handler=manejadoraPadre;
	accion_SIGALRM.sa_flags=0;
	accion_SIGALRM.sa_mask=conjunto;

	if(sigaction(SIGALRM,&accion_SIGALRM,NULL)==-1){
		perror("Error:sigaction");
		if(kill(getppid(),SIGINT)==-1)
			perror("Error:kill:sigActionALRM");
	}

	//el padre va a esperar a que se hayan creado todos los hijos
	operacionEsperarCero(2);
	//Poner un alarm() con los segundos correspondientes (tiempo)
	alarm(tiempo);

	while(1){
		//recibe el primer mensaje de la cola, se queda esperando hasta que lo reciba		
		if ( msgrcv(ipc.msgid,&mensaje,sizeof(tipoMensajeHijo) - sizeof(long),(long) 'z',0) == -1){
				perror("Error:msgrcv");
				if(kill(getpid(),SIGINT)==-1)
					perror("Error:kill:MensajeRecibido");
				pause();
		}
		//entre los almacenados busca si hay un ciclo
		//si lo hay hace una permuta
		//si no lo hay lo almacena
		hayCiclo(mensaje,soy);
	}

	return 0;
}

//Funcion para saber si hay un ciclo entre los mensajes almacenados
//si lo hay manda mensaje a los hijos, si no almacena el mensaje
void hayCiclo(tipoMensajeHijo mensaje,Cola soy[][4]){
	
	int i, j;								//contadores
	char error[40];							//para pon_error
	tipoMensajePadre mensajeConfirmacion;	//mensaje que se envia a los hijos para confirmar el cambio
	int estoy = mensaje.datos.grupo-1;		//grupo donde esta actualmente el mensaje recibido (-1 porque matrices empiezan en 0)
	int voy = mensaje.datos.destino-1;		//grupo donde quiere ir el hijo del mensaje recibido (-1 porque matrices empiezan en 0)
	char nombre = mensaje.datos.nombre;		//nombre del hijo que mando el mensaje

	//si hay mas mensajes de ese estilo en la cola correspondiente
	//si no se pudieron tratar los otros tampoco este
	if( !colaVacia( &( soy[estoy][voy] ) ) ){
		//almacenamos el mensaje
		colaInserta( &( soy[estoy][voy] ), nombre );
		return;
	}


	//Permuta de 2
	if( !colaVacia( &( soy[voy][estoy] ) ) ){
		//SI hay una permuta de 2: saca el mensaje almacenado y envia los mensajes para el cambio
		//controla que el padre espere a que se hagan las permutas		
		operacionSignal(3,2);
		
		//sacamos el mensaje almacenado y le enviamos el mensaje de confirmacion al hijo
        mensajeConfirmacion.tipo = (long) colaSuprime( &( soy[voy][estoy] ) );
		msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);
        
		mensajeConfirmacion.tipo = (long) nombre;
		msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);
		
		//controla que el padre espere a que se hagan las permutas
		operacionEsperarCero(3);		
		
		return;
		
	}
	//si no hay permuta de 2, buscamos a ver si habría permuta de 3
	for(i=0; i<4; i++){
		//no hay que comprobar ni el grupo de origen ni el grupo de destino
		//solo uno de los otros grupos ya que es una permuta de 3
		if( i!=estoy && i!=voy ){
			if( !colaVacia( &( soy[voy][i] ) ) ){
				//puede haber una permuta de 3
				if( !colaVacia( &( soy[i][estoy] ) ) ){
					//hay una permuta de 3
					operacionSignal(3,3);

					mensajeConfirmacion.tipo = (long) colaSuprime(&(soy[mensaje.datos.destino-1][i]));
					msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);

					mensajeConfirmacion.tipo = (long) colaSuprime(&(soy[i][mensaje.datos.grupo-1]));
					msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);

					mensajeConfirmacion.tipo = (long) nombre;
					msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);

					operacionEsperarCero(3);

					return;
				}
			}
		}
	}
	//buscamos si hay permuta de 4
	for(i=0; i<4; i++){
		if( i!=estoy && i!=voy ){
			if( !colaVacia( &( soy[voy][i] ) ) ){
				for(j=0; j<4; j++){
					if(j!=i && j!=voy && j!=estoy){
						//es el grupo que nos falta comprobar
						if( !colaVacia( &( soy[i][j] ) ) ){
							if( !colaVacia( &( soy[j][estoy] ) ) ){
								//hay una permuta de 4
								operacionSignal(3,4);
	            
                              	mensajeConfirmacion.tipo = (long) colaSuprime( &( soy[voy][i] ) );
                                msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);

							   	mensajeConfirmacion.tipo = (long) colaSuprime( &( soy[i][j] ) );
								msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);
                                        
								mensajeConfirmacion.tipo = (long) colaSuprime( &( soy[j][estoy] ) );    
                         		msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);
                                        
							   	mensajeConfirmacion.tipo = (long) nombre;
							    msgsnd(ipc.msgid,&mensajeConfirmacion, sizeof(tipoMensajePadre) - sizeof(long),0);

								operacionEsperarCero(3);
							     return;
							}			
						}
                   	}
				}
			}
    	}
	}

	//no se ha encontrado ninguna permuta ==> almacenamos el mensaje
	colaInserta(&(soy[mensaje.datos.grupo-1][mensaje.datos.destino-1]),mensaje.datos.nombre);
	return;
}



//funcion para eliminar los mecanismos IPC
void eliminarMecanismosIPC(void){

	if(ipc.semid!=-1){
		if(semctl(ipc.semid,0,IPC_RMID)==-1){
			perror("Error:semctl:IPC_RMID");
		}else ipc.semid=-1;
	}

	if(ipc.shmid!=-1){
		if(shmctl(ipc.shmid,IPC_RMID,NULL)==-1){
			perror("Error:shmctl:IPC_RMID");
		}else ipc.shmid=-1;
	}

	if(ipc.msgid!=-1){
		if(msgctl(ipc.msgid,IPC_RMID,NULL)==-1){
   	    	perror("Error:msgctl:IPC_RMID");
		}else ipc.msgid=-1;
	}

}

// Manejadora para eliminar los procesos, las zonas de memoria compartida y los semaforos
void manejadoraPadre(int sigset){

	int i;
	int valorRetorno;
	
	operacionEsperarCero(3);

	//matamos a todos los hijos
	for(i=0;i<32;i++){
		if(pids[i]==-2) continue;
		if(kill(pids[i],SIGTERM)==-1)
			perror("Error:kill:manejadoraSIGINT");
		waitpid(pids[i],&valorRetorno,0);	
		pids[i]=-2;
	}

	//llamamos a la funcion finCambios
	finCambios();
	//eliminamos los mecanismos ipc
	eliminarMecanismosIPC();
	//acabamos con la vida del padre 
	exit(0);
	
}

//Funcion que da nombres iniciales a los procesos
void iniciarValores(int i, char * pMem, tipoHijo * hijo){
	
	char nombre[]=LETRAS;	
	int grupo;
	int cont;
	
	hijo->nombre = nombre[i];
	if( toupper(hijo->nombre)>='A' && toupper(hijo->nombre)<='D') grupo=1;
	if( toupper(hijo->nombre)>='E' && toupper(hijo->nombre)<='H') grupo=2;
	if( toupper(hijo->nombre)>='I' && toupper(hijo->nombre)<='M') grupo=3;
	if( toupper(hijo->nombre)>='N' && toupper(hijo->nombre)<='R') grupo=4;

	hijo->grupo = grupo;

	//seccion critica para comprobar si la zona de memoria esta libre
	operacionWait(1,1);
		cont=(grupo-1)*20; //empezar en el inicio del grupo, ejemplo G1: va desde la posicion 0 hasta la 20
		while(cont<(grupo*20)){
			if(pMem[cont]==' ') break;  //si encuentra un espacio escribe
			cont+=2;
		}
		hijo->direccion=cont;
		pMem[cont]=hijo->nombre;
		pMem[cont+1]=hijo->grupo;
	operacionSignal(1,1);

}

// Funcion para hacer una operacion Wait sobre un semaforo
void operacionWait(int id, int cuanto){

	struct sembuf sops;

	sops.sem_num=id;
	sops.sem_op= cuanto * (-1);
	sops.sem_flg=0;

	semop(ipc.semid,&sops,1);
}

// Funcion para hacer una operacion signal sobre un semaforo
void operacionSignal(int id, int cuanto){

	struct sembuf sops;

	sops.sem_num=id;
	sops.sem_op=cuanto;
	sops.sem_flg=0;

	semop(ipc.semid,&sops,1);
}
// Funcion para esperar a que en un semaforo no se pueda hacer ningun wait
void operacionEsperarCero(int id){

	struct sembuf sops;

	sops.sem_num=id;
	sops.sem_op=0;
	sops.sem_flg=0;

	semop(ipc.semid,&sops,1);
}

//Funciones para las colas
int colaCreaVacia(Cola *p){
    p->frente=NULL;
    p->fondo=NULL;
    return 0;
}

int colaVacia(Cola *p){
    return(p->frente==NULL);   
}

int colaInserta(Cola *p,tipoElemento elemento){
    tipoCelda * temp;
    
    temp = (tipoCelda *) malloc (sizeof(tipoCelda));
    if(temp==NULL) return -1;
    
    temp->elemento=elemento;
    temp->sig=NULL;
    
    if(colaVacia(p)){
        p->frente=temp;
        p->fondo=temp;
        return 0;
    }
    if(p->fondo!=NULL){
        p->fondo->sig=temp;
        p->fondo=temp;
        return 0;
    }
    
    return -2;   
}

tipoElemento colaSuprime(Cola *p){
    
    tipoCelda * aBorrar;
    tipoElemento x;
    
    if(colaVacia(p)) return x;
    else{
        
        aBorrar=p->frente;
        x = aBorrar->elemento;
        p->frente=p->frente->sig;
	free(aBorrar);
    }
    
    return x;  
}

