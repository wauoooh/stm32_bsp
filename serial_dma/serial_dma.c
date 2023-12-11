#include "serial_dma.h"


/*
    使用步骤：1、配置好串口模块，并在头文件中改写串口相关定义
            2、调用serialStart()初始化并开始串口接收
            3、通过orderEmpty()判断命令列表是否为空，不为空通过orderGetXXX()获取命令信息，并通过orderDelete()删除命令
*/

/*
	接受数据帧格式: 
    head        | device	| order	    | data      | checkSum or tail
    2bytes      | 1bytes    | 1bytes    | 4bytes    | 1bytes
    0xFF 0xFF   | dev       | ord	    | d         | c
*/

OrderList *orderList1;

void serialStart(void){
    // 开启空闲中断
    __HAL_UART_ENABLE_IT(&serial_uart, UART_IT_IDLE);
    // 初始化结构体
    orderList1 = orderInit();
    // 开启串口DMA接收
    HAL_UART_Receive_DMA(&serial_uart, orderList1->rxBuff, RXBUFF_SIZE);
}

void Serial_IRQHandler(void){
    HAL_UART_IRQHandler(&serial_uart);
    // 如果空闲中断
    if(RESET != __HAL_UART_GET_FLAG(&serial_uart,UART_FLAG_IDLE)){
        __HAL_UART_CLEAR_IDLEFLAG(&serial_uart); //清除标志位

        // 获取接收长度
        orderList1->rxRear = RXBUFF_SIZE - __HAL_DMA_GET_COUNTER(&serial_dma_rx);
        // 解析命令 
        orderAnaly(orderList1, getTime());  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 超时
        // 防止缓存溢出
        if(RXBUFF_SIZE - orderList1->rxRear < 30 ){
            uint32_t len = orderList1->rxRear - orderList1->rxFront;
            memcpy(orderList1->rxBuff, orderList1->rxBuff + orderList1->rxFront, len);
            orderList1->rxFront = 0;
            orderList1->rxRear = len;
            HAL_UART_Receive_DMA(&serial_uart, orderList1->rxBuff+len, RXBUFF_SIZE-len);
        }
    }
}






// 初始化
OrderList *orderInit(void){
    OrderList *ol = (OrderList *)malloc(sizeof(OrderList));
    if(ol==NULL)
        return NULL;
    ol->txBuff = (uint8_t *)malloc(TXBUFF_SIZE);
    ol->rxBuff = (uint8_t *)malloc(RXBUFF_SIZE);
    if(ol->txBuff==NULL || ol->rxBuff==NULL){
        free(ol->txBuff);
        free(ol->rxBuff);
        free(ol);
        return NULL;
    }
    ol->txBuffLength = 0;
    ol->rxFront = 0;
    ol->rxRear = 0;
    ol->head = NULL;
    ol->tail = NULL;

    return ol;
}

// 判断命令列表是否为空
int orderEmpty(OrderList *ol){
    if(ol->head == NULL)
        return 1;
    else
        return 0;
}

// 在尾部插入命令
int orderInsert(OrderList *ol, char device, char order, DataConversion *data){
    OrderListItem *orderItem = (OrderListItem *)malloc(sizeof(OrderListItem));
    if(orderItem==NULL)
        return 0;
    orderItem->device = device;
    orderItem->order = order;
    orderItem->data.u32 = data->u32;
    orderItem->next = NULL;
    if(ol->head==NULL){
        ol->head = orderItem;
        ol->tail = orderItem;
    }
    else{
        ol->tail->next = orderItem;
    }
    return 1;
}

// 删除最早命令
int orderDelete(OrderList *ol){
    if(ol->head==NULL)
        return 0;
    OrderListItem *orderItem = ol->head;
    ol->head = ol->head->next;
    free(orderItem);
    return 1;
}

char orderGetDevice(OrderList *ol){
    if(orderEmpty(ol))
        return 0xFF;
    return ol->head->device;
}
char orderGetOrder(OrderList *ol){
    if(orderEmpty(ol))
        return 0xFF;
    return ol->head->order;
}
int32_t orderGetDatai32(OrderList *ol){
    if(orderEmpty(ol))
        return 0xFF;
    return ol->head->data.i32;
}
uint32_t orderGetDatau32(OrderList *ol){
    if(orderEmpty(ol))
        return 0xFF;
    return ol->head->data.u32;
}
float orderGetDataf32(OrderList *ol){
    if(orderEmpty(ol))
        return 0xFF;
    return ol->head->data.f;
}


// 从接收缓存中解析数据
void orderAnaly(OrderList *ol, uint64_t timeNow){
    // 检测长度
    if(ol->rxRear-ol->rxFront < 9){
        ol->timeLast = timeNow;
        return;
    }
    
    // 检测帧头
    if(ol->rxBuff[ol->rxFront]==0xFF && ol->rxBuff[ol->rxFront+1]==0xFF){
        #ifdef isCheckSum
            char checkSum = 0;
            for(int32_t i=2; i<8;i++)
                checkSum += ol->rxBuff[ol->rxFront+i];
            if(checkSum == ol->rxBuff[ol->rxFront+8]){
                // 所有检验正确
                DataConversion data;
                data.c[0] = ol->rxBuff[ol->rxFront+4];
                data.c[1] = ol->rxBuff[ol->rxFront+5];
                data.c[2] = ol->rxBuff[ol->rxFront+6];
                data.c[3] = ol->rxBuff[ol->rxFront+7];
                orderInsert(ol,ol->rxBuff[ol->rxFront+2],ol->rxBuff[ol->rxFront+3],&data);
            }
            else{
                ol->rxFront++;
                orderAnaly(ol, timeNow);
            }
        #else
            if(END_OF_FRAME == ol->rxBuff[ol->rxFront+8]){
                // 所有检验正确
                DataConversion data;
                data.c[0] = ol->rxBuff[ol->rxFront+4];
                data.c[1] = ol->rxBuff[ol->rxFront+5];
                data.c[2] = ol->rxBuff[ol->rxFront+6];
                data.c[3] = ol->rxBuff[ol->rxFront+7];
                orderInsert(ol,ol->rxBuff[ol->rxFront+2],ol->rxBuff[ol->rxFront+3],&data);
            }
            else{
                ol->rxFront++;
                orderAnaly(ol, timeNow);
            }
        #endif
    }

    // 超时，100ms（需要自己给timeNow赋值）
    if(timeNow-ol->timeLast >= 100){
        while(ol->rxBuff[ol->rxFront]!=0xFF && ol->rxRear-ol->rxFront >= 9)
            ol->rxFront++;
        orderAnaly(ol, timeNow);
    }
}

