
/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: uart_update.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: UART 기반 펌웨어 업데이트/통신 상태 관리 함수 구현
 */



#include "uart_update.h"
#include "flash_engine.h" // 공용 플래시 엔진
#include "crc.h"          // CRC 계산
#include <stdbool.h>


// ==================================================
//                  외부 핸들 참조
// ==================================================
extern UART_HandleTypeDef ANDROID_UART_HANDLE;
extern UART_HandleTypeDef DBUG_UART_HANDLE;


// ==================================================
//                  내부 변수
// ==================================================
static uint8_t rx_buffer[1024 + 64];
static UpdateState_t current_state = UPDATE_READY;
static uint32_t write_addr = APP_START_ADDR;
static bool is_uart_init = false;
static uint32_t last_request_tick = 0; // 마지막 요청 시간 저장

/* -------------------------------------------------------------------------- */
/*                  UART 업데이트 모듈 초기화 함수                             */
/* -------------------------------------------------------------------------- */
/**
 * @brief  UART 업데이트 모듈 초기화
 * @retval true: 성공, false: 실패
 */
bool uartUpdateInit(void)
{
    // 1. 상태 초기화
    is_uart_init = true;
    current_state = UPDATE_READY;
    write_addr = APP_START_ADDR;

    // 2. IDLE 인터럽트 활성화 (패킷 끝 감지용)
    __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);

    // 3. DMA 수신 시작
    if (HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, rx_buffer, sizeof(rx_buffer)) != HAL_OK) {
        return false;
    }
    return true;
}


/* -------------------------------------------------------------------------- */
/*                  UART 업데이트 처리 함수 (메인 루프)                        */
/* -------------------------------------------------------------------------- */
/**
 * @brief  UART 업데이트 처리 (메인 루프에서 반복 호출)
 */
void uartUpdateProcess(void)
{
    uint32_t current_tick = HAL_GetTick();

    // 1. FLAG_NEW 상태일 때 1초마다 Ada에게 요청 메시지 전송
    if (Get_Flag() == FLAG_NEW)
    {
        if (current_tick - last_request_tick > 1000) // 1000ms 주기
        {
            sendFwRequestToAda();
            last_request_tick = current_tick;
            
            // 디버그용 LED 반전 (동작 확인용)
            LL_GPIO_TogglePin(DBG_LED_GPIO_Port, DBG_LED_Pin); 
            // 아주 짧은 부저음 (10ms)
            LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
            HAL_Delay(10); // 10ms 정도는 시스템에 큰 지장 없음
            LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
        }
    }

    // 2. 패킷 수신 처리
    if (uartAvailable() > 0) {
        if (parsePacket(rx_buffer)) {
            uint8_t cmd = rx_buffer[1];
            uint16_t data_len = (rx_buffer[2] << 8) | rx_buffer[3];

            // Ada가 0x01(START) 패킷을 보내면, 그때부터 FLAG_ING로 바뀌며 
            // 위의 'NEW' 요청 로직은 자동으로 멈춥니다.
            switch(cmd) {
                case 0x01: // START: 업데이트 시작 알림
                    HAL_FLASH_Unlock();
                    if (Erase_App_Sectors() == HAL_OK) {
                        Update_Flag(FLAG_ING);  // 여기서 상태가 바뀌므로 Poll 중단됨
                        write_addr = APP_START_ADDR;
                        sendAckToAda();
                        current_state = UPDATE_ING;

                        // 시작 알림: 짧게 한 번 "삑!"
                        LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                        HAL_Delay(100);
                        LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                    }
                    break;

                case 0x02: // DATA: 실제 데이터 기록 구간
                {
                    uint32_t received_crc = *(uint32_t*)&rx_buffer[4 + data_len];
                    if (crcCalculate(CRC_32, &rx_buffer[4], data_len) == received_crc) {

                        // [추가] Flash 기록 시작 전 부저 ON
                        LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);

                        if (Write_Flash(write_addr, &rx_buffer[4], data_len) == HAL_OK) {
                            write_addr += data_len;

                            // [추가] 기록 완료 후 부저 OFF
                            LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);

                            sendAckToAda();
                        } else {
                            // 에러 시 부저 끄기
                            LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                        }
                    } else {
                        sendNackToAda();
                    }
                    break;
                }

                case 0x03: // END: 모든 전송 완료
                    if (Is_App_Valid()) {
                        Update_Flag(FLAG_PASS);
                        current_state = UPDATE_PASS;
                        sendAckToAda();

                        // 성공 알림: 길게 "삐이이-"
                        LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                        HAL_Delay(500);
                        LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                    }
                    HAL_FLASH_Lock();
                    break;
            }
        }
    }
}


