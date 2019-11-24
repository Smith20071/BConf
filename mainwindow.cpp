/****************************************************************************
**
** Copyright (C) 2019 Igor Kuznetcov
**
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include "QDebug"
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDesktopServices>

extern QSettings *m_config;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_serial(new QSerialPort(this)),
    m_settings(new SettingsDialog)
{

    m_ui->setupUi(this);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->tabWidjet->setEnabled(false);

    m_ui->statusBar->addWidget(m_status);

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    mode = 0;   // Командный режим
}

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

void MainWindow::openSerialPort()
{

    //const SettingsDialog::Settings p = m_settings->settings();
    const SettingsDialog::Settings p = ReadConf();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    PathToBin = p.PathToBin;
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        m_ui->actionUpgrade->setEnabled(true);
        m_ui->actionSave->setEnabled(true);
        m_ui->tabWidjet->setEnabled(true);
        m_ui->actionProfile0->setEnabled(true);
        m_ui->actionProfile1->setEnabled(true);
        m_ui->actionProfile2->setEnabled(true);
        m_ui->actionProfile3->setEnabled(true);
        m_ui->actionProfile10->setEnabled(true);
        m_ui->actionProfile11->setEnabled(true);
        m_ui->actionProfile12->setEnabled(true);
        //m_ui->tabWidjet->setTabEnabled(1, true);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        m_serial->write("ATD=1\r");
        m_serial->write("ATV\r");

    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
    strCmd = "";
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    m_ui->actionUpgrade->setEnabled(false);
    m_ui->actionSave->setEnabled(false);
    m_ui->tabWidjet->setEnabled(false);

    m_ui->actionProfile0->setEnabled(false);
    m_ui->actionProfile1->setEnabled(false);
    m_ui->actionProfile2->setEnabled(false);
    m_ui->actionProfile3->setEnabled(false);
    m_ui->actionProfile10->setEnabled(false);
    m_ui->actionProfile11->setEnabled(false);
    m_ui->actionProfile12->setEnabled(false);

    //m_ui->tabWidjet->setTabEnabled(1, false);
    showStatusMessage(tr("Disconnected"));
    strCmd = "";
}

SettingsDialog::Settings MainWindow::ReadConf() const
{
    SettingsDialog::Settings conf;

    conf.name = m_config->value("connect/port", "COM1").toString();
    conf.baudRate = m_config->value("connect/baudRate", 115200).toInt();

    conf.stringBaudRate = "115200";
    conf.parity = QSerialPort::NoParity;
    conf.stringParity = m_config->value("connect/parity", "NoParity").toString();
    conf.stopBits = QSerialPort::OneStop;
    conf.stringStopBits = m_config->value("connect/stringStopBits", 1).toString();
    conf.dataBits = QSerialPort::Data8;
    conf.stringDataBits = m_config->value("connect/stringDataBits", 8).toString();
    conf.flowControl = QSerialPort::NoFlowControl;
    conf.stringFlowControl = m_config->value("connect/stringFlowControl", "None").toString();
    conf.PathToBin = m_config->value("path", "C:\\").toString();
    return  conf;
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("Конфигуратор"),
                       tr("Конфигуратор модифицированного блока переключетелй "
                          "УАЗ Патриот 56.3769-10М3 Версия 1.06 от 17/11/2019"
                          "Автор: Кузнецов И.С.  "
                          "email: i.s.kuznetcov@mail.ru"));
}

void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
}

void MainWindow::readData()
{
    const QByteArray data = m_serial->readAll();
    if (mode == 0) {
        ParseFrame(data);
    }
    else if (mode == 1) {
        rxBuf = data;
    }
}
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionUpgrade, &QAction::triggered, this, &MainWindow::FW_Upgrade);
}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::SetMode(uint32_t value) {
    mode = value;
}
uint32_t MainWindow::GetMode(void) {
    return  mode;
}

//START,
//ACK_START,
//SEND_ENCODING,
//ACK_ENCODING,
//SEND_ADDR,
//ACK_ADDR,
//SEND_PAGE_SIZE,
//ACK_PAGE_SIZE,
//SEND_COUNT_PAGE,
//ACK_COUNT_PAGE,
//SEND_PAGE,
//ACK_PAGE,
//SEND_CRC,
//ACK_CRC,
//END

void MainWindow::FW_Upgrade(void) {
    const char stuff = 'a';

    state.pageSize = 512;    // Размер страницы

    QByteArray bufPage;
    QByteArray buf32;
    QByteArray buf16;
    QByteArray buftmp;
    QByteArray fw;          // Содержит прошивку
    bufPage.resize(state.pageSize);
    buf32.resize(4);
    buf16.resize(2);
    uint8_t crc;            // CRC блока
    qint64 xRet;
    int32_t page = 0;       // Текущая страница
    bool start = true;
    SetMode(1); // Переключаем программу в режим загрузки прошивки

    state.sign[0] = 'A';
    state.sign[1] = '5';
    state.phase = START;
    state.encoding = 1;
    state.addres = 0x08004000;

//    QMessageBox::information(this, "Внимание","Нажмите и удерживайте кнопку селектора трансмиссии");
//    xRet = m_serial->write("ATBOOT>\r");

    // Считаем путь к файлу из конфига
    QString strDir = m_config->value("PathToBin", QDir::currentPath()).toString();

    // Открываем файл с прошивкой *.bin
    QString fileName = QFileDialog::getOpenFileName(this,
                                QString::fromUtf8("Открыть файл"),
                                strDir,
                                "Bin (*.bin);;All files (*.*)");
    if (fileName.isEmpty()) {
        return;
    }

    QMessageBox::information(this, "Внимание","Для загрузки прошивки, необходимо выключить зажигание, "
                                              "зажать и удерживать кнопку переключения трансмисии "
                                              "и не отпуская кнопку включить зажигание");
    xRet = m_serial->write("ATBOOT>\r");

    //qDebug() << fileName;
    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        // Сохраним путь к файлу
        strDir = QFileInfo(fileName).absolutePath();
        m_config->setValue("PathToBin", strDir);
        QDataStream stream(&file);
        stream.setVersion (QDataStream::Qt_4_2) ;
        if(stream.status() != QDataStream::Ok)
        {
            //qDebug() << "Ошибка чтения файла";
            QMessageBox::warning(this, "Внимание","Ошибка открытия файла!");
            file.close();
            return;
        }
        fw = file.readAll();  // считали весь файл в массив
        state.fwSize = fw.size();   // размер массива
        state.pageCount = state.fwSize / state.pageSize;
        if ( (state.fwSize % state.pageSize) > 0) {
            state.pageCount++;
        }
        qDebug() << "FirmWare Size=" << state.fwSize << " PageSize=" << state.pageSize << " PageCount=" << state.pageCount;
        if (fw.at(304) == (char) 0x0 && fw.at(305) == (char) 0xf0) {            // Проверяем, что это прошивка
            qApp->processEvents();
            fw.append((state.pageSize * state.pageCount - state.fwSize), stuff);                   // выравниваем на размер страницы, байт
            QProgressDialog progress("Upgraging..", "Cancel", 0, 5);
            progress.setRange(0, state.pageCount);
            progress.setModal(true);
            rxBuf.clear();
            while (start) {
                qApp->processEvents();          // передать управление для обработки событий
                //qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                switch (state.phase) {
                case START:                     // Передаем сигнатуру старта передачи
                    buftmp.resize(2);
                    buftmp[0] = (char)state.sign[0];
                    buftmp[1] = (char)state.sign[1];
                    xRet = m_serial->write(buftmp);
                    state.phase = ACK_START;
                    qDebug() << ">>START char send=" << state.sign[0] << state.sign[1];
                    qDebug() << "GO ACK_START";
                    break;
                case ACK_START:                 // Ждем получения ответа
                    if (!rxBuf.isEmpty()) {
                        if (rxBuf.at(0) == 'S') {
                            state.phase = SEND_ENCODING;
                            qDebug() << "<<ACK_START=OK";
                        }
                        else {
                            state.phase = START;
                        }
                        rxBuf.clear();
                    }
                    break;
                case SEND_ENCODING:               // Передаем тип кодирования прошивки
                    xRet = m_serial->putChar(state.encoding);
                    state.phase = ACK_ENCODING;
                    qDebug() << ">>SEND ENCODING=" << QString::number(state.encoding);
                    qDebug() << "GO ACK_ENCODING";
                    break;
                case ACK_ENCODING:                 // Ждем получения ответа
                    if (!rxBuf.isEmpty()) {
                        if (rxBuf.at(0) == 'E') {
                            state.phase = SEND_ADDR;
                            qDebug() << "<<ACK_ENCODING=OK";
                        }
                        rxBuf.clear();
                    }
                    break;
                case SEND_ADDR:               // Передаем адрес начала прошивки
                    buf32[0] = (state.addres & 0xff);
                    buf32[1] = ((state.addres >> 8) & 0xff);
                    buf32[2] = ((state.addres >> 16) & 0xff);
                    buf32[3] = ((state.addres >> 24) & 0xff);
                    xRet = m_serial->write(buf32);
                    state.phase = ACK_ADDR;
                    qDebug() << ">>SEND_ADDR=" << QString::number(state.addres, 16);
                    qDebug() << "GO ACK_ADDR";
                    break;
                case ACK_ADDR:                 // Ждем получения ответа
                    if (rxBuf.at(0) == 'A') {
                        rxBuf.clear();
                        state.phase = SEND_PAGE_SIZE;
                        qDebug() << "<<ACK_ADDR=OK";
                    }
                    break;
                case SEND_PAGE_SIZE:               // Передаем размер страницы
                    buf32[0] = (state.pageSize & 0xff);
                    buf32[1] = ((state.pageSize >> 8) & 0xff);
                    buf32[2] = ((state.pageSize >> 16) & 0xff);
                    buf32[3] = ((state.pageSize >> 24) & 0xff);
                    xRet = m_serial->write(buf32);
                    state.phase = ACK_PAGE_SIZE;
                    qDebug() << ">>SEND_PAGE_SIZE char send=" << xRet;
                    qDebug() << "GO ACK_PAGE_SIZE";
                    break;
                case ACK_PAGE_SIZE:                 // Ждем получения ответа
                    if (rxBuf.at(0) == 'P') {
                        rxBuf.clear();
                        //m_serial->clear();
                        state.phase = SEND_COUNT_PAGE;
                        qDebug() << "<<ACK_PAGE_SIZE=OK";
                    }
                    break;
                case SEND_COUNT_PAGE:               // Передаем количество страниц
                    buf32[0] = (state.pageCount & 0xff);
                    buf32[1] = ((state.pageCount >> 8) & 0xff);
                    buf32[2] = ((state.pageCount >> 16) & 0xff);
                    buf32[3] = ((state.pageCount >> 24) & 0xff);
                    xRet = m_serial->write(buf32);
                    state.phase = ACK_COUNT_PAGE;
                    qDebug() << ">>SEND_COUNT_PAGE=" << state.pageCount;
                    qDebug() << "GO ACK_COUNT_PAGE";
                    break;
                case ACK_COUNT_PAGE:                 // Ждем получения ответа
                    if (rxBuf.at(0) == 'C') {
                        rxBuf.clear();
                        state.phase = SEND_PAGE;
                        qDebug() << "<<ACK_COUNT_PAGE=OK";
                    }
                    break;
                case SEND_PAGE:
                    bufPage = fw.mid(page * state.pageSize, state.pageSize);
                    crc = crc8(bufPage);
                    xRet = m_serial->write(bufPage);
                    progress.setValue(page);
                    state.phase = ACK_PAGE;
                    qDebug() << ">>SEND_PAGE  page=" << page << "x" << xRet << "byte=" << page * xRet + state.pageSize << " CRC =" << QString::number(crc, 16);
                    page++;
                    break;
                case ACK_PAGE:
                    if (rxBuf.at(0) == 'B') {
                        rxBuf.clear();
                        state.phase = SEND_CRC; //
                        qDebug() << "<<ACK_PAGE=OK";
                    }
                    break;
                case SEND_CRC:
                    m_serial->putChar((char)crc);
                    state.phase = ACK_CRC;
                    qDebug() << ">>SEND_CRC=" << crc;
                    break;
                case ACK_CRC:
                    if (rxBuf.at(0) == '\r') {
                        rxBuf.clear();
                        state.phase = (page < state.pageCount) ? SEND_PAGE : END;
                        qDebug() << "<<ACK_CRC=OK";
                    }
                    else if (rxBuf.at(0) == '!') {
                        rxBuf.clear();
                        state.phase = END;
                         QMessageBox::warning(this, "Upgrading","Произошла ошибка!");
                        qDebug() << "<<ACK_CRC=ERROR";
                    }
                    break;
                case END:
                    start = false;
                    qDebug() << "END";
                    break;
                }
                progress.setValue(page);
                if (progress.wasCanceled()) {
                    start = false;
                }
            }
        }
        else
        {
            QMessageBox::warning(this, "Внимание","Файл не является прошивкой!");
        }
    file.close();
    SetMode(0);
    }
}

uint8_t MainWindow::crc8(QByteArray array) {
    uint8_t xRet = 0;
    int sz = array.size();
    for (int i = 0; i < sz; i++) {
        xRet += array.at(i);
    }
    return xRet;
}

void MainWindow::ParseFrame(QString cmd) {
    QString pStr;
    QStringList list_cmd;
    QString strTerminal;

    //qDebug() << "cmd = " << cmd << endl;

    if (!cmd.endsWith("\n\r")) {
        strCmd += cmd;
        return;
    }

    pStr = strCmd + cmd;
    strCmd = "";
    m_ui->Terminal->append(pStr);

    //qDebug() << "pStr = " << pStr << endl;
    pStr = pStr.remove('\n');
    //qDebug() << "pStr = " << pStr << endl;

    list_cmd = pStr.split("\r", QString::SkipEmptyParts);
    for (int i = 0; i < list_cmd.size(); ++i) {
        //qDebug() << "cmd[" << i << "]= " << list_cmd.at(i) << endl;
        if (list_cmd.at(i).startsWith("#>")) {
            // Получили команду. Парсим
            //qDebug() << "cmd[" << i << "]=" << list_cmd.at(i) << endl;
            DecodeCmd(list_cmd.at(i));
        }
    }
}


//*************************************************************************
// Декодирование команд поступивших от stm32
//*************************************************************************
void MainWindow::DecodeCmd(QString cmd) {
    QString arg1 = "";
    QString arg2 = "";
    QStringList list_cmd;
    int nValue;
    //float fValue;

    cmd.remove("#>");
    list_cmd = cmd.split("=", QString::SkipEmptyParts);
    int n_arg = list_cmd.size();

    arg1 = list_cmd.at(0) + "=";

    if (n_arg > 0) {
        arg2 = list_cmd.at(1);
    }
    qDebug() << "arg1=" << arg1 << "arg2=" << arg2 << endl;

    //****************************************************************
    // Управление индикацией топлива в баках
    if (arg1.startsWith("FM=")) {
        nValue = arg2.toInt();
        if (nValue == 0) {
            m_ui->fm0->setChecked(true);
        }
        else if (nValue == 1) {
            if (m_ui->checkBoxFS->checkState()) {
                m_ui->fm2->setChecked(true);
            } else {
                m_ui->fm1->setChecked(true);
            }
        }
        else if (nValue == 2) {
            if (m_ui->checkBoxFS->checkState()) {
                m_ui->fm1->setChecked(true);
            }
            else {
                m_ui->fm2->setChecked(true);
            }
        }
        else if (nValue == 3) {
            m_ui->fm3->setChecked(true);
        }
    }
    else if (arg1.startsWith("FS=")) {
        nValue = arg2.toInt();
        if (nValue == 0) {
            m_ui->checkBoxFS->setCheckState(Qt::CheckState::Unchecked);
        } else {
            m_ui->checkBoxFS->setCheckState(Qt::CheckState::Checked);
        }
    }
    else if (arg1.startsWith("FL=")) {
        m_ui->FL->setText(arg2);
    }
    else if (arg1.startsWith("FR=")) {
        m_ui->FR->setText(arg2);
    }
    else if (arg1.startsWith("FLA0=")) {
        m_ui->FLA0->setText(arg2);
    }
    else if (arg1.startsWith("FLA1=")) {
        m_ui->FLA1->setText(arg2);
    }
    else if (arg1.startsWith("FLD0=")) {
        m_ui->FLD0->setText(arg2);
    }
    else if (arg1.startsWith("FLD1=")) {
        m_ui->FLD1->setText(arg2);
    }
    else if (arg1.startsWith("FLPW0=")) {
        m_ui->FLPW0->setText(arg2);
    }
    else if (arg1.startsWith("FLPW1=")) {
        m_ui->FLPW1->setText(arg2);
    }
    else if (arg1.startsWith("FLPA0=")) {
        m_ui->FLPA0->setText(arg2);
    }
    else if (arg1.startsWith("FLPA1=")) {
        m_ui->FLPA1->setText(arg2);
    }
    else if (arg1.startsWith("FLPD0=")) {
        m_ui->FLPD0->setText(arg2);
    }
    else if (arg1.startsWith("FLPD1=")) {
        m_ui->FLPD1->setText(arg2);
    }
    else if (arg1.startsWith("FRA0=")) {
        m_ui->FRA0->setText(arg2);
    }
    else if (arg1.startsWith("FRA1=")) {
        m_ui->FRA1->setText(arg2);
    }
    else if (arg1.startsWith("FRD0=")) {
        m_ui->FRD0->setText(arg2);
    }
    else if (arg1.startsWith("FRD1=")) {
        m_ui->FRD1->setText(arg2);
    }
    else if (arg1.startsWith("FRPW0=")) {
        m_ui->FRPW0->setText(arg2);
    }
    else if (arg1.startsWith("FRPW1=")) {
        m_ui->FRPW1->setText(arg2);
    }
    else if (arg1.startsWith("FRPA0=")) {
        m_ui->FRPA0->setText(arg2);
    }
    else if (arg1.startsWith("FRPA1=")) {
        m_ui->FRPA1->setText(arg2);
    }
    else if (arg1.startsWith("FRPD0=")) {
        m_ui->FRPD0->setText(arg2);
    }
    else if (arg1.startsWith("FRPD1=")) {
        m_ui->FRPD1->setText(arg2);
    }
    else if (arg1.startsWith("FRPD1=")) {
        m_ui->FRPD1->setText(arg2);
    }
    else if (arg1.startsWith("FLK=")) {
        m_ui->FLK->setText(arg2);
    }
    else if (arg1.startsWith("FLB=")) {
        m_ui->FLB->setText(arg2);
    }
    else if (arg1.startsWith("FRK=")) {
        m_ui->FRK->setText(arg2);
    }
    else if (arg1.startsWith("FRB=")) {
        m_ui->FRB->setText(arg2);
    }
    else if (arg1.startsWith("FLUADC=")) {
        m_ui->FU_ADC->setText(arg2);
    }
    else if (arg1.startsWith("FLUDAC=")) {
        m_ui->FU_DAC->setText(arg2);
    }
    else if (arg1.startsWith("E=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->checkEngine->setCheckState(Qt::CheckState::Unchecked);
            break;
        default:
            m_ui->checkEngine->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    //****************************************************************
    // Управление селектором трансмиссии
    else if (arg1.startsWith("DM=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->Drive_2H->setChecked(true);
            break;
        case 1:
            m_ui->Drive_4H->setChecked(true);
            break;
        case 2:
            m_ui->Drive_4L->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("DV=")) {
        m_ui->Deive_MaxSpeed->setText(arg2);
    }
    //****************************************************************
    // Управление подогревом сидений
    else if (arg1.startsWith("T=")) {
        m_ui->TCurrent->setText(arg2);
    }
    else if (arg1.startsWith("+HT=")) {
        m_ui->HTA->setText(arg2);
    }
    else if (arg1.startsWith("HML=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->HML0->setChecked(true);
            break;
        case 1:
            m_ui->HML1->setChecked(true);
            break;
        case 2:
            m_ui->HML2->setChecked(true);
            break;
        case 3:
            m_ui->HML3->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("HMR=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->HMR0->setChecked(true);
            break;
        case 1:
            m_ui->HMR1->setChecked(true);
            break;
        case 2:
            m_ui->HMR2->setChecked(true);
            break;
        case 3:
            m_ui->HMR3->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("HTL=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->HTL->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->HTL->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("HTR=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->HTR->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->HTR->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("HL1=")) {
        m_ui->HL1->setText(arg2);
    }
    else if (arg1.startsWith("HL2=")) {
        m_ui->HL2->setText(arg2);
    }
    else if (arg1.startsWith("HL3=")) {
        m_ui->HL3->setText(arg2);
    }
    else if (arg1.startsWith("+H0=")) {
        m_ui->H0->setText(arg2);
    }
    else if (arg1.startsWith("+H1=")) {
        m_ui->H1->setText(arg2);
    }
    else if (arg1.startsWith("+H2=")) {
        m_ui->H2->setText(arg2);
    }
    else if (arg1.startsWith("+H3=")) {
        m_ui->H3->setText(arg2);
    }
    else if (arg1.startsWith("+T0=")) {
        m_ui->T0->setText(arg2);
    }
    else if (arg1.startsWith("+T1=")) {
        m_ui->T1->setText(arg2);
    }
    else if (arg1.startsWith("+T2=")) {
        m_ui->T2->setText(arg2);
    }
    else if (arg1.startsWith("+T3=")) {
        m_ui->T3->setText(arg2);
    }

    //****************************************************************
    // CAN
    else if (arg1.startsWith("CAN=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->CAN0->setChecked(true);
            break;
        case 1:
            m_ui->CAN1->setChecked(true);
            break;
        case 2:
            m_ui->CAN2->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("CAC=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->CANC->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->CANC->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("CANE=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->CANE0->setChecked(true);
            break;
        case 1:
            m_ui->CANE1->setChecked(true);
            break;
        case 2:
            m_ui->CANE2->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("CANS=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->CANS0->setChecked(true);
            break;
        case 1:
            m_ui->CANS1->setChecked(true);
            break;
        case 2:
            m_ui->CANS2->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("CANT=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->CANT0->setChecked(true);
            break;
        case 1:
            m_ui->CANT1->setChecked(true);
            break;
        case 2:
            m_ui->CANT2->setChecked(true);
            break;
        }
    }
    //****************************************************************
    // Канал компрессора
    else if (arg1.startsWith("PEN=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->PEN0->setChecked(true);
            break;
        case 1:
            m_ui->PEN1->setChecked(true);
            break;
        case 2:
            m_ui->PEN2->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("PM=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->PM->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->PM->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("P=")) {
        m_ui->P_Current->setText(arg2);
    }
    else if (arg1.startsWith("P0=")) {
        m_ui->P0->setText(arg2);
    }
    else if (arg1.startsWith("P1=")) {
        m_ui->P1->setText(arg2);
    }
    else if (arg1.startsWith("P2=")) {
        m_ui->P2->setText(arg2);
    }
    else if (arg1.startsWith("PT=")) {
        m_ui->PT->setText(arg2);
    }
    else if (arg1.startsWith("PU=")) {
        m_ui->PU->setText(arg2);
    }
    //****************************************************************
    // Канал блокировок
    else if (arg1.startsWith("LFEN=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->LFEN0->setChecked(true);
            break;
        case 1:
            m_ui->LFEN1->setChecked(true);
            break;
        case 2:
            m_ui->LFEN2->setChecked(true);
            break;
        case 3:
            m_ui->LFEN3->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("LREN=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->LREN0->setChecked(true);
            break;
        case 1:
            m_ui->LREN1->setChecked(true);
            break;
        case 2:
            m_ui->LREN2->setChecked(true);
            break;
        case 3:
            m_ui->LREN3->setChecked(true);
            break;
        }
    }
    else if (arg1.startsWith("LFM=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->LFM->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->LFM->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("LRM=")) {
        switch (arg2.toInt()) {
        case 0:
            m_ui->LRM->setCheckState(Qt::CheckState::Unchecked);
            break;
        case 1:
            m_ui->LRM->setCheckState(Qt::CheckState::Checked);
            break;
        }
    }
    else if (arg1.startsWith("LF0=")) {
        m_ui->LF0->setText(arg2);
    }
    else if (arg1.startsWith("LF1=")) {
        m_ui->LF1->setText(arg2);
    }
    else if (arg1.startsWith("LR0=")) {
        m_ui->LR0->setText(arg2);
    }
    else if (arg1.startsWith("LR1=")) {
        m_ui->LR1->setText(arg2);
    }
    //****************************************************************
    // Информация
    else if (arg1.startsWith("HW=")) {
        m_ui->HW->setText(arg2);
    }
    else if (arg1.startsWith("FW=")) {
        m_ui->FW->setText(arg2);
    }
    else if (arg1.startsWith("BL=")) {
        m_ui->BUILD->setText(arg2);
    }
    else if (arg1.startsWith("CNF=")) {
        m_ui->CONFIG->setText(arg2);
    }


    int f_s = (m_ui->FL->text().toInt() + m_ui->FR->text().toInt()) / 2;
    m_ui->FSUM->setNum(f_s);
} // DecodeCmd()

//************************************************************************


void MainWindow::SendCmd(QString cmd) {
    QByteArray baCmd;
    baCmd.append(cmd);
    m_ui->Terminal->append(cmd);
    MainWindow::writeData(baCmd);
}


void MainWindow::on_fm0_clicked()
{
    SendCmd("ATFM=0\r");
}

void MainWindow::on_fm1_clicked()
{
    if (m_ui->checkBoxFS->checkState()) {
        SendCmd("ATFM=2\r");
    }
    else {
        SendCmd("ATFM=1\r");
    }
    SendCmd("ATFL?\r");
    SendCmd("ATFR?\r");
}

void MainWindow::on_fm2_clicked()
{
    if (m_ui->checkBoxFS->checkState()) {
        SendCmd("ATFM=1\r");
    }
    else {
        SendCmd("ATFM=2\r");
    }
    SendCmd("ATFL?\r");
    SendCmd("ATFR?\r");
}


void MainWindow::on_fm3_clicked()
{
    SendCmd("ATFM=3\r");
    SendCmd("ATFL?\r");
    SendCmd("ATFR?\r");
}

void MainWindow::on_ButtonReadFuel_clicked()
{
    SendCmd("ATF?\r");
    SendCmd("ATFM?\r");
    SendCmd("AT+F?\r");
    SendCmd("ATFL?\r");
    SendCmd("ATFLI?\r");
    SendCmd("ATFR?\r");
    SendCmd("ATFRI?\r");
    SendCmd("ATFS?\r");
    SendCmd("ATFU?\r");
    SendCmd("ATE?\r");
}

void MainWindow::on_checkBoxFS_stateChanged(int arg1)
{
    if (arg1 == 0) {
        SendCmd("ATFS=0\r");
    } else {
        SendCmd("ATFS=1\r");
    }
    SendCmd("ATFM?\r");
}

void MainWindow::on_ButtonCalibrL_clicked()
{
    SendCmd("ATFL#\r");
}

void MainWindow::on_ButtonCalibrR_clicked()
{
    SendCmd("ATFR#\r");
}

void MainWindow::on_ButtonReadDeive_clicked()
{
    SendCmd("ATDM?\r");
    SendCmd("ATDV?\r");
}

void MainWindow::on_actionRead_triggered()
{
    QDesktopServices::openUrl(QUrl("https://yadi.sk/d/G_Bwbwvz0Vh8dA"));
}

void MainWindow::on_Drive_2H_clicked()
{
    SendCmd("ATDM=0\r");
}

void MainWindow::on_Drive_4H_clicked()
{
    SendCmd("ATDM=1\r");
}

void MainWindow::on_Drive_4L_clicked()
{
    SendCmd("ATDM=2\r");
}

void MainWindow::on_Deive_MaxSpeed_editingFinished()
{
    QString strCmd = m_ui->Deive_MaxSpeed->text();
    strCmd = "ATDV=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_BTReadHeat_clicked()
{
    SendCmd("ATHAR?\r");
}

void MainWindow::on_BTReadCAN_clicked()
{
    SendCmd("ATCAN?\r");
}

void MainWindow::on_BTReadAIR_clicked()
{
    SendCmd("ATP?\r");
    SendCmd("ATPEN?\r");
    SendCmd("ATP0?\r");
    SendCmd("ATP1?\r");
    SendCmd("ATP2?\r");
    SendCmd("ATPM?\r");
    SendCmd("ATP?\r");
    SendCmd("ATPT?\r");
    SendCmd("ATPU?\r");
}

void MainWindow::on_BTReadLock_clicked()
{
    SendCmd("ATLFEN?\r");
    SendCmd("ATLREN?\r");
    SendCmd("ATLFM?\r");
    SendCmd("ATLRM?\r");
    SendCmd("ATLF0?\r");
    SendCmd("ATLF1?\r");
    SendCmd("ATLR0?\r");
    SendCmd("ATLR1?\r");
}

void MainWindow::on_actionProfile0_triggered()
{
    SendCmd("ATZ\r");
}

void MainWindow::on_actionProfile1_triggered()
{
    SendCmd("ATZ1\r");
}

void MainWindow::on_actionProfile2_triggered()
{
    SendCmd("ATZ2\r");
}

void MainWindow::on_actionProfile3_triggered()
{
    SendCmd("ATZ3\r");
}

void MainWindow::on_HML0_clicked()
{
    SendCmd("ATHML=0\r");
}

void MainWindow::on_HML1_clicked()
{
    SendCmd("ATHML=1\r");
}

void MainWindow::on_HML2_clicked()
{
    SendCmd("ATHML=2\r");
}

void MainWindow::on_HML3_clicked()
{
    SendCmd("ATHML=3\r");
}

void MainWindow::on_HMR0_clicked()
{
    SendCmd("ATHMR=0\r");
}

void MainWindow::on_HMR1_clicked()
{
    SendCmd("ATHMR=1\r");
}

void MainWindow::on_HMR2_clicked()
{
    SendCmd("ATHMR=2\r");
}

void MainWindow::on_HMR3_clicked()
{
    SendCmd("ATHMR=3\r");
}


void MainWindow::on_CAN0_clicked()
{
    SendCmd("ATCAN=0\r");
}

void MainWindow::on_CAN1_clicked()
{
    SendCmd("ATCAN=1\r");
}

void MainWindow::on_CAN2_clicked()
{
    SendCmd("ATCAN=2\r");
}

void MainWindow::on_CANC_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATCANC=0\r");
        break;
    default:
        SendCmd("ATCANC=1\r");
        break;
    }
}

void MainWindow::on_CANE0_clicked()
{
    SendCmd("ATCANE=0\r");
}

void MainWindow::on_CANE1_clicked()
{
    SendCmd("ATCANC=1\r");
}


void MainWindow::on_CANE2_clicked()
{
    SendCmd("ATCANC=2\r");
}

void MainWindow::on_CANS0_clicked()
{
    SendCmd("ATCANS=0\r");
}

void MainWindow::on_CANS1_clicked()
{
    SendCmd("ATCANS=1\r");
}

void MainWindow::on_CANS2_clicked()
{
    SendCmd("ATCANS=2\r");
}

void MainWindow::on_actionSave_triggered()
{
    SendCmd("ATW\r");
}

void MainWindow::on_PEN0_clicked()
{
    SendCmd("ATPEN=0\r");
}

void MainWindow::on_PEN1_clicked()
{
    SendCmd("ATPEN=1\r");
}

void MainWindow::on_PEN2_clicked()
{
    SendCmd("ATPEN=2\r");
}

void MainWindow::on_PM_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATPM=0\r");
        break;
    default:
        SendCmd("ATPM=1\r");
        break;
    }
}

void MainWindow::on_P0_editingFinished()
{
    QString strCmd = m_ui->P0->text();
    strCmd = "ATP0=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_P1_editingFinished()
{
    QString strCmd = m_ui->P1->text();
    strCmd = "ATP1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_P2_editingFinished()
{
    QString strCmd = m_ui->P2->text();
    strCmd = "ATP2=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_PT_editingFinished()
{
    QString strCmd = m_ui->PT->text();
    strCmd = "ATPT=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_PU_editingFinished()
{
    QString strCmd = m_ui->PU->text();
    strCmd = "ATPU=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_LFEN0_clicked()
{
    SendCmd("ATLFEN=0\r");
}

void MainWindow::on_LFEN1_clicked()
{
    SendCmd("ATLFEN=1\r");
}

void MainWindow::on_LFEN2_clicked()
{
    SendCmd("ATLFEN=2\r");
}

void MainWindow::on_LFEN3_clicked()
{
    SendCmd("ATLFEN=3\r");
}

void MainWindow::on_LREN0_clicked()
{
    SendCmd("ATLREN=0\r");
}

void MainWindow::on_LREN1_clicked()
{
    SendCmd("ATLREN=1\r");
}

void MainWindow::on_LREN2_clicked()
{
    SendCmd("ATLREN=2\r");
}

void MainWindow::on_LREN3_clicked()
{
    SendCmd("ATLREN=3\r");
}

void MainWindow::on_LFM_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATLFM=0\r");
        break;
    default:
        SendCmd("ATLFM=1\r");
        break;
    }
}

void MainWindow::on_LRM_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATLRM=0\r");
        break;
    default:
        SendCmd("ATLRM=1\r");
        break;
    }
}

void MainWindow::on_LF0_editingFinished()
{
    QString strCmd = m_ui->LF0->text();
    strCmd = "ATLF0=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_LF1_editingFinished()
{
    QString strCmd = m_ui->LF1->text();
    strCmd = "ATLF1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_LR0_editingFinished()
{
    QString strCmd = m_ui->LR0->text();
    strCmd = "ATLR0=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_LR1_editingFinished()
{
    QString strCmd = m_ui->LR1->text();
    strCmd = "ATLR1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_H0_editingFinished()
{
    QString strCmd = m_ui->H0->text();
    strCmd = "AT+H0=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_H1_editingFinished()
{
    QString strCmd = m_ui->H1->text();
    strCmd = "AT+H1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_H2_editingFinished()
{
    QString strCmd = m_ui->H2->text();
    strCmd = "AT+H2=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_H3_editingFinished()
{
    QString strCmd = m_ui->H3->text();
    strCmd = "AT+H3=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_T0_editingFinished()
{
    QString strCmd = m_ui->T0->text();
    strCmd = "AT+T0=" + strCmd + "\r";
    SendCmd(strCmd);
}


void MainWindow::on_T1_editingFinished()
{
    QString strCmd = m_ui->T1->text();
    strCmd = "AT+T1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_T2_editingFinished()
{
    QString strCmd = m_ui->T2->text();
    strCmd = "AT+T2=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_T3_editingFinished()
{
    QString strCmd = m_ui->T3->text();
    strCmd = "AT+T3=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_HL3_editingFinished()
{
    QString strCmd = m_ui->HL3->text();
    strCmd = "ATHL3=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_HL2_editingFinished()
{
    QString strCmd = m_ui->HL2->text();
    strCmd = "ATHL2=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_HL1_editingFinished()
{
    QString strCmd = m_ui->HL1->text();
    strCmd = "ATHL1=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_HTL_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATHTL=0\r");
        break;
    default:
        SendCmd("ATHTL=1\r");
        break;
    }
}

void MainWindow::on_HTR_stateChanged(int arg1)
{
    switch (arg1) {
    case 0:
        SendCmd("ATHTR=0\r");
        break;
    default:
        SendCmd("ATHTR=1\r");
        break;
    }
}

//Help
void MainWindow::on_action_Help_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile("help.mht"));
}

void MainWindow::on_HTA_editingFinished()
{
    QString strCmd = m_ui->HTA->text();
    strCmd = "AT+HT=" + strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_BtSendCMD_clicked()
{
    QString strCmd = m_ui->lineEditCMD->text();
    qDebug() << strCmd;
    strCmd = strCmd + "\r";
    SendCmd(strCmd);
}

void MainWindow::on_action_url_triggered()
{
    QDesktopServices::openUrl(QUrl("https://disk.yandex.ru/client/disk/56.3769-10/bootloader/BConf3"));
}
