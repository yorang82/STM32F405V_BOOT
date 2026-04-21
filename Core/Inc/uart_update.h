/*
 * uart_update.h
 *
 *  Created on: 2026. 4. 21.
 *      Author: YK_Cho
 */

#ifndef INC_UART_UPDATE_H_
#define INC_UART_UPDATE_H_

#include "main.h"
#include <stdbool.h>

// 업데이트 상태 정의
typedef enum {
    UPDATE_READY,
    UPDATE_ING,
    UPDATE_PASS,
    UPDATE_FAIL
} UpdateState_t;

bool uartUpdateInit(void);
void uartUpdateProcess(void); // 메인 루프에서 호출
UpdateState_t uartGetState(void);
void sendAckToAda(void);
void sendNackToAda(void);

#endif /* INC_UART_UPDATE_H_ */
