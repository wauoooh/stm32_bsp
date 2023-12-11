#include "seq_queue.h"

// 创建队列，queueSize:队列最大数据个数
SeqQueue* queueCreate(int queueSize){
    SeqQueue *Q = (SeqQueue *)malloc(sizeof(SeqQueue));
    if(Q == NULL)
        return NULL;
    Q->data = (ElemType *)malloc((queueSize+1)*sizeof(ElemType));
    if(Q->data == NULL)
        return NULL;
    Q->maxSize = queueSize+1;
    Q->front = 0;
    Q->rear = 0;
    return Q;
}

// 求队列长度
int queueLength(SeqQueue *Q){
    return (Q->rear-Q->front+Q->maxSize) % Q->maxSize;
}


// 判断队列是否为空
int queueEmpty(SeqQueue *Q){
    return Q->front == Q->rear;
}

// 判断队列是否满
int queueFull(SeqQueue *Q){
    return (Q->rear+1)%Q->maxSize == Q->front;
}

// 入队
int queuePush(SeqQueue *Q, ElemType data){
    if(queueFull(Q))
        return 0;
    Q->data[Q->rear] = data;
    Q->rear = (Q->rear+1) % Q->maxSize;
    return 1;
}

// 出队
int queuePop(SeqQueue *Q, ElemType *data){
    if(queueEmpty(Q))
        return 0;
    *data = Q->data[Q->front];
    Q->front = (Q->front+1) % Q->maxSize;
    return 1;
}



