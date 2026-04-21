---
name: coding_style
description: "Use when writing, refactoring, reviewing, or documenting embedded C firmware code in this project. Apply the project's coding style for file headers, section dividers, Doxygen comments, naming, inline comments, table-driven drivers, and error handling with LOG_INFO -> FAIL_WITH_ERROR -> type-appropriate return. Relevant for ADC, GPI, GPO, PWM, UART, dispatcher, CLI, and app-layer code."
---

# Coding Style Guide
**Project:** Hydro Derma Firmware  
**Author:** Young Kwan CHO  
**AI Assistant:** Lucy (루시)  
**Last Updated:** 2026-04-07

> **개발 환경 안내**  
> 개발자는 AI 어시스턴트를 **"루시(Lucy)"** 라고 부릅니다.  
> 이 문서는 별도 지시가 없어도 항상 우선적으로 참고해야 하는 기본 코딩 스타일입니다.
> 사용자 응답은 특별한 요청이 없는 한 기본적으로 **한국어**로 작성합니다.

---

## Use When
- Writing or refactoring embedded C firmware in this repository
- Editing driver code such as `adc_driver.c`, `gpi_driver.c`, `gpo_driver.c`, `pwm_driver.c`, `uart_driver.c`
- Adding CLI, module, dispatcher, or application-layer logic
- Updating comments, naming, section layout, or error handling style

---

## 1. 파일 헤더

모든 소스 파일(`.c`, `.h`)은 다음 형식의 헤더로 시작합니다:

```c
/*
 * Project: 33KHz Ultrasound Firmware
 * 
 * File: [filename].c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: YYYY-MM-DD
 * Description: [상세 설명 - 여러 줄 가능]
 */
```

**예시:**
```c
/*
 * Project: 33KHz Ultrasound Firmware
 * 
 * File: adc_driver.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2025-12-21
 * Description: ADC 드라이버 (LL 버전, API + CLI)
 *              배터리 전압, 전류, 온도 측정 기능 포함
 */
```

---

## 2. 섹션 구분

### 2.1 큰 섹션 (주요 기능 블록)

```c
/* -------------------------------------------------------------------------- */
/*                                SECTION NAME                                */
/* -------------------------------------------------------------------------- */
```

### 2.2 작은 섹션 (변수 그룹, Define 그룹 등)

```c
// ------------------- 섹션 이름 -------------------
```

또는

```c
// ---------------- 섹션 이름 ----------------
```

### 2.3 섹션 설명 (블록 주석)

```c
/* 한 줄 설명 또는 여러 줄 설명 */
/* 이렇게 계속 이어서 작성 가능 */
```

---

## 3. 함수 설명

모든 함수 위에는 Doxygen 스타일 주석을 작성합니다.
특히 헤더 파일의 공개 API는 **사용 목적이 바로 보이도록 `@brief`, `@param`, `@note`, `@retval` 중심으로 간결하게 작성**합니다.

```c
/**
 * @brief  함수의 간단한 설명
 *         (추가 설명이 필요하면 여기에)
 * @param  param1 파라미터 설명
 * @return 반환값 설명
 */
```

### 3.1 권장 헤더 API 주석 형식
```c
/**
 * @brief  A3979 드라이버 및 제어 변수 초기화
 * @note   부팅 시 또는 모터 초기화 단계에서 1회 호출합니다.
 */
void A3979_Driver_Init(void);

/**
 * @brief  DAC를 이용한 모터 구동 전류(토크) 설정
 * @param  target_mA : 목표 전류 (mA 단위, 예: 300)
 * @note   입력한 mA 값에 맞춰 내부적으로 공식(Vref = 8 * I_trip * Rs)을 통해 DAC 값을 자동 계산합니다.
 */
void A3979_Driver_SetCurrent(uint32_t target_mA);

/**
 * @brief  모터 동작 완료 여부 확인
 * @retval 1 : 동작 완료 (또는 정지 상태), 0 : 동작 중
 */
int A3979_Driver_IsDone(void);
```

### 3.2 작성 원칙
- `@brief` 는 한눈에 기능이 이해되도록 짧고 명확하게 쓴다.
- 입력 인자가 있으면 `@param` 으로 각 의미를 설명한다.
- 호출 시점, 주의점, 내부 동작 설명이 필요하면 `@note` 를 추가한다.
- 반환값이 상태값/코드일 때는 `@retval` 또는 `@return` 으로 의미를 명확히 적는다.
- 불필요하게 장황하게 쓰기보다 **헤더를 보는 사람이 바로 사용할 수 있게** 작성한다.

---

## 4. 변수 및 Define

### 4.1 Define 정의

값의 의미를 인라인 주석으로 설명합니다:

```c
#define VALUE_NAME    123    // 설명
```

### 4.2 전역/Static 변수

변수의 용도를 인라인 주석으로 설명합니다:

```c
static type variable_name;     // 변수 용도 설명
```

### 4.3 구조체 멤버

각 멤버에 인라인 주석으로 설명을 추가합니다:

```c
typedef struct
{
  uint8_t port;         // 포트 번호
  uint8_t pin;          // 핀 번호
  gpio_mode_t mode;     // 입출력 모드
} gpio_cfg_t;
```

---

## 5. 인라인 주석

코드 라인 끝에 간단한 설명을 추가합니다:

```c
variable = value;  // 간단한 설명
```

