#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void SIGUSR1_handler(int); // SIGUSR1
void SIGTERM_handler(int); // SIGTERM --> 39, 40, 41
void SIGTERMR_handler(int); // SIGTERM el resto

pid_t siguiente;
pid_t siguiente1;
pid_t p1, p2, p3, p4, p5, p6, p7, p8, p9;


int main (int argc, char* argv[]){


	char nombreInvalido[]="El nombre del ejecutable no es el correcto\n";
	char argumentoInvalido[]="No introduzcas argumentos\n";
	struct	sigaction action_USR1, action_TERM, action_TERMR;
	sigset_t mi_mascara, back_up;

    int archivo;
    int *fich;
    pid_t x;


	if(strcmp(argv[0], "./dom")){
		if(write(1,nombreInvalido,sizeof(nombreInvalido))==-1) return 1;
		return 1;
	}
	if(argc>1){
		if(write(1,argumentoInvalido,sizeof(argumentoInvalido))==-1) return 1;
		return 1;
	}


	action_USR1.sa_handler=SIGUSR1_handler;
	action_USR1.sa_flags=0;
	sigemptyset(&action_USR1.sa_mask);
	if(sigaction(SIGUSR1,&action_USR1,NULL)==-1){
		perror("SIGACTION USR1");
		exit(1);
	}

	action_TERMR.sa_handler=SIGTERMR_handler;
	action_TERMR.sa_flags=0;
	sigemptyset(&action_TERMR.sa_mask);
	if(sigaction(SIGTERM,&action_TERMR,NULL)==-1){
		perror("SIGACTION USR1");
		exit(1);
	}

	sigfillset(&mi_mascara);
	sigdelset(&mi_mascara,SIGUSR1);
	sigdelset(&mi_mascara,SIGTERM);

	sigprocmask(SIG_SETMASK,&mi_mascara,&back_up);
	
    x=getpid();
    archivo=open("prueba.txt", O_RDWR | O_CREAT, 0600);
    write(archivo,&x,sizeof(int)*6);
    fich=(int*)mmap(0, sizeof(pid_t)*6, PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
    
    close(archivo);
    
    fich[0]=0;
    fich[1]=0;
    fich[2]=0;
    
    
    if (fich == MAP_FAILED) {
	    perror("mmap:error");
        return -1;
	}

    p1=getpid();
	// proceso 37

	switch(siguiente=fork()){

		case -1: 
			perror("dom:fork"); return 2;
		case 0:
			// proceso 38
			p2=getpid();

			switch(siguiente=fork()){

				case -1: 
					perror("dom:fork"); return 2;
				case 0:
					// proceso 39
					p3=getpid();
					switch(siguiente=fork()){

						case -1: 
							perror("dom:fork"); return 2;
						case 0:
							// proceso 40
							switch(siguiente=fork()){

								case -1:

									perror("dom:fork"); return 2;

								case 0:
									// proceso 42
									switch(siguiente=fork()){

										case -1:
											perror("dom:fork"); return 2;
										case 0:
											// proceso 46
											switch(siguiente=fork()){

												case -1:
													perror("dom:fork"); return 2;
												case 0:
													// proceso 50
													switch(siguiente=fork()){

														case -1:
															perror("dom:fork"); return 2;
														case 0:
															// proceso 54
															fich[3]=getpid();
															p7=getpid();
															switch(siguiente=fork()){

																case -1:
																	perror("dom:fork"); return 2;
																case 0:
																	// proceso 56
																	
																	fich[5]=getpid();
																	p9=getpid();
																	siguiente=0;
																	switch(siguiente=fork()){

																		case -1:
																			perror("dom:fork"); return 2;
																		case 0:
																			// proceso 57
																			switch(siguiente=fork()){

																				case -1:
																					perror("dom:fork"); return 2;
																				case 0:
																					// proceso 58
																					break;

																				default:
																					break;
																			}
																			break;

																		default:
																			break;
																	}
																	break;

																default:
																
																    siguiente=0;
																	break;
															}
															break;

														default:
														
														    siguiente=0;
															break;
													}
													break;

												default:
													break;
											}
											break;

										default:
											break;
									}
									break;

								default:
									// proceso 40
									switch(siguiente1=fork()){

										case -1:

											perror("dom:fork"); return 2;

										case 0:
											// proceso 43
											switch(siguiente=fork()){

												case -1:
													perror("dom:fork"); return 2;
												case 0:
													// proceso 47
													switch(siguiente=fork()){

														case -1:
															perror("dom:fork"); return 2;
														case 0:
															// proceso 51
															
															fich[1]=getpid();
															p4=getpid();
															break;

														default:
															break;
													}
													break;

												default:
													break;
											}
											break;

										default:
											// proceso 40
											action_TERM.sa_handler=SIGTERM_handler;
											action_TERM.sa_flags=0;
											sigemptyset(&action_TERM.sa_mask);
											if(sigaction(SIGTERM,&action_TERM,NULL)==-1){
												perror("SIGACTION USR1");
												exit(1);
											}
											break;
									}
									break;

							}
							break;
						default:
							// proceso 39
							switch(siguiente1=fork()){

								case -1: 
									perror("dom:fork"); return 2;
								case 0:
									// proceso 41
									switch(siguiente=fork()){

										case -1:
											perror("dom:fork"); return 2;
										case 0:
											// proceso 44
											switch(siguiente=fork()){

												case -1:
													perror("dom:fork"); return 2;
												case 0:
													// proceso 48
													switch(siguiente=fork()){

														case -1:
															perror("dom:fork"); return 2;
														case 0:
															// proceso 52
															switch(siguiente=fork()){

																case -1:
																	perror("dom:fork"); return 2;
																case 0:
																	// proceso 55
																	fich[4]=getpid();
																	p6=getpid();
																	p8=getpid();
																	break;

																default:
																    siguiente=0;
																	break;
															}
															break;

														default:
															break;
													}
													break;

												default:
													break;
											}
											break;

										default:
											// proceso 41
											switch(siguiente1=fork()){

												case -1:
													perror("dom:fork"); return 2;
												case 0:
													// proceso 45
													switch(siguiente=fork()){

														case -1:
															perror("dom:fork"); return 2;
														case 0:
															// proceso 49
															switch(siguiente=fork()){

																case -1:
																	perror("dom:fork"); return 2;
																case 0:
																	// proeso 53
																	
																	fich[2]=getpid();
																	p5=getpid();
																	break;

																default:
																	break;
															}
															break;

														default:
															break;
													}
													break;

												default:
													// proceso 41
													action_TERM.sa_handler=SIGTERM_handler;
													action_TERM.sa_flags=0;
													sigemptyset(&action_TERM.sa_mask);
													if(sigaction(SIGTERM,&action_TERM,NULL)==-1){
														perror("SIGACTION USR1");
														exit(1);
													}
													break;
											}
											break;
									}
									break;
								default:
									// proceso 39
									action_TERM.sa_handler=SIGTERM_handler;
									action_TERM.sa_flags=0;
									sigemptyset(&action_TERM.sa_mask);
									if(sigaction(SIGTERM,&action_TERM,NULL)==-1){
										perror("SIGACTION USR1");
										exit(1);
									}
									break;
							}	
							break;
						}
					break;
				default:
					break;
			}
			break;
		default:
			break;
}

    // padres

    if(getpid()==p1){
	    sigsuspend(&mi_mascara);
	    kill(fich[1],SIGUSR1);  
    }
    
    if(getpid()==p2){
        sigsuspend(&mi_mascara);
        kill(fich[2],SIGUSR1);   
    }
    
    if(getpid()==p3){
    	sigsuspend(&mi_mascara);
    	kill(fich[4],SIGUSR1); 
    }
    
    
    // lectores
    if(getpid()==p4){
        sigsuspend(&mi_mascara);
        siguiente=fich[3];
    }
    if(getpid()==p5){
        sigsuspend(&mi_mascara);
        siguiente=fich[4];
    }
    if(getpid()==p6){
        sigsuspend(&mi_mascara);
        siguiente=fich[5];
    }
    
    // escritores
    if(getpid()==p7){
    kill(p1,SIGUSR1);
    
    }
    if(getpid()==p8){
    kill(p2,SIGUSR1);
    
    }
    if(getpid()==p9){
    kill(p3,SIGUSR1);
    
    }

	sigsuspend(&mi_mascara);

	sigprocmask(SIG_SETMASK,&back_up,&mi_mascara);
	
    /*if (munmap(fich, sizeof(int)*3) == -1) {
	    perror("Error un-mmapping the file");
    }*/
    
	return 0;
}



void SIGUSR1_handler(int sigset) {}// No hace nada

void SIGTERM_handler(int sigset) // Envía la señal a dos procesos
{
	kill(siguiente,SIGTERM);
	kill(siguiente1,SIGTERM);
}

void SIGTERMR_handler(int sigset) // Envía la señal a un proceso
{
	if(siguiente!=0) kill(siguiente,SIGTERM);
}
