#ifndef __SERIAL_DMA_H__
#define __SERIAL_DMA_H__

#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "main.h"
#include "usart.h"

#define serial_uart         huart1
#define serial_dma_rx       hdma_usart1_rx
#define Serial_IRQHandler   USART1_IRQHandler



// #define isCheckSum
#define END_OF_FRAME 0x3A

#define TXBUFF_SIZE  100
#define RXBUFF_SIZE  100


typedef union
{
    uint8_t c[4];
    uint32_t u32;
    int32_t i32;
    float f;
}DataConversion;

typedef struct OrderListItem{
    uint8_t device;    // 设备号
    uint8_t order;     // 命令
    DataConversion data;    // 与命令对应数据
    struct OrderListItem *next;    // 指向下一个命令
}OrderListItem;

typedef struct{
    uint8_t *txBuff;
    uint8_t *rxBuff;
    __IO uint32_t txBuffLength;
    __IO uint32_t rxFront;
    __IO uint32_t rxRear;

    __IO uint64_t timeNow;
    __IO uint64_t timeLast;

    OrderListItem *head;
    OrderListItem *tail;
}OrderList;



extern OrderList *orderList1;

void serialStart(void);
void Serial_IRQHandler(void);

OrderList *orderInit(void);
int orderEmpty(OrderList *ol);
int orderInsert(OrderList *ol, char device, char order, DataConversion *data);
int orderDelete(OrderList *ol);

char orderGetDevice(OrderList *ol);
char orderGetOrder(OrderList *ol);
int32_t orderGetDatai32(OrderList *ol);
uint32_t orderGetDatau32(OrderList *ol);
float orderGetDataf(OrderList *ol);

void orderAnaly(OrderList *ol);



#endif // __SERIAL_DMA_H__

