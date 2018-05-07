#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define VALUE 30
typedef struct{

	pid_t sigPID;
	pid_t sigAlternative;
	pid_t H1,H2,H3,H4,N2,N3;
	int num;

} YO;

int vueltas=0;
YO personal_data;

void SIGUSR1_handler(int);
void SIGUSR2_handler(int);
void SIGALRM_handler(int);
void EMPTY_handler(int);

int main (int argc, char* argv[]){

	int nombre_incorrecto, demasiados_argumentos, value;
	char nombreInvalido[]="El nombre del ejecutable no es el correcto\n";
	char argumentoInvalido[]="No introduzcas argumentos\n";
	struct	sigaction action_USR1, action_Alarm, action_USR2;
	sigset_t set_to_USR, set_to_restore;
	pid_t P;

	if(strcmp(argv[0], "./infinito")){
		if(write(1,nombreInvalido,sizeof(nombreInvalido))==-1) return 1;
		return 1;
	}
	if(argc>1){
		if(write(1,argumentoInvalido,sizeof(argumentoInvalido))==-1) return 1;
		return 1;
	}

	action_USR1.sa_handler=EMPTY_handler;
	action_USR1.sa_flags=0;
	sigemptyset(&action_USR1.sa_mask);
	if(sigaction(SIGUSR1,&action_USR1,NULL)==-1){
		perror("SIGACTION USR1");
		exit(1);
	}

	action_Alarm.sa_handler=SIGALRM_handler;
	action_Alarm.sa_flags=0;
	sigemptyset(&action_Alarm.sa_mask);
	if(sigaction(SIGALRM,&action_Alarm,NULL)==-1){
		perror("SIGACTION ALARM");
		exit(1);
	}

	sigfillset(&set_to_USR);
	sigdelset(&set_to_USR,SIGUSR1);
	sigdelset(&set_to_USR,SIGALRM);
	sigdelset(&set_to_USR,SIGTERM);
	sigdelset(&set_to_USR,SIGUSR2);

	sigprocmask(SIG_SETMASK,&set_to_USR,&set_to_restore);
	
	personal_data.num=1;
	P=getpid();

	switch(personal_data.H1=fork()){

        case -1:
            perror("infinito:fork"); return 2;
		case 0:

			personal_data.sigPID=getppid();
			personal_data.num=2;
			break;

		default:

			switch(personal_data.H4=fork()){
                
                case -1:
                    perror("infinito:fork"); return 7;
                case 0:

                    personal_data.sigPID=getppid();
                    personal_data.num=7;
                    break;

                default:

                    switch(personal_data.H2=fork()){

                        case -1:
                            perror("infinito:fork"); return 3;
                        case 0:

                            switch(personal_data.N2=fork()){

                                case -1:
                                    perror("infinito:fork"); return 4;
                                case 0:
                                    personal_data.sigPID=personal_data.H1;
                                    personal_data.num=4;
                                    break;
                                default:
                                    personal_data.sigPID=personal_data.N2;
                                    personal_data.num=3;
                            }
							break;

                        default:

                            switch(personal_data.H3=fork()){

                                case -1:
                                    perror("infinito:fork"); return 5;
                                case 0:

                                    switch(personal_data.N3=fork()){

                                        case -1:
                                            perror("infinito:fork"); return 6;
                                        case 0:

                                            personal_data.sigPID=personal_data.H4;
                                            personal_data.num=6;
                                    	    break;

                                        default:

                                            personal_data.sigPID=personal_data.N3;
                                            personal_data.num=5;
                                    }
                                    break;

								default:
                                    personal_data.sigPID=personal_data.H3;
                                    personal_data.sigAlternative=personal_data.H2;
                            }
                    }
            }
	}
	switch(personal_data.num){

		case 1:
			alarm(1);
			sigsuspend(&set_to_USR);
			personal_data.num=1;
			action_USR2.sa_handler=SIGUSR2_handler;
			action_USR2.sa_flags=0;
			sigemptyset(&action_USR2.sa_mask);
			if(sigaction(SIGUSR2,&action_USR2,NULL)==-1){
				perror("SIGACTION USR2");
				exit(1);
			}
			action_USR1.sa_handler=SIGUSR1_handler;
			action_USR1.sa_flags=0;
			sigemptyset(&action_USR1.sa_mask);
			if(sigaction(SIGUSR1,&action_USR1,NULL)==-1){
				perror("SIGACTION USR1");
				exit(1);
			}
			alarm(VALUE);
			kill(personal_data.sigPID,SIGUSR1);
			break;
		default:
			action_USR1.sa_handler=SIGUSR1_handler;
			action_USR1.sa_flags=0;
			sigemptyset(&action_USR1.sa_mask);
			if(sigaction(SIGUSR1,&action_USR1,NULL)==-1){
				perror("SIGACTION USR1");
				exit(1);
			}
	}
	switch(personal_data.num){
		case 6:
			kill(P,SIGUSR1);
			break;
	}
	while(personal_data.num){
		sigsuspend(&set_to_USR);
	}

	kill(personal_data.sigPID,SIGALRM);
	kill(personal_data.sigAlternative,SIGALRM);
	
	kill(personal_data.H1,SIGTERM);
	waitpid(personal_data.H1,&value,0);

	kill(personal_data.H4,SIGTERM);
	waitpid(personal_data.H4,&value,0);

	kill(personal_data.H2,SIGTERM);
	waitpid(personal_data.H2,&value,0);

	kill(personal_data.H3,SIGTERM);
	waitpid(personal_data.H3,&value,0);
	
	printf("Hemos dado %d vueltas\n", vueltas);

	sigprocmask(SIG_SETMASK,&set_to_restore,NULL);

	return 0;
}

void SIGUSR1_handler(int sigset){

	switch(personal_data.num){
		case 0:
			break;
		case 7:
			kill(personal_data.sigPID,SIGUSR2);
			break;
		default:
			kill(personal_data.sigPID,SIGUSR1);
	}
}

void SIGUSR2_handler(int sigset){

	vueltas++;
	kill(personal_data.sigAlternative,SIGUSR1);

}

void SIGALRM_handler(int sigset){

	int x;

	switch(personal_data.num){
		case 1:
			personal_data.num=0;
			break;
		default:
			kill(personal_data.sigPID,SIGTERM);
			waitpid(personal_data.sigPID,&x,0);
	}
}

void EMPTY_handler(int sigset){}
