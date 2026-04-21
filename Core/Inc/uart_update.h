/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: uart_update.h
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: UART 기반 펌웨어 업데이트/통신 상태 관리 API
 */


#ifndef INC_UART_UPDATE_H_
#define INC_UART_UPDATE_H_


#include "main.h"
#include <stdbool.h>


// ==================================================
//                  매크로 정의
// ==================================================

#define ANDROID_UART_HANDLE   huart2   // 안드로이드 통신용 UART 핸들
#define DBUG_UART_HANDLE      huart3   // 디버그용 UART 핸들


// ==================================================
//                  타입 정의
// ==================================================

/**
 * @brief  업데이트 상태 열거형
 */
typedef enum {
    UPDATE_READY = 0,   // 준비 상태
    UPDATE_ING,         // 업데이트 진행 중
    UPDATE_PASS,        // 업데이트 성공
    UPDATE_FAIL         // 업데이트 실패
} UpdateState_t;


// ==================================================
//                  외부 API
// ==================================================

/**
 * @brief  UART 업데이트 모듈 초기화
 * @retval true: 성공, false: 실패
 */
bool uartUpdateInit(void);

/**
 * @brief  UART 업데이트 처리 (메인 루프에서 반복 호출)
 */
void uartUpdateProcess(void);

/**
 * @brief  현재 업데이트 상태 반환
 * @retval UpdateState_t (READY/ING/PASS/FAIL)
 */
UpdateState_t uartGetState(void);

/**
 * @brief  수신 버퍼에 남아있는 데이터 개수 반환
 * @retval 남은 바이트 수
 */
uint32_t uartAvailable(void);

/**
 * @brief  수신 패킷 파싱
 * @param  p_buffer  패킷 데이터 포인터
 * @retval true: 정상 패킷, false: 오류
 */
bool parsePacket(uint8_t *p_buffer);

/**
 * @brief  ADA(상위)로 ACK 전송
 */
void sendAckToAda(void);

/**
 * @brief  ADA(상위)로 NACK 전송
 */
void sendNackToAda(void);


#endif /* INC_UART_UPDATE_H_ */
