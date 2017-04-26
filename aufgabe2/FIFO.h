/** ****************************************************************
 * @file    aufgabe2/fifo.h
 * @author  Stefan Belic (Stefan.Belic@haw-hamburg.de)
 * @author  Philip Scheer (Philip.Scheer@haw-hamburg.de)
 * @version 1.0
 * @date    26.04.2017
 * @brief   FIFO Buffer
 ******************************************************************
 */
#ifndef FIFO_H_
#define FIFO_H_

void FIFO_push(char);
char FIFO_pop(void);
void FIFO_init(void);
int FIFO_getLength(void);

#endif /* FIFO_H_ */

