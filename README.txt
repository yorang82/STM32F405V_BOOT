================================================================================
STM32F405V_Boot 파일설명
1. XXXXXXXX_boot.hex : 부트로더 ( ST-LINK 장비로 STM32F405 칩 플래쉬메모리에 적용)
================================================================================

----------------------
[2026-04-21] 펌웨어 업데이트 파일/주소/플래그 설정 안내
----------------------
1. 업데이트 파일명
	 - 기본 파일명: update.bin
	 - 변경하려면 Core/Inc/flash_engine.h에서
		 #define UPDATE_FILENAME   "update.bin"
		 부분을 원하는 파일명으로 수정

2. Application 시작 주소
	 - 기본 주소: 0x08020000 (Sector 5)
	 - 변경하려면 Core/Inc/flash_engine.h에서
		 #define APP_START_ADDR    0x08020000
		 부분을 원하는 주소로 수정

3. 플래그 저장 주소
	 - 기본 주소: 0x08060000 (Sector 7)
	 - 변경하려면 Core/Inc/flash_engine.h에서
		 #define FLAG_ADDR         0x08060000
		 부분을 원하는 주소로 수정


----------------------
USB 전원 및 이벤트 콜백 처리 안내
----------------------

1. USB 전원 제어 (VBus)
   - 파일: USB_HOST/Target/usbh_conf.c
   - 함수: USBH_LL_DriverVBus

     ```c
     USBH_StatusTypeDef USBH_LL_DriverVBus(USBH_HandleTypeDef *phost, uint8_t state)
     {
       if (state == 0) {
         // 전원 OFF
         HAL_GPIO_WritePin(USB_PENA_GPIO_Port, USB_PENA_Pin, GPIO_PIN_RESET);
       } else {
         // 전원 ON
         HAL_GPIO_WritePin(USB_PENA_GPIO_Port, USB_PENA_Pin, GPIO_PIN_SET);
       }
       return USBH_OK;
     }
     ```

2. USB 이벤트 콜백 (상태 전이)
   - 파일: USB_HOST/App/usb_host.c
   - 함수: static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)

     ```c
     switch(id)
     {
       case HOST_USER_SELECT_CONFIGURATION:
         break;
       case HOST_USER_DISCONNECTION:
         Appli_state = APPLICATION_DISCONNECT;
         break;
       case HOST_USER_CLASS_ACTIVE:
         // MSC(대용량 저장장치)가 실제로 준비된 시점에만 APPLICATION_READY로 세팅
         Appli_state = APPLICATION_READY;
         break;
       case HOST_USER_CONNECTION:
         Appli_state = APPLICATION_START;
         break;
       default:
         break;
     }
     ```

   - 반드시 HOST_USER_CLASS_ACTIVE에서만 APPLICATION_READY로 세팅해야 USB 업데이트가 정확히 동작합니다.
