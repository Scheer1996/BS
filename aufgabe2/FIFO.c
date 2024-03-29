/** ****************************************************************
 * @file    aufgabe2/fifo.c
 * @author  Stefan Belic (Stefan.Belic@haw-hamburg.de)
 * @author  Philip Scheer (Philip.Scheer@haw-hamburg.de)
 * @version 1.0
 * @date    26.04.2017
 * @brief   FIFO Buffer
 ******************************************************************
 */

#include "FIFO.h"
#include<stdio.h>

static char puffer[PUFFER_SIZE];
static int pufferPushZeiger;
static int pufferPopZeiger;
static int length;

void FIFO_init(void){
	int i;
	for(i = 0; i < PUFFER_SIZE; i++){
			puffer[i] = '_';
		}
	pufferPushZeiger = 0;
	pufferPopZeiger = 0;
	length = 0;
}

void FIFO_push(char neuerChar){
	puffer[pufferPushZeiger] = neuerChar;
	printf("%c wurde eingefugt\n",neuerChar);
	fflush(stdout);
	pufferPushZeiger++;
	if(pufferPushZeiger == PUFFER_SIZE){
		pufferPushZeiger = 0;
	}
	length++;
}

char FIFO_pop(void){
	char popChar = puffer[pufferPopZeiger];
	printf("%c wurde entommen\n",popChar);
	fflush(stdout);
	puffer[pufferPopZeiger]='_';
	pufferPopZeiger++;
		if(pufferPopZeiger == PUFFER_SIZE){
			pufferPopZeiger = 0;
		}
	length--;
	return popChar;
}

int FIFO_getLength(void){
	return length;
}
