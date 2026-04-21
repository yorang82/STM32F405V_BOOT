---
name: cli_style
description: "Use when creating or refactoring CLI commands in this firmware project. Apply consistent command naming, argument parsing, loop-monitor behavior, help/arg-list registration, and safe logging patterns for STM32 UART CLI modules."
---

# CLI Style Guide
**Project:** Hydro Derma Firmware  
**Author:** Young Kwan CHO  
**AI Assistant:** Lucy (루시)  
**Last Updated:** 2026-04-07

---

## Use When
- Adding new CLI commands in driver/module/application code
- Refactoring existing command handlers (`cliXxx`) for readability and safety
- Registering command auto-complete/help entries (`cliAddArgList`)
- Implementing monitor/stream mode using `cliSetLoopHandler()`
- Standardizing CLI output wording, usage text, and error handling

---

## 1. 기본 구조
CLI 구현은 소스 파일 하단의 별도 섹션으로 분리한다.

```c
/* -------------------------------------------------------------------------- */
/*                                    CLI                                     */
/* -------------------------------------------------------------------------- */
#ifdef _USE_CLI
#include "cli.h"

static void cliModule(cli_args_t *args)
{
  // command parsing
}

void moduleCliInit(void)
{
  cliAdd("module", cliModule);
  cliAddArgList("module", "info");
}
#endif
```

원칙:
- CLI 코드는 반드시 `#ifdef _USE_CLI` 블록 내부에 작성한다.
- CLI 전용 상태 변수는 `static`으로 선언하고 `s_` 접두어를 사용한다.
- `cliMain()` 경로와 충돌하지 않도록 loop handler 상태를 명확히 관리한다.

---

## 2. 명령어 네이밍 규칙

### 2.1 메인 명령어
- 모듈/드라이버 이름을 사용한다.
- 예: `adc`, `gpo`, `a3979`, `flash`

### 2.2 서브 명령어
- 동사 중심의 짧은 키워드 사용
- 권장: `info`, `read`, `write`, `set`, `go`, `stop`, `status`

### 2.3 방향/모드 토큰
- 약어는 프로젝트에서 이미 쓰는 형식을 따른다.
- 예: `cw`, `ccw`, `on`, `off`

---

## 3. 파싱 패턴
조건문은 `argc` 검사 후 `isStr()`로 분기한다.

```c
if (args->argc == 1 && args->isStr(0, "info"))
{
  // info 처리
}
else if (args->argc == 2 && args->isStr(0, "current"))
{
  uint32_t ma = (uint32_t)args->getData(1);
  // current 처리
}
else
{
  // usage 출력
}
```

원칙:
- 입력값 범위가 있으면 반드시 범위 체크를 수행한다.
- 변환 후 타입 캐스팅 의도를 명확히 한다 (`(uint32_t)args->getData(n)`).
- 파싱 실패/미지원 명령에서는 usage를 항상 출력한다.

---

## 4. 출력/메시지 스타일
- 상태 출력은 사람이 보기 쉬운 고정 포맷으로 작성한다.
- 단위는 반드시 명시한다 (`Hz`, `mA`, `step`, `ms`).
- 성공/시작/중단 메시지는 짧고 즉시 이해 가능하게 작성한다.

예시:
```c
cliPrintf("전류 설정: %lu mA\n", (unsigned long)ma);
cliPrintf("상태 모니터 시작. Ctrl+C 로 중단...\n");
```

---

## 5. 모니터/스트림(loop) 규칙
`cliSetLoopHandler()`를 사용할 때는 주기 제한과 종료 조건을 반드시 둔다.

```c
static uint32_t s_prev_time = 0U;

static void moduleCliLoop(void)
{
  uint32_t now = millis();

  if ((s_prev_time != 0U) && ((now - s_prev_time) < 200U))
  {
    return;
  }
  s_prev_time = now;

  cliPrintf("...\n");
}
```

원칙:
- 출력 주기 없는 연속 출력 금지 (UART flooding 방지).
- 루프 시작 명령에서 기준 시간 변수를 초기화한다.
- 필요 시 `cliSetLoopHandler(NULL)`로 종료 경로를 제공한다.

---

## 6. 등록 규칙
`moduleCliInit()`에서 명령어와 arg-list를 함께 등록한다.

```c
void moduleCliInit(void)
{
  cliAdd("module", cliModule);

  cliAddArgList("module", "info");
  cliAddArgList("module", "set <value>");
  cliAddArgList("module", "status");
}
```

원칙:
- `cliAdd()` 누락 금지
- 실제 지원하는 명령만 `cliAddArgList()`에 등록
- 문법 문자열은 usage와 동일하게 유지

---

## 7. 에러 처리 원칙
CLI 핸들러 내부에서도 프로젝트 공통 에러 스타일을 따른다.

- 하위 드라이버 API에서 실패 시: `LOG_INFO()` -> `FAIL_WITH_ERROR` -> 타입별 return
- CLI는 사용자에게도 실패 원인을 짧게 알려준다.

예시:
```c
if (id >= MAX_CH)
{
  cliPrintf("Invalid channel\n");
  return;
}
```

---

## 8. app 연동 규칙
새 CLI를 추가하면 초기화 경로에 등록한다.

- 위치: `appInit()`의 `#ifdef _USE_CLI` 블록
- 순서: `cliInit()` 이후 각 모듈 `xxxCliInit()` 호출

예시:
```c
cliInit();
a3979CliInit();
```

---

## 9. 체크리스트
- `#ifdef _USE_CLI`로 감쌌는가
- `cliAdd()`와 `cliAddArgList()`를 모두 등록했는가
- 미지원 입력에서 usage를 출력하는가
- 모니터 명령은 주기 제한이 있는가
- 단위/상태 메시지가 명확한가
- app 초기화 루틴에 `xxxCliInit()`를 연결했는가
