/*
 *  mainwindow.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>

#include "mazewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionShow_Solution_triggered();

    void on_actionShow_Maze_triggered();

    void on_action_Rounded_Paths_triggered();

    void on_action_New_Maze_triggered();

    void on_actionE_xit_triggered();

    void on_action_Distance_Between_Paths_triggered();

    void on_action_Path_Width_triggered();

    void on_action_Solution_Width_triggered();

    void on_action_Reset_Distance_Widths_triggered();

    void on_action_About_triggered();

    void on_deletingOldMaze();
    void on_allocatingMemory();
    void on_generatingMaze();
    void on_solvingMaze();
    void on_mazeCreated();

    void hide_mazeCreated();

    void on_actionStatus_bar_triggered();

    void on_action_Print_triggered();

    void on_actionExport_Image_triggered();

private:
    Ui::MainWindow *ui;
    QScrollArea *scrollArea;
    MazeWidget *mazeWidget;
    QLabel permanentStatus;
    bool showStatusBar = true;
    void enableMenuItems(bool enabled);
};

#endif // MAINWINDOW_H
