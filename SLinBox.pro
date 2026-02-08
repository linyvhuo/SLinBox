QT       += core gui serialport printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

msvc {
   QMAKE_CFLAGS += /utf-8
   QMAKE_CXXFLAGS += /utf-8
}
# Windows库
win32 {
    LIBS += -luser32 -lgdi32 -lpsapi
}

# OpenCV配置
DEFINES += USE_OPENCV

INCLUDEPATH += "E:/OpenCV 4.4.0/opencv/build/include"
INCLUDEPATH += "E:/OpenCV 4.4.0/opencv/build/include/opencv2"

# 2. 库文件路径
LIBS += -L"E:/OpenCV 4.4.0/opencv/build/x64/vc14/lib"

# 3. 区分 Debug/Release 链接库（避免冲突）
CONFIG(debug, debug|release) {
    # Debug 模式：链接带 d 后缀的库
    LIBS += -lopencv_world440d
} else {
    # Release 模式：链接无 d 后缀的库
    LIBS += -lopencv_world440
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    slinbox.cpp \
    core/configmanager.cpp \
    core/logger.cpp \
    core/windowhelper.cpp \
    core/screenhelper.cpp \
    core/globalhotkeymanager.cpp \
    modules/wechatauto/wechatautomodule.cpp \
    modules/screenshotocr/screenshotocrmodule.cpp \
#    modules/screenshotocr/screenshoteditor.cpp \
    modules/serialport/serialportmodule.cpp \
    modules/baseconvert/baseconvertmodule.cpp \
    modules/crccheck/crccheckmodule.cpp \
    modules/timingdiagram/timingdiagrammodule.cpp \
    modules/delaycalc/delaycalcmodule.cpp \
    modules/asciiquery/asciiquerymodule.cpp \
    modules/checksum/checksummodule.cpp \
    modules/datasheet/datasheetmodule.cpp \
    ui/mainwindow.cpp \
    ui/settingsdialog.cpp \
    ui/floatwindow.cpp \
    ui/useragreementdialog.cpp \
    ui/userguidedialog.cpp \
    ui/hotkeysettingdialog.cpp \
    ui/screenshoteditorwindow.cpp

HEADERS += \
    slinbox.h \
    core/configmanager.h \
    core/logger.h \
    core/windowhelper.h \
    core/screenhelper.h \
    core/globalhotkeymanager.h \
    modules/wechatauto/wechatautomodule.h \
    modules/screenshotocr/screenshotocrmodule.h \
#    modules/screenshotocr/screenshoteditor.h \
    modules/serialport/serialportmodule.h \
    modules/baseconvert/baseconvertmodule.h \
    modules/crccheck/crccheckmodule.h \
    modules/timingdiagram/timingdiagrammodule.h \
    modules/delaycalc/delaycalcmodule.h \
    modules/asciiquery/asciiquerymodule.h \
    modules/checksum/checksummodule.h \
    modules/datasheet/datasheetmodule.h \
    ui/mainwindow.h \
    ui/floatwindow.h \
    ui/settingsdialog.h \
    ui/useragreementdialog.h \
    ui/userguidedialog.h \
    ui/hotkeysettingdialog.h \
    ui/screenshoteditorwindow.h

FORMS += \
    slinbox.ui \
    ui/mainwindow.ui \
    ui/floatwindow.ui \
    ui/settingsdialog.ui \
    ui/useragreementdialog.ui \
    ui/userguidedialog.ui \
    ui/hotkeysettingdialog.ui \
    ui/screenshoteditorwindow.ui \
    modules/wechatauto/wechatautomodule.ui \
    modules/screenshotocr/screenshotocrmodule.ui \
#    modules/screenshotocr/screenshoteditor.ui \
    modules/serialport/serialportmodule.ui \
    modules/baseconvert/baseconvertmodule.ui \
    modules/crccheck/crccheckmodule.ui \
    modules/timingdiagram/timingdiagrammodule.ui \
    modules/delaycalc/delaycalcmodule.ui \
    modules/asciiquery/asciiquerymodule.ui \
    modules/checksum/checksummodule.ui \
    modules/datasheet/datasheetmodule.ui

RESOURCES += \
    resources/icons.qrc

RC_ICONS = E:\QT_Project\SLinBox\app_icon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
