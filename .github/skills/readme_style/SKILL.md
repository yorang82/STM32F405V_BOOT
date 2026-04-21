---
name: readme_style
description: "Use when updating README.txt or README.md, writing changelog entries, version history, release notes, update notes, or engineering guidance in this project. Apply the project's README formatting, section separators, reverse-chronological history, and fixed-area protection rules."
---

# README Style Guide
**Project:** ReLief U Firmware  
**Author:** Young Kwan CHO  
**AI Assistant:** Lucy (루시)  
**Last Updated:** 2026-04-06

---

## Use When
- Updating `README.txt` or `README.md`
- Writing version history, update logs, release notes, or engineering notes
- Summarizing firmware changes after refactors or driver updates
- Keeping project documentation format consistent

---

## 1. 기본 형식
프로젝트 루트의 `README.txt` 또는 `README.md` 는 아래 형식의 변경이력 블록을 따른다.

```markdown
----------------------
[YYYY-MM-DD] vX.XX
----------------------
1. 주요 변경 사항
   - 상세 설명
   - 세부 항목

2. 다음 변경 사항
   - 상세 설명
```

---

## 2. 작성 원칙
- 최신 버전이 위에 오도록 **역순 배치**한다.
- 날짜/버전은 반드시 **`[YYYY-MM-DD] vX.XX`** 형식으로 작성한다.
- 주요 변경 사항은 숫자 목록으로 작성한다.
- 세부 설명은 `-` 항목으로 정리한다.
- 무엇을, 왜, 어떻게 바꿨는지 **기술 중심으로 명확히 기록**한다.
- 파일명, 함수명, 모듈명은 가능한 한 정확히 명시한다.

---

## 3. 구분선 규칙
- 버전 구분선은 `----------------------` 형식 사용
- 큰 섹션 구분선은 필요 시 `=` 또는 `-` 반복 형식 유지
- 기존 문서에 쓰이던 포맷과 간격을 가능한 한 그대로 유지한다.

---

## 4. 프로젝트 고정 규칙
- `README.txt` 변경이력 작성 시 상위 고정 영역(기존 1~14번 기본 영역)은 수정하지 않는다.
- 그 이후 영역부터 버전 기록과 주요 변경점을 추가한다.
* 20번 줄(15번째 줄) 이후부터 자유롭게 변경이력, 버전 기록, 주요 변경점 등을 추가/수정할 것
- 기존 변경이력 포맷(날짜 / 구분선 / 번호 / 내용)은 유지한다.

---

## 5. 예시
```markdown
----------------------
[2025-12-17] v0.01
----------------------
1. ADC 드라이버 HAL → LL 라이브러리로 변경
   - `adc_driver.c`, `adc_driver.h` 수정
   - LL API 사용으로 성능 향상 및 메모리 최적화
   - 채널 선택, 변환 시작, 데이터 읽기 모두 LL 함수로 변경

2. 코딩 스타일 가이드 생성
   - 파일 헤더 형식 정의
   - 섹션 구분 방식 통일
   - Doxygen 스타일 함수 주석 규칙 정리
```

---

## 6. 주의 사항
- 단순 감상문보다 **기술적 변경점** 위주로 쓴다.
- 최신 변경 내역은 위쪽에 배치한다.
- 기존 고정 영역을 건드리지 말고, 이어서 누적한다.
