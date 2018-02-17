/*
 *  mainwindow.cpp
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "mainwindow.h"
#include "newdialog.h"
#include "ui_mainwindow.h"
#include "ui_about.h"

#include <QInputDialog>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scrollArea = new DragScrollArea(this);
    setCentralWidget(scrollArea);

    mazeWidget = new MazeWidget(this);
    mazeWidget->setBackgroundRole(QPalette::Light);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    scrollArea->setWidget(mazeWidget);

    ui->actionShow_Solution->setChecked(mazeWidget->getShowSolution());
    ui->actionShow_Maze->setChecked(mazeWidget->getShowMaze());
    ui->action_Rounded_Paths->setChecked(mazeWidget->getRoundedPaths());
    ui->actionStatus_bar->setChecked(showStatusBar);

    ui->statusBar->addPermanentWidget(&permanentStatus, 0);

    connect(mazeWidget, SIGNAL(on_deletingOldMaze()), this, SLOT(on_deletingOldMaze()));
    connect(mazeWidget, SIGNAL(on_allocatingMemory()), this, SLOT(on_allocatingMemory()));
    connect(mazeWidget, SIGNAL(on_generatingMaze()), this, SLOT(on_generatingMaze()));
    connect(mazeWidget, SIGNAL(on_solvingMaze()), this, SLOT(on_solvingMaze()));
    connect(mazeWidget, SIGNAL(on_mazeCreated()), this, SLOT(on_mazeCreated()));
}

MainWindow::~MainWindow()
{
    delete mazeWidget;
    delete scrollArea;
    delete ui;
}

void MainWindow::on_actionShow_Solution_triggered()
{
    mazeWidget->setShowSolution(!mazeWidget->getShowSolution());
    ui->actionShow_Solution->setChecked(mazeWidget->getShowSolution());
}

void MainWindow::on_actionShow_Maze_triggered()
{
    mazeWidget->setShowMaze(!mazeWidget->getShowMaze());
    ui->actionShow_Maze->setChecked(mazeWidget->getShowMaze());
}

void MainWindow::on_action_Rounded_Paths_triggered()
{
    mazeWidget->setRoundedPaths(!mazeWidget->getRoundedPaths());
    ui->action_Rounded_Paths->setChecked(mazeWidget->getRoundedPaths());
}

void MainWindow::on_action_New_Maze_triggered()
{
    NewDialog *newDialog = new NewDialog(this, mazeWidget->getMazeWidth(), mazeWidget->getMazeHeight());
    if (QDialog::Accepted == newDialog->exec()) {
        mazeWidget->setMazeWidth(newDialog->getWidth());
        mazeWidget->setMazeHeight(newDialog->getHeight());
        mazeWidget->generateMaze();
        enableMenuItems(false);
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

    delete newDialog;
}

void MainWindow::on_actionE_xit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_action_Distance_Between_Paths_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Distance Between Paths"), tr("Distance Between Paths:                   "), mazeWidget->getGridSpacing(), mazeWidget->getHallThickness() + 1, 256, 1, &ok);
    if (ok)
        mazeWidget->setGridSpacing(value);
}

void MainWindow::on_action_Path_Width_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Path Width"), tr("Path Width:"), mazeWidget->getHallThickness(), mazeWidget->getSolutionThickness(), mazeWidget->getGridSpacing() - 1, 1, &ok);
    if (ok)
        mazeWidget->setHallThickness(value);
}

void MainWindow::on_action_Solution_Width_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Solution Width"), tr("Solution Width:"), mazeWidget->getSolutionThickness(), 1, mazeWidget->getHallThickness(), 1, &ok);
    if (ok)
        mazeWidget->setSolutionThickness(value);
}

void MainWindow::on_action_Reset_Distance_Widths_triggered()
{
    mazeWidget->resetDefaultSpacing();
    mazeWidget->setRoundedPaths(true);
    ui->action_Rounded_Paths->setChecked(mazeWidget->getRoundedPaths());
}

void MainWindow::on_action_About_triggered()
{
    QDialog *about = new QDialog(this, Qt::Dialog);
    Ui_Dialog aboutUi;
    aboutUi.setupUi(about);
    about->exec();
    delete about;
}

void MainWindow::on_deletingOldMaze()
{
    permanentStatus.setText("Deleting Old Maze...");
}

void MainWindow::on_allocatingMemory()
{
    permanentStatus.setText("Allocating Memory...");
}

void MainWindow::on_generatingMaze()
{
    permanentStatus.setText("Generating Maze...");
}

void MainWindow::on_solvingMaze()
{
    permanentStatus.setText("Solving Maze...");
}

void MainWindow::on_mazeCreated()
{
    QApplication::restoreOverrideCursor();
    enableMenuItems(true);
    permanentStatus.setText("Maze Solved!");
    QTimer::singleShot(3000, this, SLOT(hide_mazeCreated()));
}

void MainWindow::hide_mazeCreated()
{
    permanentStatus.setText("");
}

void MainWindow::on_actionStatus_bar_triggered()
{
    showStatusBar = !showStatusBar;
    if (showStatusBar)
        ui->statusBar->show();
    else
        ui->statusBar->hide();
}

void MainWindow::on_action_Print_triggered()
{
    mazeWidget->printMaze();
}

void MainWindow::on_actionExport_Image_triggered()
{
    mazeWidget->exportImage();
}

void MainWindow::enableMenuItems(bool enabled)
{
    ui->action_New_Maze->setEnabled(enabled);
    ui->actionExport_Image->setEnabled(enabled);
    ui->action_Print->setEnabled(enabled);
}

void MainWindow::on_action_Classic_Maze_Style_triggered()
{
    mazeWidget->setSolutionThickness(2);
    mazeWidget->setHallThickness(6);
    mazeWidget->setGridSpacing(7);
    mazeWidget->setRoundedPaths(false);
    ui->action_Rounded_Paths->setChecked(mazeWidget->getRoundedPaths());
}
