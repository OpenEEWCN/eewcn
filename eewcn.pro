lessThan(QT_MAJOR_VERSION,5):error(Qt older than 5.3 is not supported.)
equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,3):error(Qt older than 5.3 is not supported.)

# 注意QML文件移除后应当手动指定Location依赖
QT       += core gui network multimedia widgets quickwidgets websockets svg sql positioning location

# 特例：iOS中没有串口模块
!ios: QT += serialport

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# 此处读取机器ID
MACHINE_UUID = "$$cat(qmake_muuid.txt)"
DEFINES += VALIDATION_MACHINE_ID=\\\"'$$MACHINE_UUID'\\\"

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    contextmenuswindow.cpp \
    earthquakelistentryform.cpp \
    eewsettings.cpp \
    eewsounds.cpp \
    geopolygon.cpp \
    listhistorywidget.cpp \
    logger.cpp \
    main.cpp \
    mainwindow.cpp \
    mapviewframe.cpp \
    sethomedialog.cpp \
    settingsdialog.cpp \
    stationdata.cpp \
    testeewdialog.cpp \
    value.cpp

HEADERS += \
    aboutdialog.h \
    contextmenuswindow.h \
    earthquakelistentryform.h \
    eewsettings.h \
    eewsounds.h \
    geopolygon.h \
    listhistorywidget.h \
    logger.h \
    macloginitems.h \
    mainwindow.h \
    mapviewframe.h \
    sethomedialog.h \
    settingsdialog.h \
    stationdata.h \
    testeewdialog.h \
    value.h

FORMS += \
    aboutdialog.ui \
    contextmenuswindow.ui \
    earthquakelistentryform.ui \
    mainwindow.ui \
    sethomedialog.ui \
    settingsdialog.ui \
    testeewdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

msvc:QMAKE_CXXFLAGS += -utf-8
# 如果要将RC_FILE放在条件编译里的话必须放在win32里而不是msvc中
win32:RC_FILE += resource.rc

macx{
SOURCES += macloginitems.mm
ICON = eewcn.icns
}

DISTFILES +=

RESOURCES +=

TRANSLATIONS = eewcn_zh_CN.ts eewcn_zh_TW.ts eewcn_ja.ts

ANDROID_EXTRA_LIBS =

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
android: include(/Users/yuxin/Library/Android/sdk/android_openssl/openssl.pri)

# 根据 https://forum.qt.io/topic/30078/solved-bundle-files-along-with-qt-ios-app/3 的讨论，
# Mac及iOS系统可以使用 QMAKE_BUNDLE_DATA 打包文件，其他系统可以用 INSTALLS 打包文件
# 项目根目录下的文件不能合并写
# 官方文档 https://doc.qt.io/qt-5/ios-platform-notes.html#application-assets
ios{
dirFonts.files = Fonts
dirFonts.path = /

dirgeodata.files = geodata
dirgeodata.path = /

dirMedia.files = Media
dirMedia.path = /

dirRoot.files = eewcn_zh_CN.qm eewcn_zh_TW.qm eewcn_ja.qm eewcn.icns
dirRoot.path = /

QMAKE_BUNDLE_DATA += dirFonts dirgeodata dirMedia dirRoot

# 指定 Info.plist 文件
QMAKE_INFO_PLIST = ios/Info.plist
}
