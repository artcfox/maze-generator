/*
 *  mainwindow.h
 *  MazeGenerator
 *
 *  Created by Matthew T Pandina on 2/11/18.
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>

#include "Maze.h"

#define DEFAULT_GRID_SPACING 24
#define DEFAULT_HALL_THICKNESS 16
#define DEFAULT_SOLUTION_THICKNESS 8

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int getMazeHeight() const;
    void setMazeHeight(int value);

    int getMazeWidth() const;
    void setMazeWidth(int value);

    int getGridSpacing() const;
    void setGridSpacing(int value);

    int getHallThickness() const;
    void setHallThickness(int value);

    int getSolutionThickness() const;
    void setSolutionThickness(int value);

private slots:
    void on_actionExit_triggered();

    void on_action_New_triggered();

    void on_action_Print_triggered();

    void on_action_Show_Solution_triggered();

    void on_actionSave_As_Image_triggered();

    void on_action_Default_Width_triggered();

    void on_action_Default_Height_triggered();

    void on_action_Grid_Spacing_triggered();

    void on_action_Path_Width_triggered();

    void on_action_Solution_Width_triggered();

    void on_actionRounded_Paths_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionZoom_Normal_triggered();

    void on_actionShow_Maze_triggered();

    void on_action_About_triggered();

    void on_action_Reset_Widths_triggered();

    void on_action_Linear_Gradient_triggered();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    MazeRef myMaze = 0;
    int mazeHeight = 25;
    int mazeWidth = 25;

    int gridSpacing = DEFAULT_GRID_SPACING;
    int hallThickness = DEFAULT_HALL_THICKNESS;
    int solutionThickness = DEFAULT_SOLUTION_THICKNESS;

    bool showSolution = true;
    bool showMaze = true;
    bool roundedPaths = true;
    bool linearGradient = false;

    void generateMaze();
    void displayMaze();
};

#endif // MAINWINDOW_H
