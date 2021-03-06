/*
 *  newdialog.h
 *  MazeGenerator
 *
 *  Copyright 2018 Matthew T. Pandina. All rights reserved.
 *
 */

#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>

namespace Ui {
class NewDialog;
}

class NewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewDialog(QWidget *parent = 0, int width = 25, int height = 25);
    ~NewDialog();

    int getWidth() const;
    int getHeight() const;

private slots:
    void on_spinBoxWidth_valueChanged(int arg1);

    void on_spinBoxHeight_valueChanged(int arg1);

private:
    void updateMemoryDisplay();
    QString sizeHuman(uint64_t size);

    Ui::NewDialog *ui;
    uint32_t width = 0;
    uint32_t height = 0;
};

#endif // NEWDIALOG_H
