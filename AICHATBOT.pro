QT += core gui network multimedia texttospeech

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    chatbot.cpp \
    message.cpp \
    filehandler.cpp \
    user.cpp \
    voicerecorder.cpp

HEADERS += \
    mainwindow.h \
    chatbot.h \
    message.h \
    filehandler.h \
    user.h \
    voicerecorder.h

TARGET = AICHATBOT
TEMPLATE = app