#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>		
#include <unistd.h>	
#include <err.h>	
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
	/* Jest zdefiniowane w sys/sem.h */
#else
	union semun {
        int val;						// wartosc dla SETVAL
        struct semid_ds *buf;			// bufor dla IPC_STAT, IPC_SET
        unsigned short int *array;		// tablica dla GETALL, SETALL
        struct seminfo *__buf;			// bufor dla IPC_INFO
	};
#endif



int pid_1;
int pid_2;
int pid_3;
int pid_rodzica;
int zmienna2=1;
int zmienna3=1;

void odblokuj_3(int signal){
	zmienna3=0;
	}

void odblokuj_2(int signal){
		zmienna2=0;
	
}

void przekaz(int signal){
	kill(pid_2, 15);
}



int semid_3;
union semun ctl2;
//wstrzymywanie ==============================

void wstrzymaj2(int signal){
ctl2.val=1;
semctl(semid_3,0,SETVAL,ctl2);
}

void wstrzymaj1(int signal){
ctl2.val=1;
semctl(semid_3,0,SETVAL,ctl2);
}

void wstrzymaj3(int signal){
kill(pid_rodzica,20);
}

void wstrzymaj3_rodzic(int signal){
kill(pid_2, 20);
}


//wznawianie ===============================


void wznow2(int signal){
ctl2.val=0;
semctl(semid_3,0,SETVAL,ctl2);
}

void wznow1(int signal){
ctl2.val=0;
semctl(semid_3,0,SETVAL,ctl2);
}

void wznow3(int signal){
kill(pid_rodzica,21);
}

void wznow3_rodzic(int signal){
kill(pid_2,21);
}

//=============================
int shmid;
int pdes[2];
int semid_1;
int semid_2;


//zakonczenie
void zakoncz(int signal){
kill(pid_rodzica,8);
}
void zakoncz_rodzic(int signal){
kill(pid_1,9);
kill(pid_2,9);
kill(pid_3,9);
shmctl(shmid, IPC_RMID, 0);

semctl(semid_1,0,IPC_RMID,NULL);
semctl(semid_2,0,IPC_RMID,NULL);
semctl(semid_3,0,IPC_RMID,NULL);
close(pdes[0]);
close(pdes[1]);
exit(0);
}




int main(){



//Semafory===================================================================
	
	
	
	union semun ctl;
	
	
	if ((semid_1 = semget(1, 1, IPC_CREAT | 0600)) == -1)
    	errx(2, "Blad tworzenia semafora!");
    	
    if ((semid_2 = semget(2, 1, IPC_CREAT | 0600)) == -1)
    	errx(2, "Blad tworzenia semafora!");
    	
    if ((semid_3 = semget(3, 1, IPC_CREAT | 0600)) == -1)
    	errx(2, "Blad tworzenia semafora!");
    	
    ctl.val = 1;	
    semctl(semid_1, 0,  SETVAL, ctl); 
	semctl(semid_2, 0,  SETVAL, ctl); 
	
	ctl2.val=0;
	semctl(semid_3, 0,  SETVAL, ctl2); 
	
	
	
//===========================================================================
	
//Pamięć Współdzielona ======================================================
	
	
	char* shm;


	
	if ((shmid = shmget(1234, sizeof(char), IPC_CREAT | 0666)) < 0) 
			errx(2, "Blad tworzenia segmentu pamieci dzielonej!");


	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1)	
			errx(3, "Blad przylaczania pamieci dzielonej!");
    	
//============================================================================	
	
	
	
	
//PIPE ===================

	
	pipe(pdes);

//=====================

//Sygnały======================

	
	
	
	
	
	
	
//======================================

//PIDY===============================
pid_rodzica=getpid();
signal(12, przekaz);
signal(20, wstrzymaj3_rodzic);
signal(21, wznow3_rodzic);
signal(8, zakoncz_rodzic);

//==================================
	

unsigned char buf[1];

if((pid_3=fork())){
	if((pid_2=fork())){
		if((pid_1=fork())){
			while(1){}
			
			}
		
		
		
		
		else{
		//PROCES 1
			signal(8, zakoncz);
			signal(21, wznow1);
			signal(20, wstrzymaj1);
			while(1){
				if(read(0,buf,1)>0){
				*shm=buf[0];
				ctl.val = 0;	
				semctl(semid_2, 0,  SETVAL, ctl);
				while(semctl(semid_1, 0,  GETVAL, NULL)){}
				ctl.val=1;
				semctl(semid_1, 0,  SETVAL, ctl);
				}
				else{
				exit(0);	
				}	 
			}	
		}
		}
	
	
	else{
	//PROCES 2
		signal(8, zakoncz);
		signal(21, wznow2);
		signal(15, odblokuj_2);
		signal(20, wstrzymaj2);
		while(1){
			while(semctl(semid_3,0,GETVAL,NULL)){}
			while(semctl(semid_2, 0,  GETVAL, NULL)){}
			buf[0]=*shm;
			close(pdes[0]);	
			write(pdes[1], buf, 1);
			kill(pid_3, 10);
			while(zmienna2){}
			zmienna2=1;
			ctl.val=1;
			semctl(semid_2, 0,  SETVAL, ctl);
			ctl.val=0;
			semctl(semid_1, 0,  SETVAL, ctl);
			
		
		
		}
	}
}
else{
//PROCES 3
	signal(8, zakoncz);
	signal(21, wznow3);
	signal(10, odblokuj_3);
	signal(20,wstrzymaj3);
	while(1){
		while(zmienna3){}
		zmienna3=1;
		close(pdes[1]);
		read(pdes[0],buf,1);
		printf("%x\n",buf[0]);	
		kill(pid_rodzica, 12);
	}	
}	
		






return 0;
}
