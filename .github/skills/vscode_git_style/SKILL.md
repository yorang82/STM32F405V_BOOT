---
name: vscode_git_style
description: "Use when setting up or fixing VS Code workspace files, tasks.json, settings.json, launch.json, c_cpp_properties.json, IntelliSense, Git ignore rules, or PC-to-PC environment compatibility for this STM32 firmware project. Apply environment-variable-based paths and avoid committing machine-specific VS Code settings."
---

# VS Code / Git Style Guide
**Project:** ReLief U Firmware  
**Author:** Young Kwan CHO  
**AI Assistant:** Lucy (루시)  
**Last Updated:** 2026-04-06

---

## Use When
- Configuring `.vscode/tasks.json`, `.vscode/settings.json`, `.vscode/launch.json`
- Fixing IntelliSense or Ctrl+Click navigation issues
- Setting up STM32CubeIDE tool paths on a new PC
- Deciding what VS Code files should or should not be committed to Git
- Making workspace settings portable across multiple developer machines

---

## 1. VS Code 환경 설정 (PC 간 호환성)

### 1.1 문제점
`.vscode/tasks.json`, `.vscode/settings.json`, `.vscode/launch.json` 등의 파일은 절대 경로를 포함하면 PC마다 호환성 문제가 생길 수 있다.

**문제가 되는 예시:**
```json
{
  "command": "& \"C:\\ST\\STM32CubeIDE_1.18.0\\STM32CubeIDE\\headless-build.bat\" -data \"E:\\workspace\""
}
```

### 1.2 해결 방법
절대 경로 대신 환경 변수를 우선 사용한다.

```json
{
  "command": "& \"${env:STM32_CUBE_IDE}\\headless-build.bat\" -data \"${env:STM32_WORKSPACE}\""
}
```

### 1.3 Windows 환경 변수 설정 예시
```powershell
[System.Environment]::SetEnvironmentVariable('STM32_CUBE_IDE', 'C:\ST\STM32CubeIDE_1.18.0\STM32CubeIDE', 'User')
[System.Environment]::SetEnvironmentVariable('STM32_WORKSPACE', 'E:\workspace', 'User')
```

---

## 2. Git / .gitignore 규칙

개인 PC별 설정 파일은 Git에 커밋하지 않는 것을 원칙으로 한다.

```gitignore
# VS Code (PC별 환경 설정은 제외)
.vscode/*.log
.vscode/tasks.json
.vscode/settings.json
.vscode/launch.json
.vscode/c_cpp_properties.json
```

### 주의 사항
- `.vscode`의 개인별 절대경로 설정은 되도록 커밋하지 않는다.
- 프로젝트 공유 시 필요한 환경 변수 목록과 설정 방법은 `README.txt`에 기록한다.

---

## 3. IntelliSense 설정

### 3.1 기본 설정
`.vscode/settings.json` 파일에서 IntelliSense가 비활성화되지 않았는지 확인한다.

```json
{
  "C_Cpp.intelliSenseEngine": "default",
  "C_Cpp.errorSquiggles": "enabled",
  "cmake.configureOnOpen": false
}
```

### 3.2 컴파일러 경로
`.vscode/c_cpp_properties.json` 의 `compilerPath` 는 각 PC의 STM32CubeIDE 설치 경로에 맞게 수정한다.

### 3.3 적용 후 조치
- `Developer: Reload Window` 실행
- 색인 생성 완료 후 Ctrl+Click 이동 동작 확인

---

## 4. 새 PC 설정 체크리스트
1. STM32CubeIDE 설치 경로 확인
2. 환경 변수 설정 또는 `compilerPath` 수정
3. IntelliSense 활성화 상태 확인
4. VS Code 창 다시 로드
5. 함수 이동 / 자동완성 동작 확인

---

## 5. 기본 원칙
- VS Code 설정은 **공유 가능한 것과 개인 전용 것을 구분**한다.
- 가능하면 **환경 변수 기반**으로 작성한다.
- Git에는 **재현 가능한 공용 설정만** 남긴다.
