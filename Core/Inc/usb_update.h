/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: usb_update.h
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: USB 기반 펌웨어 업데이트 처리 API
 */


#ifndef INC_USB_UPDATE_H_
#define INC_USB_UPDATE_H_


#include "main.h"


// ==================================================
//                  외부 API
// ==================================================

/**
 * @brief  USB 업데이트 처리 (메인 루프에서 반복 호출)
 */
void Process_USB_Update(void);



#endif /* INC_USB_UPDATE_H_ */
