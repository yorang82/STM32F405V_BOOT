/**
 * File: uart_update.c
 * Description: UART 기반 펌웨어 업데이트 통합 모듈 (HMI 커맨드 & UPD 데이터 처리)
 * Structure: [HMI] (18 bytes) / [UPD] (141 bytes)
 */

#include "uart_update.h"
#include "flash_engine.h"
#include "crc.h"
#include <string.h>
#include <stdbool.h>


// ==============================================================================
// 1. 프로토콜 및 패킷 정의
// ==============================================================================
#define HMI_HDR          "[HMI]"
#define UPD_HDR          "[UPD]"
#define PKT_TAIL         "\xFF\xFC\xFF\xFF"

<<<<<<< HEAD
// ==============================================================================
// [환경 설정] 여기서 사용할 CRC 타입을 한 번만 정의하세요!
// ==============================================================================
#define CONFIG_HMI_CRC_TYPE    CRC_NONE  // 옵션: CRC_NONE, CRC_8, CRC_16, CRC_32

// ------------------------------------------------------------------------------
// 아래는 건드리지 마세요. (설정에 따라 자동 계산되는 매크로)
// ------------------------------------------------------------------------------
#if (CONFIG_HMI_CRC_TYPE == CRC_NONE)
    #define CONFIG_CRC_LEN 0
#elif (CONFIG_HMI_CRC_TYPE == CRC_8)
    #define CONFIG_CRC_LEN 1
#elif (CONFIG_HMI_CRC_TYPE == CRC_16)
    #define CONFIG_CRC_LEN 2
#elif (CONFIG_HMI_CRC_TYPE == CRC_32)
    #define CONFIG_CRC_LEN 4
#else
    #define CONFIG_CRC_LEN 0
#endif

#define HMI_PKT_LEN      (14 + CONFIG_CRC_LEN)    // [HMI](5) + CMD(1)+PAGE(1)+UNIT(1)+VAL(2) + CRC(가변) + TAIL(4)
#define UPD_PKT_LEN      141   // [UPD](5) + DATA(128) + CRC(4) + TAIL(4)
=======
// ============================================================================== 
// [환경 설정] 여기서 사용할 CRC 타입을 한 번만 정의하세요! 
// ============================================================================== 
// CRC 타입 enum은 crc.h에 이미 정의되어 있으므로 중복 정의하지 않음
#define CONFIG_HMI_CRC_TYPE    CRC_16  // 옵션: CRC_NONE, CRC_8, CRC_16, CRC_32
#define CONFIG_CRC_LEN              2  // CRC_16의 경우 2바이트, CRC_32의 경우 4바이트, CRC_NONE의 경우 0바이트

#define HMI_PKT_LEN      (14 + CONFIG_CRC_LEN)    // [HMI](5) + CMD(1)+PAGE(1)+UNIT(1)+VAL(2) + CRC(가변) + TAIL(4)
#define UPD_PKT_LEN      (141 + CONFIG_CRC_LEN)   // [UPD](5) + OFFSET(4) + DATA(128) + CRC(가변) + TAIL(4)
>>>>>>> 3413773 (Initial commit)
#define UPD_DATA_SIZE    128   // STM32 Flash 4바이트 정렬 최적화 크기

// ==============================================================================
// 2. 내부 변수 및 외부 핸들 참조
// ==============================================================================
static uint32_t fw_offset = 0;        // 0부터 시작하는 기록 오프셋
static uint32_t last_request_tick = 0;
static uint8_t  pkt_processing_buffer[160]; 
static uint32_t pkt_idx = 0;

extern UART_HandleTypeDef DBUG_UART_HANDLE;     // Debug UART
extern UART_HandleTypeDef ANDROID_UART_HANDLE; // Ada(Android) 연결 UART
uint8_t dma_rx_buffer[1024 + 64];       // DMA 수신용 원형 버퍼