### 5.1 주석 언어 원칙
- **모든 주석(인라인, 블록, Doxygen)은 가능한 한 한글로 작성합니다.**
- 외부 공개 API나 영문 기술 용어(예: prescaler, ARR, ISR)는 원문 그대로 사용해도 됩니다.
- 영문 주석이 더 명확한 경우(공식, 단위 표기 등)를 제외하면 한글 우선으로 작성합니다.

```c
// ✅ 권장
v_a3979_step_count++;     // 현재 진행 스텝 수 증가
LL_TIM_EnableCounter(TIM8); // 타이머 카운터 활성화

// ❌ 지양
v_a3979_step_count++;     // increment step counter
```

---

## 6. 네이밍 규칙

### 6.1 함수명
- `camelCase` 사용
- 동사로 시작
- 예: `gpioInit()`, `adcReadVoltage()`, `uartWrite()`

### 6.2 변수명 (스코프 기반 접두어 사용)
변수의 유효 범위(Scope)를 명확히 하기 위해 접두어를 사용하며, 기본적으로 `snake_case`를 따릅니다.

- **지역 변수 (Local)**: 함수 내부에서만 사용되는 변수는 접두어 없이 소문자로 작성
  - 예: `count`, `timeout_ms`, `rx_data`
- **내부 전역 변수 (Static)**: 파일 내부에서만 사용하는 변수는 `m_` 접두어 사용
  - 예: `m_is_open`, `m_rx_buf_dbug`
- **외부 전역 변수 (Global)**: 다른 파일에서도 참조 가능한 전역 변수는 `g_` 접두어 사용
  - 예: `g_system_error`, `g_user_config`

### 6.3 Define / Enum
- `UPPER_CASE` 대문자 사용
- 단어는 언더스코어(`_`)로 구분
- 예: `ADC_VREF`, `UART_MAX_CH`, `GPIO_OUTPUT`

### 6.4 타입/구조체명
- `snake_case_t` 형식 (`_t` 접미사)
- 예: `gpio_cfg_t`, `adc_ch_t`

---

## 7. 코드 구조

### 7.1 헤더 파일 (`.h`)

```c
#ifndef INCLUDE_MODULE_H_
#define INCLUDE_MODULE_H_

#include "def.h"

// ==================================================
//                  매크로 정의
// ==================================================

// ==================================================
//                  타입 정의
// ==================================================

// ==================================================
//                  외부 API
// ==================================================

#ifdef _USE_CLI
void moduleCliInit(void);
#endif

#endif /* INCLUDE_MODULE_H_ */
```

### 7.2 소스 파일 (`.c`)

```c
#include "module.h"

// ==================================================
//                  내부 구조체
// ==================================================

// ==================================================
//                  내부 변수
// ==================================================

// ==================================================
//                  내부 함수
// ==================================================

// ==================================================
//                  외부 함수
// ==================================================

// ==================================================
//                  CLI
// ==================================================
#ifdef _USE_CLI
// CLI 구현
#endif
```

---

## 8. 특수 기호 사용

### 8.1 중요 표시
```c
// ⚡ 중요한 설정이나 주의사항
```

### 8.2 회로도 설명
```c
// VBAT → R1 →(측정점)→ R2 → GND
```

---

## 9. 에러 처리 및 리턴 스타일

### 9.1 기본 원칙
- 예외/방어 코드에서는 **반드시 `LOG_INFO()`로 실패 이유를 먼저 남기고, 그 다음 `FAIL_WITH_ERROR`로 위치를 기록**한다.
- 하위 드라이버(`adc`, `gpi`, `gpo`, `pwm`)에서는 **함수 반환형에 맞는 값으로 직접 return** 한다.
- `return FAIL_WITH_ERROR;` 는 **상위 계층에서 에러코드를 반환하도록 설계된 함수**에서만 사용한다.

### 9.2 하위 드라이버 스타일
```c
if (ch >= ADC_MAX_CH)
{
  LOG_INFO("ADC ReadRaw Fail: Invalid channel (%u)\r\n", (unsigned int)ch);
  FAIL_WITH_ERROR;
  return 0U;
}
```

```c
if (id >= GPO_MAX_CH)
{
  LOG_INFO("GPO Write Fail: Invalid CH (%d)\r\n", id);
  FAIL_WITH_ERROR;
  return;
}
```

```c
if (p_gpio == NULL)
{
  LOG_INFO("GPIO Init Fail: Port is NULL\r\n");
  FAIL_WITH_ERROR;
  return false;
}
```

### 9.3 `return FAIL_WITH_ERROR;` 사용 가능 조건
아래 조건을 **모두 만족할 때만** 허용한다.
- 함수가 상위 로직/매니저/서비스 계층이다.
- 반환형이 `uint32_t` 같은 **에러코드 전달용 타입**이다.
- 성공 시 `SUCCESS_WITHOUT_ERROR`, 실패 시 에러코드를 상위로 전파하는 구조다.

```c
uint32_t moduleStart(void)
{
  if (driverInit() == false)
  {
    LOG_INFO("Module Start Fail: driverInit failed\r\n");
    return FAIL_WITH_ERROR;
  }

  return SUCCESS_WITHOUT_ERROR;
}
```

### 9.4 금지 사항
- 하위 드라이버의 `void`, `bool`, 센서값 반환 함수에서 `return FAIL_WITH_ERROR;` 사용 금지
- 실패 이유 없이 `FAIL_WITH_ERROR`만 단독 호출하는 패턴 지양
- 정상값처럼 해석될 수 있는 반환형(`uint32_t`, `float`)은 반드시 로그를 먼저 남기고 방어 반환값을 명시할 것

---

---
