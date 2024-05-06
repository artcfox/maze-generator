/*
 *  mainwindow.h
 *  MazeGenerator
 *
 *  Copyright 2018-2024 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include "dragscrollarea.h"
#include <QScrollBar>
#include <QLabel>
#include "mazewidget.h"
#include "newdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void closeEvent(QCloseEvent *event) override;

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
    void enableMenuItemsAndRefreshStatusBar();

    void on_openMaze();
    void on_openMazeError(QString err);

    void on_savingMaze();
    void on_saveMazeError(QString err);
    void on_mazeSaved();

    void openMazeWorker_start();
    void saveMazeWorker_start();

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

    void on_action_Save_Maze_As_triggered();

    void on_action_Open_Maze_triggered();

    void on_actionHighlight_Cell_triggered();

    void aboutDialogFinished(int result);
    void newDialogFinished(int result);

private:
    Ui::MainWindow *ui;
    NewDialog *newDialog;
    QDialog *about;
    DragScrollArea *scrollArea;
    MazeWidget *mazeWidget;
    QLabel permanentStatus;
    QString previousStatus;
    bool showStatusBar = true;
    void enableMenuItems(bool enabled);
    bool reallyQuit();
};

#endif // MAINWINDOW_H
