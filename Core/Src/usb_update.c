
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
    uint8_t writeBuffer[FLASH_CHUNK_SIZE] __ALIGNED(4);
    uint32_t currentAddr = APP_START_ADDR;

    // 1. USB 스택이 준비될 때까지 최대 3초 대기
    uint32_t timeout = HAL_GetTick() + 3000;
    while (USBH_MSC_IsReady(&hUsbHostFS) != USBH_OK) {
        MX_USB_HOST_Process();
        if (HAL_GetTick() > timeout) return;
    }

    // 2. 파일 시스템 마운트 및 업데이트 진행
    if (f_mount(&usbFatFs, "", 1) == FR_OK) {
        if (f_open(&updateFile, UPDATE_FILENAME, FA_READ) == FR_OK) {
            HAL_FLASH_Unlock();
            if (Erase_App_Sectors() == HAL_OK) {
                Update_Flag(FLAG_ING);
                while (f_read(&updateFile, writeBuffer, FLASH_CHUNK_SIZE, &bytesRead) == FR_OK && bytesRead > 0) {
                    if (Write_Flash(currentAddr, writeBuffer, bytesRead) != HAL_OK) break;
                    currentAddr += bytesRead;
                }
                if (f_eof(&updateFile)) {
                    Update_Flag(FLAG_PASS);
                }
            }
            HAL_FLASH_Lock();
            f_close(&updateFile);
        }
        f_mount(NULL, "", 0);
    }
}
