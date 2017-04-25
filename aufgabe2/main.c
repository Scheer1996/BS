/*
 * main.c
 *
 *  Created on: Apr 16, 2017
 *      Author: Stefan
 */


#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define SEMAPHORE 1

#include "FIFO.h"


//Die lange des string mit dem buchstaben
#define STRING_LAENGE 26

//Der mutex der sich um denn puffer kummert
static pthread_mutex_t haupt_mutex = PTHREAD_MUTEX_INITIALIZER;

//Der mutex der denn consumer 1 blockt
pthread_mutex_t consumer_1_m = PTHREAD_MUTEX_INITIALIZER;
//Der mutex der denn consumer 2 blockt
pthread_mutex_t consumer_2_m = PTHREAD_MUTEX_INITIALIZER;

#ifdef SEMAPHORE
//Der semaphor fuer einfugen in den puffer
sem_t puffer_input;
//Der semaphore fuer rausnehmen aus den puffer
sem_t puffer_output;

#else
//Die conditional variable fuer eingugen in den puffer
pthread_cond_t cond_p = PTHREAD_COND_INITIALIZER;
//Die conditional variable fuer rausnehmen aus den puffer
pthread_cond_t cond_c = PTHREAD_COND_INITIALIZER;

#endif

void producerFunction(char*string,pthread_mutex_t* consumer_m){

	int charZeiger = 0;//Momentane position im string

		while(1){
			pthread_mutex_lock(consumer_m);//check ob der producer 1 geblockt ist
			pthread_mutex_unlock(consumer_m);
			sleep(3);

				#ifdef SEMAPHORE
					sem_wait(&puffer_input);//check ob der puffer frei ist

					pthread_mutex_lock(&haupt_mutex);
					FIFO_push(string[charZeiger]);
					pthread_mutex_unlock(&haupt_mutex);

					sem_post(&puffer_output);

				#else
					//Critical section anfang
					pthread_mutex_lock(&haupt_mutex);
					while(FIFO_getLength() == 10) pthread_cond_wait(&cond_p,&haupt_mutex);//Condition variable
					FIFO_push(string[charZeiger]);
					pthread_mutex_unlock(&haupt_mutex);
					//Critical section ende

					pthread_cond_signal(&cond_c);

				#endif

				charZeiger++;//Geht zur nachste position im string
				if(charZeiger==STRING_LAENGE){
					charZeiger=0;
				}
		}

}

void *producer_1_f(void *a){
	char* string = "abcdefghijklmnopqrstuvwxyz";//Der string
	producerFunction(string,&consumer_1_m);
	return NULL;
}
//Das selbe wie producer 1
void *producer_2_f(void *a){
	char* string = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	producerFunction(string,&consumer_2_m);
	return NULL;
}

void *consumer_f(void *a){
	char var;//Variable wo man denn puffer element nimt wo man am nachsten schreiben will

	while(1){
				sleep(2);
					#ifdef SEMAPHORE
						sem_wait(&puffer_output);;//check ob der puffer frei ist

						pthread_mutex_lock(&haupt_mutex);
						var = FIFO_pop();
						pthread_mutex_unlock(&haupt_mutex);

						sem_post(&puffer_input);
					#else
						//Critical section anfang
						pthread_mutex_lock(&haupt_mutex);
						while(FIFO_getLength() == 0) pthread_cond_wait(&cond_c,&haupt_mutex);
						var = FIFO_pop();
						pthread_mutex_unlock(&haupt_mutex);
						//Critical section ende
						pthread_cond_signal(&cond_p);

					#endif
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
	FIFO_init();

	#ifdef SEMAPHORE
		sem_init(&puffer_input, 0, 10);
		sem_init(&puffer_output, 0, 0);
	#endif

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

	#ifdef SEMAPHORE
		sem_destroy(&puffer_input);
		sem_destroy(&puffer_output);
	#else
		pthread_cond_destroy(&cond_c);
		pthread_cond_destroy(&cond_p);
	#endif

	pthread_mutex_destroy(&consumer_1_m);
	pthread_mutex_destroy(&consumer_2_m);
	pthread_mutex_destroy(&haupt_mutex);

	printf("\nDas program wurde beendet");
	fflush(stdout);

	return 0;
}



