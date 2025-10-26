cmake_minimum_required(VERSION 3.19)
project(chatproject LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Network)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(chatproject
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    db.cpp db.h db.ui
    sign.h sign.cpp sign.ui
    initialwidget.h initialwidget.cpp initialwidget.ui
    loadingwidget.h loadingwidget.cpp loadingwidget.ui
    logo.qrc
    popup.h popup.cpp
    find.h find.cpp find.ui
    idfind.h idfind.cpp idfind.ui
    pwfind.h pwfind.cpp pwfind.ui
    pwreset.h pwreset.cpp pwreset.ui
    friendlist.h friendlist.cpp friendlist.ui
    setting.h setting.cpp setting.ui
    more.h more.cpp more.ui
    serch.h serch.cpp serch.ui
    serchok.h serchok.cpp serchok.ui
    frienddelete.h frienddelete.cpp frienddelete.ui
    myprofile.h myprofile.cpp myprofile.ui
    chatlist.h chatlist.cpp
    chatroom.h chatroom.cpp
    profilechange.h profilechange.cpp profilechange.ui
    chatlistsetting.h chatlistsetting.cpp chatlistsetting.ui
    chatlistitem.h chatlistitem.cpp chatlistitem.ui
    chatlistleave.h chatlistleave.cpp chatlistleave.ui
)

target_link_libraries(chatproject
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS chatproject
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET chatproject
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
