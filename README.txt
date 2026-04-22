================================================================================
STM32F405V_Boot 파일설명
1. XXXXXXXX_boot.hex : 부트로더 ( ST-LINK 장비로 STM32F405 칩 플래쉬메모리에 적용)
================================================================================



[2026-04-23] 부트로더 개선 및 안정화 작업 내역

- UART 업데이트 시 Flash 쓰기 주소를 APP_START_ADDR + fw_offset로 명확히 지정 (정상 기록 보장)
- Flash 쓰기(Write_Flash) 성공 후 2ms 지연(HAL_Delay(2)) 추가로 연속 쓰기 안정성 향상
- Flash 쓰기 시 주소 범위(0x08020000~0x0805FFFF) 초과 방지 코드 추가 (NACK 및 오류 로그)
- Write_Flash 함수 내부에서 Flash Unlock/Lock을 안전하게 처리하도록 개선
- UART 업데이트 정상 동작 시 ACK 로그는 주석 처리, 오류 상황만 로그로 남김
- 종료 커맨드(UNIT 3) 수신 시 로그 추가로 정상 종료 여부 확인 가능
- USB 업데이트와 UART 업데이트의 Flash 기록 일치성 및 점프 동작 검증
- UART 업데이트 중 패킷 손실/순서 오류/종료 타이밍 문제 진단 및 개선 가이드 추가


[2026-04-21] 펌웨어 업데이트 파일/주소/플래그 설정 안내

1. DMA 기반 UART 수신 초기화
  - 함수: uartUpdateInit()
  - DMA 수신 시작 및 IDLE 인터럽트 활성화 필요
  - 예시:
    ```c
    HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, dma_rx_buffer, sizeof(dma_rx_buffer));
    __HAL_UART_CLEAR_IDLEFLAG(&ANDROID_UART_HANDLE);
    __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);
    ```

2. UART 수신 DMA 버퍼 및 콜백 처리
  - DMA 원형 버퍼: dma_rx_buffer[1024+64] 사용
  - IDLE 인터럽트 발생 시, 수신 데이터 처리 함수 호출 필요

3. 패킷 파싱 및 처리
  - HMI/UPD 패킷 헤더 구분, 길이 체크, CRC 및 테일 검증
  - 함수: uartHandleByte(), processFullPacket()

4. ACK/NACK 및 펌웨어 요청 송신
  - 함수: sendUpdateAckToAda(), sendUpdateNackToAda(), sendFwRequestToAda()

5. 상태 관리 및 업데이트 진행
  - 상태 반환: uartGetState()
  - 플래그 갱신: Update_Flag(FLAG_ING), Update_Flag(FLAG_PASS)
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

----------------------
UART 이벤트 콜백 처리 및 DMA 설정안내
----------------------
1. DMA 기반 UART 수신 초기화
  - 함수: uartUpdateInit()
  - DMA 수신 시작 및 IDLE 인터럽트 활성화 필요
  - 예시:
    ```c
    HAL_UART_Receive_DMA(&ANDROID_UART_HANDLE, dma_rx_buffer, sizeof(dma_rx_buffer));
    __HAL_UART_CLEAR_IDLEFLAG(&ANDROID_UART_HANDLE);
    __HAL_UART_ENABLE_IT(&ANDROID_UART_HANDLE, UART_IT_IDLE);
    ```

2. UART 수신 DMA 버퍼 및 콜백 처리
  - DMA 원형 버퍼: dma_rx_buffer[1024+64] 사용
  - IDLE 인터럽트 발생 시, 수신 데이터 처리 함수 호출 필요

  /**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
// [핵심] IDLE 플래그가 세워졌는지 확인
if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE)) 
  {
      __HAL_UART_CLEAR_IDLEFLAG(&huart2);

      // [수정] sizeof(dma_rx_buffer) 대신 실제 크기(1088)를 직접 쓰세요.
      uint32_t length = 1088 - __HAL_DMA_GET_COUNTER(huart2.hdmarx); 

      for (uint32_t i = 0; i < length; i++) {
          uartHandleByte(dma_rx_buffer[i]); 
      }

      HAL_UART_AbortReceive(&huart2);
      // [수정] 여기도 sizeof 대신 1088!
      HAL_UART_Receive_DMA(&huart2, dma_rx_buffer, 1088); 
  }
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

3. 패킷 파싱 및 처리
  - HMI/UPD 패킷 헤더 구분, 길이 체크, CRC 및 테일 검증
  - 함수: uartHandleByte(), processFullPacket()

4. ACK/NACK 및 펌웨어 요청 송신
  - 함수: sendUpdateAckToAda(), sendUpdateNackToAda(), sendFwRequestToAda()

5. 상태 관리 및 업데이트 진행
  - 상태 반환: uartGetState()
  - 플래그 갱신: Update_Flag(FLAG_ING), Update_Flag(FLAG_PASS)