// ==============================================================================
// 3. 초기화 및 수신 엔진 (Receiver)
// ==============================================================================
/**
 * @brief UART 업데이트 모듈 초기화 및 DMA 수신 시작
 */
bool uartUpdateInit(void)
{
    fw_offset = 0;
    pkt_idx = 0;

    // DMA 수신 시작 (Normal Mode 권장)
    if (HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, dma_rx_buffer, sizeof(dma_rx_buffer)) != HAL_OK) {
        return false;
    }

    // IDLE 인터럽트 클리어 및 활성화
    __HAL_UART_CLEAR_IDLEFLAG(&ANDROID_UART_HANDLE);
    __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);

    return true;
}

// ==============================================================================
// 4. 실행 엔진 (Processor)
// ==============================================================================

void processFullPacket(uint8_t *buf, uint32_t len)
{
    // 1. 테일 검증
    if (memcmp(&buf[len - 4], PKT_TAIL, 4) != 0) return;

    // 2. [HMI] 커맨드 분기 처리
    if (memcmp(buf, HMI_HDR, 5) == 0) {
<<<<<<< HEAD
        uint8_t unit = buf[7]; // UNIT: 1(Start), 2(Data Ready), 3(End)
        
        switch(unit) {
            case 1: // 시작 커맨드 (0x73, UNIT 1)
                if (Erase_App_Sectors() == HAL_OK) { // 성공
                    Update_Flag(FLAG_ING);
                    fw_offset = 0;
=======
        uint8_t cmd = buf[6]; // UNIT: 1(Start), 2(Data Ready), 3(End)
        
        switch(cmd) {
            case 1: // 시작 커맨드 (0x73, UNIT 1)
                // UART 속도 38400으로 변경
                printf("[UPD] START CMD received\n");
                HAL_UART_AbortReceive(&ANDROID_UART_HANDLE);
                ANDROID_UART_HANDLE.Init.BaudRate = 38400;
                printf("[UPD] UART baud rate set to 38400\n");
                if (HAL_UART_Init(&ANDROID_UART_HANDLE) != HAL_OK) {
                    Error_Handler();
                }
                // DMA 재시작
                HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, dma_rx_buffer, sizeof(dma_rx_buffer));
                __HAL_UART_CLEAR_IDLEFLAG(&ANDROID_UART_HANDLE);
                __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);
                if (Erase_App_Sectors() == HAL_OK) { // 성공
                    Update_Flag(FLAG_ING);
                    fw_offset = 0;
                    printf("[UPD] Erase complete. Ready for update\n");
>>>>>>> 3413773 (Initial commit)
                    sendUpdateAckToAda();
                }
                break;
            case 2: // 데이터 준비 (UNIT 2)
                sendUpdateAckToAda();
                break;
            case 3: // 종료 커맨드 (UNIT 3)
                printf("[UPD] END CMD received\n");
                Update_Flag(FLAG_PASS);
<<<<<<< HEAD
=======
                // UART 속도 115200으로 복구
                HAL_UART_AbortReceive(&ANDROID_UART_HANDLE);
                ANDROID_UART_HANDLE.Init.BaudRate = 115200;
                if (HAL_UART_Init(&ANDROID_UART_HANDLE) != HAL_OK) {
                    Error_Handler();
                }
                HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, dma_rx_buffer, sizeof(dma_rx_buffer));
                __HAL_UART_CLEAR_IDLEFLAG(&ANDROID_UART_HANDLE);
                __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);
>>>>>>> 3413773 (Initial commit)
                sendUpdateAckToAda();
                HAL_FLASH_Lock();
                break;
            default:
                // [여기가 쓰레기 처리장!]
                // Ada가 보낸 setpage(UNIT이 1,2,3이 아닌 경우)는 여기서 그냥 무시됩니다.
                // 아무런 응답(ACK)도 주지 않고 조용히 넘어가는 게 포인트입니다.
                break;
        }
    }
    // 3. [UPD] 펌웨어 데이터 기록
    else if (memcmp(buf, UPD_HDR, 5) == 0) {
<<<<<<< HEAD
        // Write_Flash(오프셋, 데이터, 크기)
        uint32_t dest_addr = APP_START_ADDR + fw_offset;
=======
        // [UPD][OFFSET(4)][DATA(128)][CRC][TAIL]
        uint32_t offset = (uint32_t)buf[5] |
                          ((uint32_t)buf[6] << 8) |
                          ((uint32_t)buf[7] << 16) |
                          ((uint32_t)buf[8] << 24);
        uint32_t dest_addr = APP_START_ADDR + offset;

        // 1. Flash 기록 주소 범위 체크
>>>>>>> 3413773 (Initial commit)
        if ((dest_addr + UPD_DATA_SIZE - 1) > 0x0805FFFF) {
            printf("[UPD][ERR] Write out of range! addr=0x%08lX\n", dest_addr);
            sendUpdateNackToAda();
            return;
        }
<<<<<<< HEAD
        printf("[UPD] Write_Flash addr=0x%08lX, offset=0x%08lX\n", dest_addr, fw_offset);
        HAL_StatusTypeDef res = Write_Flash(dest_addr, &buf[5], UPD_DATA_SIZE);
        if (res == HAL_OK) {
            fw_offset += UPD_DATA_SIZE;
            HAL_Delay(2); // Flash 연속 쓰기 안정화 지연
            sendUpdateAckToAda(); // 정상 기록 시 ACK (다음 패킷 요청)
            // printf("[UPD] ACK sent, offset=0x%08lX\n", fw_offset);
            // 기록 중 부저 반전
            LL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
        } else {
            printf("[UPD][ERR] Flash Write Fail! addr=0x%08lX, offset=0x%08lX, ret=%d\n", dest_addr, fw_offset, res);
=======

        // 2. CRC 체크 (Android와 동일하게 [UPD] 헤더~DATA까지 137바이트 계산)
        if (CONFIG_CRC_LEN > 0) {
            // 패킷에서 CRC 추출 (big-endian)
            uint32_t recv_crc = 0;
            for (uint8_t i = 0; i < CONFIG_CRC_LEN; i++) {
                recv_crc = (recv_crc << 8) | buf[9 + UPD_DATA_SIZE + i];
            }
            // CRC 계산 범위: [UPD]헤더(5)+OFFSET(4)+DATA(128) = 137바이트
            uint32_t calc_crc = crcCalculate(CONFIG_HMI_CRC_TYPE, &buf[0], 137);
            if (recv_crc != calc_crc) {
                printf("[UPD][ERR] CRC mismatch! recv=0x%08lX, calc=0x%08lX\n", recv_crc, calc_crc);
                sendUpdateNackToAda();
                return;
            }
        }

        // 3. Flash 기록
        // printf("[UPD] Write_Flash addr=0x%08lX, offset=0x%08lX\n", dest_addr, offset);
        HAL_StatusTypeDef res = Write_Flash(dest_addr, &buf[9], UPD_DATA_SIZE);
        if (res == HAL_OK) {
            fw_offset = offset + UPD_DATA_SIZE;
            // printf("[UPD] ACK sent, offset=0x%08lX\n", fw_offset);
            sendUpdateAckToAda(); // 정상 기록 시 ACK (다음 패킷 요청)
            // 기록 중 부저 반전
            LL_GPIO_TogglePin(DBG_LED_GPIO_Port, DBG_LED_Pin);
        } else {
            printf("[UPD][ERR] Flash Write Fail! addr=0x%08lX, offset=0x%08lX, ret=%d\n", dest_addr, offset, res);
>>>>>>> 3413773 (Initial commit)
            sendUpdateNackToAda();
        }
    }
}

