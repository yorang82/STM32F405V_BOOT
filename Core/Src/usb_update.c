
/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: usb_update.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: USB 기반 펌웨어 업데이트 처리 함수 구현
 */

#include "usb_update.h"
#include "ff.h"            // FATFS, FIL 등 파일시스템 헤더
#include "flash_engine.h"  // 공용 플래시 엔진
#include "usb_host.h"      // USBH_MSC_IsReady 사용
#include "usbh_msc.h"      // MSC 클래스 함수 사용
#include <stdio.h>         // printf



// ==================================================
//                  외부 핸들 참조
// ==================================================
extern USBH_HandleTypeDef hUsbHostFS;


/* -------------------------------------------------------------------------- */
/*                  USB 업데이트 처리 함수                                    */
/* -------------------------------------------------------------------------- */
/**
 * @brief  USB 기반 펌웨어 업데이트 처리 (메인 루프에서 반복 호출)
 */
void Process_USB_Update(void)
{
    FATFS usbFatFs;
    FIL updateFile;
    UINT bytesRead;
    uint8_t writeBuffer[128] __ALIGNED(4); // 128바이트 전략
    uint32_t currentAddr = APP_START_ADDR;

    // 1. 마운트 (0: 지연 마운트 사용)
    // 경로를 "0:/"으로 명시해 보세요.
    if (f_mount(&usbFatFs, "0:/", 0) == FR_OK) {
        
        // 2. 파일 열기 시도 (여기서 실제 에러 코드를 확인합니다)
        FRESULT res = f_open(&updateFile, UPDATE_FILENAME, FA_READ);
        if (res == FR_OK) {
            printf("File Found! Size: %lu bytes\r\n", (uint32_t)f_size(&updateFile));
            
<<<<<<< HEAD
            HAL_FLASH_Unlock();
            if (Erase_App_Sectors() == HAL_OK) {
                Update_Flag(FLAG_ING);
                HAL_FLASH_Unlock(); // 잠긴 플래시 다시 해제
=======
            if (Erase_App_Sectors() == HAL_OK) {
                Update_Flag(FLAG_ING);
>>>>>>> 3413773 (Initial commit)

                LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
                while (f_read(&updateFile, writeBuffer, 128, &bytesRead) == FR_OK && bytesRead > 0) {
                    MX_USB_HOST_Process(); // 쓰기 중에도 USB 엔진 유지
                    if (Write_Flash(currentAddr, writeBuffer, bytesRead) != HAL_OK) break;
                    currentAddr += bytesRead;
                }
                LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);

                if (f_eof(&updateFile)) {
                    Update_Flag(FLAG_PASS);
                    printf("USB Update Success! Resetting...\r\n");
                }
            }
<<<<<<< HEAD
            HAL_FLASH_Lock();
=======
>>>>>>> 3413773 (Initial commit)
            f_close(&updateFile);
        } else {
            // [중요] 에러 코드가 4(FR_NO_FILE)라면 파일명이 틀린 겁니다.
            printf("f_open failed! Error Code: %d\r\n", (int)res);
        }
        f_mount(NULL, "0:/", 0); // 언마운트
    }
}
