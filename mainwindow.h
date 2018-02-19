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
#include "dragscrollarea.h"
#include <QScrollBar>
#include <QLabel>
#include <QTimer>
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

    void on_action_Round_Caps_triggered();

    void on_action_New_Maze_triggered();

    void on_actionE_xit_triggered();

    void on_action_Distance_Between_Walls_triggered();

    void on_action_Wall_Thickness_triggered();

    void on_action_Solution_Thickness_triggered();

    void on_action_Default_Maze_Style_triggered();

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

    void on_action_Classic_Maze_Style_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionZoom_Normal_triggered();

    void on_actionAn_tialiased_triggered();

    void on_action_Tiny_Maze_Style_triggered();

    void on_actionIn_verse_triggered();

    void on_actionDefault_In_verse_triggered();

private:
    Ui::MainWindow *ui;
    DragScrollArea *scrollArea;
    MazeWidget *mazeWidget;
    QLabel permanentStatus;
    QTimer singleShot;
    bool showStatusBar = true;
    void enableMenuItems(bool enabled);
};

#endif // MAINWINDOW_H
