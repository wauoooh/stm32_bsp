#ifndef __SEQ_QUEUE_H__
#define __SEQ_QUEUE_H__

#include <stdlib.h>
#include "stm32f4xx_hal.h"

typedef int ElemType;

typedef struct{
    ElemType *data;
    int maxSize;
    int front;
    int rear;
}SeqQueue;


SeqQueue* queueCreate(int queueSize);
int queueLength(SeqQueue *Q);
int queueEmpty(SeqQueue *Q);
int queueFull(SeqQueue *Q);
int queuePush(SeqQueue *Q, ElemType data);
int queuePop(SeqQueue *Q, ElemType *data);



#endif // __SEQ_QUEUE_H__

