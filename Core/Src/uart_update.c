/*
 * uart_update.c
 *
 *  Created on: 2026. 4. 21.
 *      Author: YK_Cho
 */


#include "uart_update.h"
#include "flash_engine.h" // 이전에 만든 공용 플래시 엔진
#include "crc.h"          // App에서 가져온 CRC 소스

/* main.c의 핸들러 참조 (태블릿 연결 포트 확인 필요) */
extern UART_HandleTypeDef huart2;

static uint8_t rx_buffer[1024 + 64];
static UpdateState_t current_state = UPDATE_READY;
static uint32_t write_addr = APP_START_ADDR;
static bool is_uart_init = false;

bool uartUpdateInit(void) {
    // 1. 상태 초기화
    is_uart_init = true;
    current_state = UPDATE_READY;
    write_addr = APP_START_ADDR; // flash_engine.h 정의

    // 2. IDLE 인터럽트 활성화 (패킷 끝 감지용)
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);

    // 3. DMA 수신 시작 (주석 해제 및 변수명 수정)
    // rx_buffer로 이름을 맞추고 전체 크기만큼 받도록 설정합니다.
    if (HAL_UART_Receive_DMA(&huart2, rx_buffer, sizeof(rx_buffer)) != HAL_OK) {
        return false;
    }

    return true;
}

void uartUpdateProcess(void) {
    // 1. uartAvailable() 및 parsePacket()은 프로젝트에 맞춰 구현 필요
    if (uartAvailable() > 0) {
        if (parsePacket(rx_buffer)) {
            uint8_t cmd = rx_buffer[1]; // CMD 위치 확인
            uint16_t data_len = (rx_buffer[2] << 8) | rx_buffer[3]; // 길이 파싱 예시

            switch(cmd) {
                case 0x01: // START
                    HAL_FLASH_Unlock(); // 쓰기 권한 해제
                    if (Erase_App_Sectors() == HAL_OK) {
                        Update_Flag(FLAG_ING);
                        write_addr = APP_START_ADDR;
                        sendAckToAda();
                        current_state = UPDATE_ING;
                    }
                    break;

                case 0x02: // DATA
                    // crcCalculate(CRC_32, ...) 활용
                    uint32_t received_crc = *(uint32_t*)&rx_buffer[4 + data_len];
                    if (crcCalculate(CRC_32, &rx_buffer[4], data_len) == received_crc) {
                        if (Write_Flash(write_addr, &rx_buffer[4], data_len) == HAL_OK) {
                            write_addr += data_len;
                            sendAckToAda();
                        }
                    } else {
                        sendNackToAda(); // CRC 오류 시 요청
                    }
                    break;

                case 0x03: // END
                    if (Is_App_Valid()) { // 최종 무결성 확인
                        Update_Flag(FLAG_PASS);
                        current_state = UPDATE_PASS;
                        sendAckToAda();
                    }
                    HAL_FLASH_Lock(); // 쓰기 권한 잠금
                    break;
            }
        }
    }
}

UpdateState_t uartGetState(void) {
    return current_state;
}

// 성공 응답 (ACK: 0x06)
void sendAckToAda(void) {
    // 구조: [STX(0x02)] [ACK(0x06)] [LEN(0x00)] [ETX(0x03)]
    uint8_t ack_pkt[] = {0x02, 0x06, 0x00, 0x03};
    HAL_UART_Transmit(&huart2, ack_pkt, sizeof(ack_pkt), 100);
}

// 실패 응답 (NACK: 0x15)
void sendNackToAda(void) {
    // 구조: [STX(0x02)] [NACK(0x15)] [LEN(0x00)] [ETX(0x03)]
    uint8_t nack_pkt[] = {0x02, 0x15, 0x00, 0x03};
    HAL_UART_Transmit(&huart2, nack_pkt, sizeof(nack_pkt), 100);
}
