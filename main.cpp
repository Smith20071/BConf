/****************************************************************************
**
** Copyright (C)
** Copyright (C)
** Contact:
**
**
****************************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

QSettings *m_config = nullptr;

//m_config(new QSettings("bconf.conf",QSettings::IniFormat))

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    m_config = new QSettings("bconf.conf",QSettings::IniFormat);
    MainWindow w;
    w.show();
    return a.exec();
}
