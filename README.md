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
