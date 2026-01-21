# ESP32 무선 라이트 스위치 봇 및 모바일 앱

이 프로젝트는 ESP32 보드와 서보 모터를 사용하여 물리적인 스위치를 원격으로 제어하는 시스템과, 이를 제어하기 위한 모바일 애플리케이션으로 구성됩니다.

## 1. ESP32 무선 라이트 스위치 봇

이 부분은 ESP32 보드와 서보 모터를 사용하여 물리적인 스위치를 원격으로 제어하는 간단한 웹 애플리케이션입니다. Wi-Fi 네트워크에 연결된 웹 서버를 통해 스위치를 켜고 끌 수 있습니다.

### 주요 기능

- **서보 모터를 이용한 스위치 제어**: ESP32를 사용하여 서보 모터를 정밀하게 제어합니다.
- **웹 기반 인터페이스**: 웹 브라우저를 통해 어디서든 스위치를 제어할 수 있는 간단한 UI를 제공합니다.
- **실시간 상태 표시**: 웹 페이지에서 스위치의 현재 상태(ON/OFF)를 실시간으로 확인할 수 있습니다.
- **토글 방식 제어**: 단일 버튼으로 스위치를 켜고 끄는 토글 기능을 지원합니다.
- **전력 절약 기능**: 서보 모터 작동 후 2초가 지나면 PWM 신호를 중지하여 불필요한 전력 소모와 소음을 줄입니다.

### 준비물

#### 하드웨어

- ESP32 개발 보드
- 서보 모터 (예: SG90)
- 전원 공급 장치 및 연결선
- 2.4GHz를 지원하는 Wi-Fi 공유기

#### 소프트웨어

