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
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    ui->actionStatus_bar->setChecked(showStatusBar);

    ui->statusBar->addPermanentWidget(&permanentStatus, 0);

    singleShot.setSingleShot(true);
    singleShot.setInterval(3000);
    connect(&singleShot, SIGNAL(timeout()), this, SLOT(hide_mazeCreated()));

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
        singleShot.stop();
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
    singleShot.start(3000);
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

void MainWindow::on_actionZoom_In_triggered()
{
    // Ensure widget doesn't exceed maximum size
    if ((mazeWidget->width() * 2.0 > QWIDGETSIZE_MAX) || (mazeWidget->height() * 2.0 > QWIDGETSIZE_MAX))
        return;

    int hScroll = scrollArea->horizontalScrollBar()->value();
    int vScroll = scrollArea->verticalScrollBar()->value();
    mazeWidget->setScaling(mazeWidget->getScaling() * 2.0);
    scrollArea->horizontalScrollBar()->setValue(hScroll * 2.0);
    scrollArea->verticalScrollBar()->setValue(vScroll * 2.0);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    // Ensure widget doesn't get smaller than a pixel
    if ((mazeWidget->width() / 2.0 < 1) || (mazeWidget->height() / 2.0 < 1))
        return;

    int hScroll = scrollArea->horizontalScrollBar()->value();
    int vScroll = scrollArea->verticalScrollBar()->value();
    mazeWidget->setScaling(mazeWidget->getScaling() / 2.0);
    scrollArea->horizontalScrollBar()->setValue(hScroll / 2.0);
    scrollArea->verticalScrollBar()->setValue(vScroll / 2.0);
}

void MainWindow::on_actionZoom_Normal_triggered()
{
    int hScroll = scrollArea->horizontalScrollBar()->value();
    int vScroll = scrollArea->verticalScrollBar()->value();
    qreal scaling = mazeWidget->getScaling();
    mazeWidget->setScaling(1.0);
    scrollArea->horizontalScrollBar()->setValue(hScroll / scaling);
    scrollArea->verticalScrollBar()->setValue(vScroll / scaling);
}

void MainWindow::on_actionAn_tialiased_triggered()
{
    mazeWidget->setAntialiased(!mazeWidget->getAntialiased());
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
}