/**
 * @brief 한 바이트씩 분석하여 패킷을 조립하고 분류함
 */
void uartHandleByte(uint8_t byte)
{
    pkt_processing_buffer[pkt_idx++] = byte;

    if (pkt_idx == 5) {
        if (memcmp(pkt_processing_buffer, HMI_HDR, 5) != 0 && 
            memcmp(pkt_processing_buffer, UPD_HDR, 5) != 0) {
            memmove(pkt_processing_buffer, &pkt_processing_buffer[1], 4);
            pkt_idx = 4;
            return;
        }
    }

    // HMI_PKT_LEN이 14든 16이든 18이든 알아서 딱 맞춰서 파싱합니다.
    if (memcmp(pkt_processing_buffer, HMI_HDR, 5) == 0 && pkt_idx >= HMI_PKT_LEN) {
        processFullPacket(pkt_processing_buffer, HMI_PKT_LEN);
        pkt_idx = 0;
    } 
    else if (memcmp(pkt_processing_buffer, UPD_HDR, 5) == 0 && pkt_idx >= UPD_PKT_LEN) {
        processFullPacket(pkt_processing_buffer, UPD_PKT_LEN);
        pkt_idx = 0;
    }

    if (pkt_idx >= sizeof(pkt_processing_buffer)) pkt_idx = 0; 
}

