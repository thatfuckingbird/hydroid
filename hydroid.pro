QT += quick quickcontrols2 svg network

CONFIG += c++20

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += src/

SOURCES += \
        src/filecache.cpp \
        src/hydroidimageprovider.cpp \
        src/hydroidnativeutils.cpp \
        src/hydroidsettings.cpp \
        src/hydrusapi.cpp \
        src/main.cpp \
        src/taglistmodel.cpp \
        src/thumbgridmodel.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

wasm {
QMAKE_LFLAGS += -s \"BINARYEN_TRAP_MODE=\'clamp\'\"
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

HEADERS += \
    src/filecache.h \
    src/hydroidimageprovider.h \
    src/hydroidnativeutils.h \
    src/hydroidsettings.h \
    src/hydrusapi.h \
    src/taglistmodel.h \
    src/thumbgridmodel.h \
    src/version.h