/* -------------------------------------------------------------------------- */
/*                  수신 데이터/패킷 파싱 함수                                */
/* -------------------------------------------------------------------------- */
/**
 * @brief  수신된 데이터 바이트 수 반환
 * @retval 남은 바이트 수
 */
uint32_t uartAvailable(void)
{
    return (sizeof(rx_buffer) - __HAL_DMA_GET_COUNTER(ANDROID_UART_HANDLE.hdmarx));
}

/**
 * @brief  패킷 파싱: [STX][CMD][LEN_H][LEN_L][DATA...][CRC32][ETX]
 * @param  p_buffer  패킷 데이터 포인터
 * @retval true: 정상 패킷, false: 오류
 */
bool parsePacket(uint8_t *p_buffer)
{
    if (uartAvailable() < 8) return false;
    if (p_buffer[0] == 0x02 && p_buffer[uartAvailable() - 1] == 0x03) {
        return true;
    }
    return false;
}


/* -------------------------------------------------------------------------- */
/*                  현재 업데이트 상태 반환 함수                              */
/* -------------------------------------------------------------------------- */
/**
 * @brief  현재 업데이트 상태 반환
 * @retval UpdateState_t (READY/ING/PASS/FAIL)
 */
UpdateState_t uartGetState(void)
{
    return current_state;
}


/* -------------------------------------------------------------------------- */
/*                  요청/성공/실패 응답 전송 함수                                 */
/* -------------------------------------------------------------------------- */
/**
 * @brief  안드로이드(Ada)에게 펌웨어 전송을 요청하는 패킷 송신
 */
void sendFwRequestToAda(void)
{
    // [STX(0x02)] [CMD_REQ(0x05)] [LEN(0x00)] [ETX(0x03)] 형태로 정의
    uint8_t req_pkt[] = {0x02, 0x05, 0x00, 0x03}; 
    HAL_UART_Transmit(&ANDROID_UART_HANDLE, req_pkt, sizeof(req_pkt), 100);
}

/**
 * @brief  ADA(상위)로 ACK 전송
 */
void sendAckToAda(void)
{
    uint8_t ack_pkt[] = {0x02, 0x06, 0x00, 0x03};
    HAL_UART_Transmit(&ANDROID_UART_HANDLE, ack_pkt, sizeof(ack_pkt), 100);
}

/**
 * @brief  ADA(상위)로 NACK 전송
 */
void sendNackToAda(void)
{
    uint8_t nack_pkt[] = {0x02, 0x15, 0x00, 0x03};
    HAL_UART_Transmit(&ANDROID_UART_HANDLE, nack_pkt, sizeof(nack_pkt), 100);
}


/* -------------------------------------------------------------------------- */
/*                  printf 리다이렉션 함수                                     */
/* -------------------------------------------------------------------------- */
/**
 * @brief  printf 리다이렉션 (디버그 UART)
 * @param  ch   출력 문자
 * @retval 항상 1
 */
int __io_putchar(int ch)
{
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&DBUG_UART_HANDLE, &data, 1, 10);
    return 1;
}