// ==============================================================================
// 5. 송신 엔진 (환경 설정에 따라 자동 동작)
// ==============================================================================
void hmiSendPacket(uint8_t cmd, uint8_t page, uint8_t unit, uint16_t val)
{
    // 상단 설정에 따라 버퍼 크기(14~18)가 컴파일 타임에 자동 할당됨
    uint8_t tx_buf[HMI_PKT_LEN]; 
    uint32_t idx = 0;
    
    memcpy(&tx_buf[idx], HMI_HDR, 5); idx += 5;
    tx_buf[idx++] = cmd;
    tx_buf[idx++] = page;
    tx_buf[idx++] = unit;
    tx_buf[idx++] = (uint8_t)(val & 0xFF);
    tx_buf[idx++] = (uint8_t)((val >> 8) & 0xFF);

    // CRC 길이가 0보다 클 때만 자동으로 계산 후 부착
    #if (CONFIG_CRC_LEN > 0)
        uint32_t crc = crcCalculate(CONFIG_HMI_CRC_TYPE, tx_buf, idx);
        for (uint8_t i = 0; i < CONFIG_CRC_LEN; i++) {
            uint8_t shift = (uint8_t)((CONFIG_CRC_LEN - 1U - i) * 8U);
            tx_buf[idx++] = (uint8_t)((crc >> shift) & 0xFF);
        }
    #endif
    
    memcpy(&tx_buf[idx], PKT_TAIL, 4); idx += 4;

    HAL_UART_Transmit(&ANDROID_UART_HANDLE, tx_buf, idx, 100);
}

// 호출부가 훨씬 깔끔해집니다!
void sendFwRequestToAda(uint16_t val) {
    hmiSendPacket(0x73, 0x00, 0x00, val);
}
void sendUpdateAckToAda(void) {
    hmiSendPacket(0x6F, 0x00, 0x73, 0x0001);
}
void sendUpdateNackToAda(void) {
    hmiSendPacket(0x6F, 0x00, 0x73, 0x0000); 
}

// ==============================================================================
// 6. 주기적 프로세스 (Main Loop Polling)
// ==============================================================================

void uartUpdateProcess(void)
{
    uint32_t current_tick = HAL_GetTick();
    uint32_t flag = Get_Flag();

    // 1초마다 펌웨어 요청 (NEW=20, READY=44)
    if (flag == FLAG_NEW || flag == FLAG_READY) {
        if (current_tick - last_request_tick > 1000) {
            sendFwRequestToAda((flag == FLAG_NEW) ? 20 : 44);
            last_request_tick = current_tick;
            LL_GPIO_TogglePin(DBG_LED_GPIO_Port, DBG_LED_Pin);
        }
    }
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
