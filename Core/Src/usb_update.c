/*
 * usb_update.c
 *
 *  Created on: 2026. 4. 21.
 *      Author: YK_Cho
 */
#include "usb_update.h"
#include "flash_engine.h" // [추가] 공용 엔진 헤더
#include "usb_host.h"     // [추가] USBH_MSC_IsReady 사용을 위해
#include "usbh_msc.h"     // MSC 클래스 함수 사용을 위해

extern USBH_HandleTypeDef hUsbHostFS;

void Process_USB_Update(void) {
    FATFS usbFatFs;
    FIL updateFile;
    UINT bytesRead;
    uint8_t writeBuffer[FLASH_CHUNK_SIZE] __ALIGNED(4);
    uint32_t currentAddr = APP_START_ADDR;

    // 1. USB 스택이 준비될 때까지 최대 3초 대기 (PB14는 스택이 알아서 켭니다)
    uint32_t timeout = HAL_GetTick() + 3000;
    while (USBH_MSC_IsReady(&hUsbHostFS) != USBH_OK) {
        MX_USB_HOST_Process(); // USB 상태 머신을 계속 돌려줘야 장치가 인식됩니다.
        if (HAL_GetTick() > timeout) return; // 장치 없음 혹은 인식 실패
    }

    // 2. 파일 시스템 마운트 및 업데이트 진행
    if (f_mount(&usbFatFs, "", 1) == FR_OK) {
        if (f_open(&updateFile, UPDATE_FILENAME, FA_READ) == FR_OK) {

            HAL_FLASH_Unlock();

            if (Erase_App_Sectors() == HAL_OK) {
                Update_Flag(FLAG_ING); // Sector 7 기록

                while (f_read(&updateFile, writeBuffer, FLASH_CHUNK_SIZE, &bytesRead) == FR_OK && bytesRead > 0) {
                    if (Write_Flash(currentAddr, writeBuffer, bytesRead) != HAL_OK) break;
                    currentAddr += bytesRead;
                }

                if (f_eof(&updateFile)) {
                    Update_Flag(FLAG_PASS); // 성공 시 PASS 기록
                }
            }

            HAL_FLASH_Lock();
            f_close(&updateFile);
        }
        f_mount(NULL, "", 0); // 마운트 해제
    }
}
