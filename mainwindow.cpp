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
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
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

void MainWindow::on_action_Round_Caps_triggered()
{
    mazeWidget->setRoundCaps(!mazeWidget->getRoundCaps());
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
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

void MainWindow::on_action_Distance_Between_Walls_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Distance Between Walls"), tr("Distance Between Walls:                   "), mazeWidget->getGridSpacing(), 2, 256, 1, &ok);
    if (ok)
        mazeWidget->setGridSpacing(value);
}

void MainWindow::on_action_Wall_Thickness_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Wall Thickness"), tr("Wall Thickness:"), mazeWidget->getWallThickness(), 1, 256, 1, &ok);
    if (ok)
        mazeWidget->setWallThickness(value);
}

void MainWindow::on_action_Solution_Thickness_triggered()
{
    bool ok;
    int value = QInputDialog::getInt(this, tr("Set Solution Thickness"), tr("Solution Thickness:"), mazeWidget->getSolutionThickness(), 1, 256, 1, &ok);
    if (ok)
        mazeWidget->setSolutionThickness(value);
}

void MainWindow::on_action_Default_Maze_Style_triggered()
{
    on_actionZoom_Normal_triggered();
    mazeWidget->resetDefaultSpacing();
    mazeWidget->setRoundCaps(true);
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
    mazeWidget->setAntialiased(false);
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    mazeWidget->setInverse(false);
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
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
    permanentStatus.setText("<b>Deleting Old Maze...</b>");
}

void MainWindow::on_allocatingMemory()
{
    permanentStatus.setText("<b>Allocating Memory...</b>");
}

void MainWindow::on_generatingMaze()
{
    permanentStatus.setText(QString("<b>Generating %1x%2 Maze...</b>")
                            .arg(mazeWidget->getMazeWidth())
                            .arg(mazeWidget->getMazeHeight()));
}

void MainWindow::on_solvingMaze()
{
    permanentStatus.setText(QString("<b>Solving %1x%2 Maze...</b>")
                            .arg(mazeWidget->getMazeWidth())
                            .arg(mazeWidget->getMazeHeight()));
}

void MainWindow::on_mazeCreated()
{
    QApplication::restoreOverrideCursor();
    enableMenuItems(true);
    int w = mazeWidget->getMazeWidth();
    int h = mazeWidget->getMazeHeight();
    int s = mazeWidget->getSolutionLength();
    permanentStatus.setText(QString("<b>Size: %1x%2, Cells: %3, Walls: %4, Solution Length: %5</b>")
                     .arg(w)
                     .arg(h)
                     .arg(w * h)
                     .arg(((w - 1) * h + w * (h - 1)) - (w * h - 1))
                     .arg(s));
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
    on_actionZoom_Normal_triggered();
    mazeWidget->setSolutionThickness(3);
    mazeWidget->setWallThickness(1);
    mazeWidget->setGridSpacing(8);
    mazeWidget->setRoundCaps(false);
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
    mazeWidget->setAntialiased(false);
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    mazeWidget->setInverse(false);
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
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

void MainWindow::on_action_Tiny_Maze_Style_triggered()
{
    on_actionZoom_Normal_triggered();
    mazeWidget->setSolutionThickness(1);
    mazeWidget->setWallThickness(1);
    mazeWidget->setGridSpacing(2);
    mazeWidget->setRoundCaps(false);
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
    mazeWidget->setAntialiased(false);
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    mazeWidget->setInverse(false);
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
}

void MainWindow::on_actionIn_verse_triggered()
{
    mazeWidget->setInverse(!mazeWidget->getInverse());
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
}

void MainWindow::on_actionDefault_In_verse_triggered()
{
    on_actionZoom_Normal_triggered();
    mazeWidget->resetDefaultSpacing();
    mazeWidget->setWallThickness(mazeWidget->getGridSpacing() - mazeWidget->getWallThickness());
    mazeWidget->setRoundCaps(true);
    ui->action_Round_Caps->setChecked(mazeWidget->getRoundCaps());
    mazeWidget->setAntialiased(false);
    ui->actionAn_tialiased->setChecked(mazeWidget->getAntialiased());
    mazeWidget->setInverse(true);
    ui->actionIn_verse->setChecked(mazeWidget->getInverse());
}

void MainWindow::on_action_Save_Maze_triggered()
{
    mazeWidget->saveNative();
}

void MainWindow::on_action_Load_Maze_triggered()
{
    mazeWidget->loadNative();
}