- [ESP-IDF (Espressif IoT Development Framework)](https://github.com/espressif/esp-idf)

### 개발 환경 설정 (Windows)

이 프로젝트는 ESP-IDF를 기반으로 하며, Windows 환경에서 개발할 수 있는 몇 가지 방법이 있습니다. 가장 권장되는 방법은 Visual Studio Code의 Dev Container를 활용하는 것입니다.

#### 권장 방법: Dev Container 사용

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

#### 대안 방법: ESP-IDF Tools Installer 사용

만약 툴체인을 Windows 시스템에 직접 설치하는 것을 선호한다면, Espressif에서 제공하는 공식 ESP-IDF Tools Installer를 사용할 수 있습니다.

1.  **설치 프로그램 다운로드**: [ESP-IDF Tools Installer 페이지](https://dl.espressif.com/dl/esp-idf-tools-setup-online-latest.exe)에서 최신 버전을 다운로드합니다.
2.  **설치 프로그램 실행**: 화면의 지시에 따라 필요한 툴체인, 명령줄 환경(MSYS2), ESP-IDF 복사본 등을 설치합니다.
3.  **명령 프롬프트 사용**: 설치가 완료되면 바탕화면에 생성된 "ESP-IDF Command Prompt" 바로가기를 사용합니다. 이 프롬프트에서 프로젝트 디렉토리로 이동한 후 `idf.py build`와 같은 명령어를 실행할 수 있습니다.

**참고**: Dev Container 방법이 더 간단하고 로컬 시스템 구성 문제 발생 가능성이 적으므로, 이 방법을 먼저 시도해 보시는 것을 권장합니다.

### 환경 변수 설정 (Environment Variable Configuration)

이 프로젝트는 Wi-Fi 연결을 위해 SSID 및 비밀번호를 환경 변수로 설정해야 합니다.

1.  **`sdkconfig.defaults` 파일 추가 및 수정**:
    esp-firmware 프로젝트 루트에 `sdkconfig.defaults` 파일 생성한 후 다음 변수들을 추가하거나 수정합니다.

    ```
    CONFIG_WIFI_SSID="YOUR_WIFI_SSID"
    CONFIG_WIFI_PASSWORD="YOUR_WIFI_PASSWORD"
    ```

    `YOUR_WIFI_SSID`와 `YOUR_WIFI_PASSWORD`를 실제 Wi-Fi 네트워크 정보로 변경하십시오.

2.  **`idf.py menuconfig` 사용**:
    또는 `idf.py menuconfig` 명령을 실행한 후 `Example Connection Configuration` 메뉴에서 `Wi-Fi SSID`와 `Wi-Fi Password`를 직접 입력할 수도 있습니다. `sdkconfig.defaults`에 설정된 값은 `menuconfig` 실행 시 기본값으로 로드됩니다.

### 설정 및 사용법

1.  **Wi-Fi 설정**: `idf.py menuconfig`의 `Example Connection Configuration` 메뉴에서 Wi-Fi SSID와 비밀번호를 입력합니다.

    > **참고**: ESP32는 2.4GHz Wi-Fi 네트워크에만 연결할 수 있습니다. 5GHz 네트워크는 지원하지 않습니다.

2.  **펌웨어 빌드 및 업로드**: ESP-IDF 환경에서 다음 명령어를 실행하여 프로젝트를 빌드하고 ESP32 보드에 업로드합니다.
    ```bash
    idf.py build
    idf.py -p (사용자 포트) flash monitor
    ```
    `(사용자 포트)`는 사용하는 시스템의 포트 이름으로 변경해야 합니다. (예: `/dev/ttyUSB0` 또는 `COM3`)
3.  **IP 주소 확인**: 시리얼 모니터에서 "Got IP: XXX.XXX.XXX.XXX"와 같은 로그를 확인하여 ESP32에 할당된 IP 주소를 찾습니다.
4.  **웹 인터페이스 접속**: 동일한 Wi-Fi 네트워크에 연결된 기기의 웹 브라우저에서 확인된 IP 주소로 접속합니다.
5.  **스위치 제어**: 웹 페이지에 나타난 버튼을 눌러 스위치를 원격으로 제어합니다.

## 2. 모바일 앱

이 프로젝트는 `create-expo-app`으로 생성된 [Expo](https://expo.dev) 프로젝트입니다. 이 앱은 ESP32 무선 라이트 스위치 봇을 제어하기 위한 모바일 인터페이스 역할을 합니다.

### 시작하기

앱을 시작하려면 터미널에서 다음 명령을 실행하십시오:

```bash
npm run start
```

출력에서 앱을 다음 환경에서 열 수 있는 옵션을 찾을 수 있습니다:

- [개발 빌드](https://docs.expo.dev/develop/development-builds/introduction/)
- [Android 에뮬레이터](https://docs.expo.dev/workflow/android-studio-emulator/)
- [iOS 시뮬레이터](https://docs.expo.dev/workflow/ios-simulator/)
- [Expo Go](https://expo.dev/go), Expo로 앱 개발을 시도하기 위한 제한된 샌드박스

**app** 디렉토리 내의 파일을 편집하여 개발을 시작할 수 있습니다. 이 프로젝트는 [파일 기반 라우팅](https://docs.expo.dev/router/introduction)을 사용합니다.

### 워크플로우

이 프로젝트는 일부 개발 및 릴리스 프로세스를 자동화하기 위해 [EAS 워크플로우](https://docs.expo.dev/eas/workflows/get-started/)를 사용하도록 구성되어 있습니다. 이 명령들은 [`package.json`](./package.json)에 설정되어 있으며 터미널에서 NPM 스크립트를 사용하여 실행할 수 있습니다.

#### 미리보기

`npm run draft`를 실행하여 프로젝트의 [미리보기 업데이트](https://docs.expo.dev/eas/workflows/examples/publish-preview-update/)를 게시할 수 있으며, 이는 Expo Go 또는 개발 빌드에서 볼 수 있습니다.

#### 개발 빌드

`npm run development-builds`를 실행하여 [개발 빌드를 생성](https://docs.expo.dev/eas/workflows/examples/create-development-builds/)하십시오. 참고 - 시스템에 올바른 에뮬레이터 설정이 되어 있는지 확인하려면 [사전 요구 사항](https://docs.expo.dev/eas/workflows/examples/create-development-builds/#prerequisites)을 따라야 합니다.

#### 프로덕션 배포

`npm run deploy`를 실행하여 [프로덕션에 배포](https://docs.expo.dev/eas/workflows/examples/deploy-to-production/)하십시오. 참고 - Apple 및 Google 스토어에 제출할 준비가 되어 있는지 확인하려면 [사전 요구 사항](https://docs.expo.dev/eas/workflows/examples/deploy-to-production/#prerequisites)을 따라야 합니다.

### 호스팅

Expo는 EAS Hosting을 통해 웹사이트 및 API 기능 호스팅을 제공합니다. 자세한 내용은 [시작하기 가이드](https://docs.expo.dev/eas/hosting/get-started/)를 참조하십시오.

### 새 프로젝트 가져오기

준비되면 다음을 실행하십시오:

```bash
npm run reset-project
```

이 명령은 시작 코드를 **app-example** 디렉토리로 이동하고 개발을 시작할 수 있는 빈 **app** 디렉토리를 생성합니다.

### 더 알아보기

Expo로 프로젝트 개발에 대해 더 자세히 알아보려면 다음 자료를 참조하십시오:

- [Expo 문서](https://docs.expo.dev/): 기본 사항을 배우거나 [가이드](https://docs.expo.dev/guides)를 통해 고급 주제를 살펴보십시오.
- [Expo 튜토리얼 학습](https://docs.expo.dev/tutorial/introduction/): Android, iOS 및 웹에서 실행되는 프로젝트를 생성하는 단계별 튜토리얼을 따르십시오.

### 커뮤니티 참여

유니버설 앱을 만드는 개발자 커뮤니티에 참여하십시오.

- [GitHub의 Expo](https://github.com/expo/expo): 오픈 소스 플랫폼을 보고 기여하십시오.
- [Discord 커뮤니티](https://chat.expo.dev): Expo 사용자와 채팅하고 질문하십시오.

## 3. 프로젝트 구조

```
.
├── esp-firmware/  # ESP32 펌웨어 관련 소스 코드
│   ├── main/        # 메인 애플리케이션 로직 (웹 서버, 서보 제어 등)
│   └── build/       # 컴파일된 펌웨어 및 빌드 아티팩트
│
└── mobile-app/    # React Native 모바일 앱 관련 소스 코드
    ├── app/         # 화면(Screen) 및 라우팅 설정
    ├── assets/      # 이미지, 폰트 등 정적 에셋
    ├── components/  # 재사용 가능한 UI 컴포넌트
    └── ...          # 기타 Expo 설정 및 의존성 파일
```
