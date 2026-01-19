# ESP32 무선 라이트 스위치 봇

이 프로젝트는 ESP32 보드와 서보 모터를 사용하여 물리적인 스위치를 원격으로 제어하는 간단한 웹 애플리케이션입니다. Wi-Fi 네트워크에 연결된 웹 서버를 통해 스위치를 켜고 끌 수 있습니다.

## 주요 기능

- **서보 모터를 이용한 스위치 제어**: ESP32를 사용하여 서보 모터를 정밀하게 제어합니다.
- **웹 기반 인터페이스**: 웹 브라우저를 통해 어디서든 스위치를 제어할 수 있는 간단한 UI를 제공합니다.
- **실시간 상태 표시**: 웹 페이지에서 스위치의 현재 상태(ON/OFF)를 실시간으로 확인할 수 있습니다.
- **토글 방식 제어**: 단일 버튼으로 스위치를 켜고 끄는 토글 기능을 지원합니다.
- **전력 절약 기능**: 서보 모터 작동 후 2초가 지나면 PWM 신호를 중지하여 불필요한 전력 소모와 소음을 줄입니다.

## 준비물

### 하드웨어

- ESP32 개발 보드
- 서보 모터 (예: SG90)
- 전원 공급 장치 및 연결선

### 소프트웨어

- [ESP-IDF (Espressif IoT Development Framework)](https://github.com/espressif/esp-idf)

## 개발 환경 설정 (Windows)

이 프로젝트는 ESP-IDF를 기반으로 하며, Windows 환경에서 개발할 수 있는 몇 가지 방법이 있습니다. 가장 권장되는 방법은 Visual Studio Code의 Dev Container를 활용하는 것입니다.

### 권장 방법: Dev Container 사용

이 방법은 Visual Studio Code와 Docker를 사용하여 프로젝트에 필요한 모든 개발 도구(ESP-IDF, 툴체인 등)가 미리 설정된 격리된 개발 환경을 제공합니다. 이는 로컬 시스템에 복잡한 설정을 할 필요 없이 빠르고 일관된 개발 환경을 구축할 수 있는 가장 쉬운 방법입니다.

**사전 준비물:**
1.  **Visual Studio Code**: 코드 에디터입니다.
2.  **Docker Desktop**: 컨테이너 가상화 플랫폼입니다. 설치되어 있고 실행 중인지 확인하세요.
3.  **Dev Containers 확장 (VS Code)**: VS Code를 열고 확장 뷰(Ctrl+Shift+X)에서 `ms-vscode-remote.remote-containers`를 검색하여 설치합니다.

**설정 단계:**
1.  프로젝트 폴더(`wireless_light_switch_bot`)를 Visual Studio Code에서 엽니다.
2.  일반적으로 VS Code 오른쪽 하단에 "Folder contains a Dev Container configuration file. Reopen in Container..."라는 알림이 나타납니다.
3.  **"Reopen in Container"** 버튼을 클릭합니다.

VS Code는 `.devcontainer` 디렉토리에 정의된 Docker 이미지를 빌드하고(첫 실행 시 몇 분 소요될 수 있음) 해당 컨테이너 내에서 프로젝트를 엽니다. 컨테이너 내부의 터미널에서는 `idf.py build`와 같은 ESP-IDF 명령어를 즉시 사용할 수 있습니다.

### 대안 방법: ESP-IDF Tools Installer 사용

만약 툴체인을 Windows 시스템에 직접 설치하는 것을 선호한다면, Espressif에서 제공하는 공식 ESP-IDF Tools Installer를 사용할 수 있습니다.

1.  **설치 프로그램 다운로드**: [ESP-IDF Tools Installer 페이지](https://dl.espressif.com/dl/esp-idf-tools-setup-online-latest.exe)에서 최신 버전을 다운로드합니다.
2.  **설치 프로그램 실행**: 화면의 지시에 따라 필요한 툴체인, 명령줄 환경(MSYS2), ESP-IDF 복사본 등을 설치합니다.
3.  **명령 프롬프트 사용**: 설치가 완료되면 바탕화면에 생성된 "ESP-IDF Command Prompt" 바로가기를 사용합니다. 이 프롬프트에서 프로젝트 디렉토리로 이동한 후 `idf.py build`와 같은 명령어를 실행할 수 있습니다.

**참고**: Dev Container 방법이 더 간단하고 로컬 시스템 구성 문제 발생 가능성이 적으므로, 이 방법을 먼저 시도해 보시는 것을 권장합니다.

## 환경 변수 설정 (Environment Variable Configuration)

이 프로젝트는 Wi-Fi 연결을 위해 SSID 및 비밀번호를 환경 변수로 설정해야 합니다.

1.  **`sdkconfig.defaults` 파일 추가 및 수정**:
    프로젝트 루트에 있는 `sdkconfig.defaults` 파일을 열고 다음 변수들을 추가하거나 수정합니다.

    ```
    CONFIG_WIFI_SSID="YOUR_WIFI_SSID"
    CONFIG_WIFI_PASSWORD="YOUR_WIFI_PASSWORD"
    ```

    `YOUR_WIFI_SSID`와 `YOUR_WIFI_PASSWORD`를 실제 Wi-Fi 네트워크 정보로 변경하십시오.

2.  **`idf.py menuconfig` 사용**:
    또는 `idf.py menuconfig` 명령을 실행한 후 `Example Connection Configuration` 메뉴에서 `Wi-Fi SSID`와 `Wi-Fi Password`를 직접 입력할 수도 있습니다. `sdkconfig.defaults`에 설정된 값은 `menuconfig` 실행 시 기본값으로 로드됩니다.

## 설정 및 사용법

1.  **Wi-Fi 설정**: `idf.py menuconfig`의 `Example Connection Configuration` 메뉴에서 Wi-Fi SSID와 비밀번호를 입력합니다.
2.  **펌웨어 빌드 및 업로드**: ESP-IDF 환경에서 다음 명령어를 실행하여 프로젝트를 빌드하고 ESP32 보드에 업로드합니다.
    ```bash
    idf.py build
    idf.py -p (사용자 포트) flash monitor
    ```
    `(사용자 포트)`는 사용하는 시스템의 포트 이름으로 변경해야 합니다. (예: `/dev/ttyUSB0` 또는 `COM3`)
3.  **IP 주소 확인**: 시리얼 모니터에서 "Got IP: XXX.XXX.XXX.XXX"와 같은 로그를 확인하여 ESP32에 할당된 IP 주소를 찾습니다.
4.  **웹 인터페이스 접속**: 동일한 Wi-Fi 네트워크에 연결된 기기의 웹 브라우저에서 확인된 IP 주소로 접속합니다.
5.  **스위치 제어**: 웹 페이지에 나타난 버튼을 눌러 스위치를 원격으로 제어합니다.
