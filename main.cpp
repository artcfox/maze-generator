/*
 *  main.cpp
 *  MazeGenerator
 *
 *  Created by Matthew T Pandina on 2/11/18.
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
