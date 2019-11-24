QT += widgets serialport
requires(qtConfig(combobox))

TARGET = bconf2
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    bconf2.qrc
    target.path = $$[QT_INSTALL_EXAMPLES]/bconf3
INSTALLS += target

RC_FILE = bconf.rc
