# CMake 최소 버전 지정: Qt6 권장 프로젝트에 흔히 쓰는 3.16 이상
cmake_minimum_required(VERSION 3.16)

# 프로젝트 이름/언어 설정 (C++ 사용)
project(TalkerChatServer LANGUAGES CXX)

# C++ 표준 지정 (Qt6와 잘 맞는 C++17)
set(CMAKE_CXX_STANDARD 17)

# Qt MOC/UIC/RCC 자동화 옵션
# - AUTOMOC: Q_OBJECT 있는 헤더를 자동 moc 처리
# - AUTORCC: 리소스(.qrc) 자동 처리
# - AUTOUIC: .ui 자동 처리 (여기서는 UI 없음이지만 관성적으로 켬)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt 모듈 찾기: Core, Network, Sql 사용
# (Qt6 우선, 없으면 Qt5로 폴백)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Sql)

# 실행 파일 타깃 정의: 서버 본체
add_executable(chat_server
    main.cpp
    chatserver.h chatserver.cpp
    dbutil.h dbutil.cpp
)

# 타깃에 Qt 모듈 링크
target_link_libraries(chat_server PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
)

# 배포 스크립트 생성 및 설치(Windows에서 QMYSQL 등 플러그인 포함)
include(GNUInstallDirs)

install(TARGETS chat_server
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET chat_server
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
