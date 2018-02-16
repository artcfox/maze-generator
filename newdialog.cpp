/*
 *  newdialog.cpp
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#include "newdialog.h"
#include "ui_newdialog.h"
#include <QDebug>

#include "Maze.h"
#include "BitArray.h"

NewDialog::NewDialog(QWidget *parent, int width, int height) :
    QDialog(parent),
    ui(new Ui::NewDialog),
    width(width),
    height(height)
{
    ui->setupUi(this);
    ui->spinBoxWidth->setValue(width);
    ui->spinBoxHeight->setValue(height);
    ui->spinBoxWidth->setFocus();
    updateMemoryDisplay();
}

NewDialog::~NewDialog()
{
    delete ui;
}

void NewDialog::on_spinBoxWidth_valueChanged(int arg1)
{
    width = arg1;
    updateMemoryDisplay();
}

void NewDialog::on_spinBoxHeight_valueChanged(int arg1)
{
    height = arg1;
    updateMemoryDisplay();
}

void NewDialog::updateMemoryDisplay()
{
    unsigned long long memory = sizeof(Maze);
    int dims[2] = { width, height };
    int length = 2;
    int totalPositions = 1;
    int totalWalls = 0;

    memory += sizeof(unsigned int) * length;

    for (int i = 0; i < length; ++i)
        totalPositions *= dims[i];

    for (int i = 0; i < length; ++i) {
        int subTotal = 1;
        for (int j = 0; j < length; ++j)
            if (j != i)
                subTotal *= dims[j];
        totalWalls += subTotal * (dims[i] - 1);
    }

    int maxSize = (totalWalls > (totalPositions - 1) * 3) ? totalWalls : (totalPositions - 1) * 3;

    memory += sizeof(Wall) * maxSize;
    memory += sizeof(unsigned char) * totalPositions;
    memory += sizeof(BitArrayRef) * length * 2;

    memory += sizeof(int) * totalPositions; // DisjSets_create
    memory += sizeof(BitArray) * length * 2;
    memory += (totalPositions / 8 + ((totalPositions % 8) ? 1 : 0)) * length * 2;

    ui->labelMemory->setText(sizeHuman(memory));
}

QString NewDialog::sizeHuman(unsigned long long size)
{
    float num = size;
    QStringList list;
    list << "KiB" << "MiB" << "GiB" << "TiB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext()) {
        unit = i.next();
        num /= 1024.0;
    }
    return QString().setNum(num, 'f', 2) + " " + unit;
}

int NewDialog::getHeight() const
{
    return height;
}

int NewDialog::getWidth() const
{
    return width;
}
