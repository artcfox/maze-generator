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
    uint64_t memory = sizeof(Maze);
    uint32_t dims[2] = { width, height };
    uint32_t length = 2;
    uint32_t totalPositions = 1;
    uint32_t totalWalls = 0;

    memory += sizeof(uint32_t) * length;

    for (uint32_t i = 0; i < length; ++i)
        totalPositions *= dims[i];

    for (uint32_t i = 0; i < length; ++i) {
        uint32_t subTotal = 1;
        for (uint32_t j = 0; j < length; ++j)
            if (j != i)
                subTotal *= dims[j];
        totalWalls += subTotal * (dims[i] - 1);
    }

    uint32_t maxSize = (totalWalls > (totalPositions - 1) * 3) ? totalWalls : (totalPositions - 1) * 3;

    memory += sizeof(Wall) * maxSize;
    memory += sizeof(uint8_t) * totalPositions;
    memory += sizeof(BitArrayRef) * length * 2;

    memory += sizeof(int32_t) * totalPositions; // DisjSets_create
    memory += sizeof(BitArray) * length * 2;
    memory += (totalPositions / 8 + ((totalPositions % 8) ? 1 : 0)) * length * 2;

    ui->labelMemory->setText(sizeHuman(memory));
}

QString NewDialog::sizeHuman(uint64_t size)
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
