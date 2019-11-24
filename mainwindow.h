/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSettings>
#include "QThread"
#include "settingsdialog.h"

QT_BEGIN_NAMESPACE

class QLabel;

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

//class Console;
class SettingsDialog;

int ParseInt(QString str);


// Внимание! Для корректной работы необходимо соответствие
// кодов ACK_* с одноименными кодами ACK_* загрузчика микроконтроллера
enum PHASE {
    ACK_START           = 1,
    ACK_ENCODING        = 2,
    ACK_ADDR            = 3,
    ACK_PAGE_SIZE       = 4,
    ACK_COUNT_PAGE      = 5,
    ACK_PAGE            = 6,
    ACK_CRC             = 7,
    START               = 8,
    SEND_ENCODING       = 9,
    SEND_ADDR           = 10,
    SEND_PAGE_SIZE      = 11,
    SEND_COUNT_PAGE     = 12,
    SEND_PAGE           = 13,
    SEND_CRC            = 14,
    END                 = 15
};

typedef struct {
  uint8_t  sign[2];         // Стартовая сигнатура
  PHASE    phase;           // Текущая фаза
  uint8_t  encoding;        // Режим кодировки (0 - без кодировки)
  uint32_t addres;          // Стартовый адрес прошивки
  int32_t  fwSize;          // Размер файла
  int32_t  pageSize;        // Размер блока в байтах (Важно: Кратно 4)
  int32_t  pageCount;       // Количество блоков
  int32_t  ErrorCode;
} t_state;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ParseFrame(QString cmd);
    void DecodeCmd(QString cmd);
    void FW_Upgrade(void);
    SettingsDialog::Settings ReadConf() const;
    void SetMode(uint32_t value);
    uint32_t GetMode(void);
    QByteArray rxBuf;

private slots:
    void openSerialPort();
    void closeSerialPort();
    void about();
    void writeData(const QByteArray &data);
    void readData();
    void handleError(QSerialPort::SerialPortError error);
    void on_fm0_clicked();
    void on_fm1_clicked();
    void on_fm2_clicked();
    void on_fm3_clicked();
    void on_ButtonReadFuel_clicked();
    void on_checkBoxFS_stateChanged(int arg1);
    void on_ButtonCalibrL_clicked();
    void on_ButtonCalibrR_clicked();
    void on_ButtonReadDeive_clicked();
    void on_actionRead_triggered();
    void on_Drive_2H_clicked();
    void on_Drive_4H_clicked();
    void on_Drive_4L_clicked();
    void on_Deive_MaxSpeed_editingFinished();
    void on_BTReadHeat_clicked();
    void on_BTReadCAN_clicked();
    void on_BTReadAIR_clicked();
    void on_BTReadLock_clicked();
    void on_actionProfile0_triggered();
    void on_actionProfile1_triggered();
    void on_actionProfile2_triggered();
    void on_actionProfile3_triggered();
    void on_HML0_clicked();
    void on_HML1_clicked();
    void on_HML2_clicked();
    void on_HML3_clicked();
    void on_HMR0_clicked();
    void on_HMR1_clicked();
    void on_HMR2_clicked();
    void on_HMR3_clicked();
    void on_CAN0_clicked();
    void on_CAN1_clicked();
    void on_CAN2_clicked();
    void on_CANC_stateChanged(int arg1);
    void on_CANE0_clicked();
    void on_CANE1_clicked();
    void on_CANE2_clicked();
    void on_CANS0_clicked();
    void on_CANS1_clicked();
    void on_CANS2_clicked();
    void on_actionSave_triggered();
    void on_PEN0_clicked();
    void on_PEN1_clicked();
    void on_PEN2_clicked();
    void on_PM_stateChanged(int arg1);
    void on_P0_editingFinished();
    void on_P1_editingFinished();
    void on_P2_editingFinished();
    void on_PT_editingFinished();
    void on_PU_editingFinished();
    void on_LFEN0_clicked();
    void on_LFEN1_clicked();
    void on_LFEN2_clicked();
    void on_LFEN3_clicked();
    void on_LREN0_clicked();
    void on_LREN1_clicked();
    void on_LREN2_clicked();
    void on_LREN3_clicked();
    void on_LFM_stateChanged(int arg1);
    void on_LRM_stateChanged(int arg1);
    void on_LF0_editingFinished();
    void on_LF1_editingFinished();
    void on_LR0_editingFinished();
    void on_LR1_editingFinished();
    void on_H0_editingFinished();
    void on_H1_editingFinished();
    void on_H2_editingFinished();
    void on_H3_editingFinished();
    void on_T0_editingFinished();
    void on_T1_editingFinished();
    void on_T2_editingFinished();
    void on_T3_editingFinished();
    void on_HL3_editingFinished();
    void on_HL2_editingFinished();
    void on_HL1_editingFinished();

    void on_HTL_stateChanged(int arg1);

    void on_HTR_stateChanged(int arg1);

    void on_action_Help_triggered();

    void on_HTA_editingFinished();

    void on_BtSendCMD_clicked();

    void on_action_url_triggered();

private:
    void initActionsConnections();

private:
    void showStatusMessage(const QString &message);
    void SendCmd(QString cmd);
    t_state state;
    QString PathToBin;
    uint8_t crc8(QByteArray array);
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    QSerialPort *m_serial = nullptr;
    SettingsDialog *m_settings = nullptr;
    QString strCmd;
    uint32_t    mode;       // 0 - нормальный режим. Декодирование команд, 1 - режим загрузчика
};

#endif // MAINWINDOW_H
