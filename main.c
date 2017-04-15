/*
 * main.c
 *
 *  Created on: Apr 15, 2017
 *      Author: Stefan
 */
#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

//Die lange des string mit dem buchstaben
#define STRING_LAENGE 26

//Der puffer
char puffer[10];
//Ein zeiger auf der momentanen position im puffer
int pufferZeiger;

//Der mutex der sich um denn puffer kummert
pthread_mutex_t haupt_mutex = PTHREAD_MUTEX_INITIALIZER;
//Der mutex der denn consumer 1 blockt
pthread_mutex_t consumer_1_m = PTHREAD_MUTEX_INITIALIZER;
//Der mutex der denn consumer 2 blockt
pthread_mutex_t consumer_2_m = PTHREAD_MUTEX_INITIALIZER;

//Der semaphor fuer einfugen in den puffer
sem_t puffer_input;
//Der semaphore fuer rausnehmen aus den puffer
sem_t puffer_output;

void *producer_1_f(void *a){
	char* string = "abcdefghijklmnopqrstuvwxyz";//Der string
	int charZeiger = 0;//Momentane position im string

	while(1){
		pthread_mutex_lock(&consumer_1_m);//check ob der producer 1 geblockt ist
		pthread_mutex_unlock(&consumer_1_m);

		sleep(3);
			sem_wait(&puffer_input);//check ob der puffer frei ist

			//Critical section anfang
			pthread_mutex_lock(&haupt_mutex);
			puffer[pufferZeiger] = string[charZeiger];
			pufferZeiger++;
			pthread_mutex_unlock(&haupt_mutex);

			//Critical section ende
			sem_post(&puffer_output);//erhoht den puffer fuer output so das er weiss man kann draus lessen

			charZeiger++;//Geht zur nachste position im string
			if(charZeiger==STRING_LAENGE){
				charZeiger=0;
			}

	}
	return NULL;
}
//Das selbe wie producer 1
void *producer_2_f(void *a){
	char* string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int charZeiger = 0;

		while(1){
			pthread_mutex_lock(&consumer_2_m);
			pthread_mutex_unlock(&consumer_2_m);
			sleep(3);
					sem_wait(&puffer_input);
					//Critical section anfang
					pthread_mutex_lock(&haupt_mutex);
					puffer[pufferZeiger] = string[charZeiger];
					pufferZeiger++;
					pthread_mutex_unlock(&haupt_mutex);
					//Critical section ende
					sem_post(&puffer_output);

					charZeiger++;
					if(charZeiger==STRING_LAENGE){
						charZeiger=0;
					}
		}
	return NULL;
}

void *consumer_f(void *a){
	char var;//Variable wo man denn puffer element nimt wo man am nachsten schreiben will

	while(1){
				sleep(2);
						sem_wait(&puffer_output);

						//Critical section anfang
						pthread_mutex_lock(&haupt_mutex);
						var = puffer[pufferZeiger-1];
						puffer[pufferZeiger-1]='_';
						pufferZeiger--;
						pthread_mutex_unlock(&haupt_mutex);

						//Critical section ende
						sem_post(&puffer_input);

						printf("%c",var);
						fflush(stdout);

			}
	return NULL;
}

void *control_f(void *a){
	char var;
	int running = 1;
	int consumer_1_isBlocked = 0;
	int consumer_2_isBlocked = 0;

	while(running == 1){
		scanf("%c",&var);
		switch(var){
			case 'q':return NULL;break;
			case '1':if(consumer_1_isBlocked==1){pthread_mutex_unlock(&consumer_1_m); consumer_1_isBlocked = 0;}
					else{pthread_mutex_lock(&consumer_1_m);consumer_1_isBlocked=1;}
					break;
			case '2':if(consumer_2_isBlocked==1){pthread_mutex_unlock(&consumer_2_m); consumer_2_isBlocked = 0;}
						else{pthread_mutex_lock(&consumer_2_m);consumer_2_isBlocked=1;}
						break;
		}
	}
	return NULL;
}

int main(){
	printf("Programm ist gestartet worden\n");
	fflush(stdout);
	int i;
	for(i = 0; i < 10; i++){
		puffer[i] = '_';
	}
	pufferZeiger = 0;

	sem_init(&puffer_input, 0, 10);
	sem_init(&puffer_output, 0, 0);

	pthread_t producer_1_t;
	pthread_t producer_2_t;
	pthread_t consumer_t;
	pthread_t control_t;

	//Startet alle threads
	if(pthread_create(&producer_1_t,NULL,producer_1_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		return -1;
	}
	if(pthread_create(&producer_2_t,NULL,producer_2_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		pthread_cancel(producer_1_t);
		return -1;
	}
	if(pthread_create(&consumer_t,NULL,consumer_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		pthread_cancel(producer_1_t);
		pthread_cancel(producer_2_t);
		return -1;
	}
	if(pthread_create(&control_t,NULL,control_f,NULL)){
		printf("Fehler thread koente nicht gestartet werden");
		pthread_cancel(producer_1_t);
		pthread_cancel(producer_2_t);
		pthread_cancel(consumer_t);
		return -1;
	}


	//Joint alle threads
	if(pthread_join(control_t, NULL)) {
			printf("Fehler beim joinen des threads");
			pthread_cancel(producer_1_t);
			pthread_cancel(producer_2_t);
			pthread_cancel(consumer_t);
			pthread_cancel(control_t);
			return -1;
		}

	//Beendet das programm
	pthread_cancel(producer_1_t);
	pthread_cancel(producer_2_t);
	pthread_cancel(consumer_t);

	sem_destroy(&puffer_input);
	sem_destroy(&puffer_output);

	pthread_mutex_destroy(&haupt_mutex);
	pthread_mutex_destroy(&consumer_1_m);
	pthread_mutex_destroy(&consumer_2_m);

	printf("\nDas program wurde beendet");
	fflush(stdout);

	return 0;
}
