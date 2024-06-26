#-------------------------------------------------
#
# Project created by QtCreator 2018-02-14T14:19:19
#
#-------------------------------------------------

QT += core gui printsupport

emscripten {
    QMAKE_LFLAGS +=-sASYNCIFY
    QMAKE_WASM_PTHREAD_POOL_SIZE = 4
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MazeGenerator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    mainwindow.cpp \
    mazewidget.cpp \
    newdialog.cpp \
    dragscrollarea.cpp \
    Maze.c

HEADERS  += mainwindow.h \
    Maze.h \
    DisjSets.h \
    BitArray.h \
    mazewidget.h \
    newdialog.h \
    generatemazeworker.h \
    deletemazeworker.h \
    dragscrollarea.h \
    openmazeworker.h \
    savemazeworker.h

FORMS    += mainwindow.ui \
    about.ui \
    newdialog.ui

DISTFILES += \
    README
