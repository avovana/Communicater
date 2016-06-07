#include "mainwindow.h"
#include "port.h"
#include "ui_mainwindow.h"
//#include <QDebug>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_timerWaitForReply = new QTimer(this);
    m_timerWaitForReply->setInterval(100);
    m_timerWaitForReply->setSingleShot(true);       //делает себе stop

//-----------------Действия при тайм-ауте таймера в зависимости от флагов--------------------
    connect(m_timerWaitForReply, &QTimer::timeout, this, [this](){

        switch (m_currentStatusOfRequest) {
        case ReadNumberRequest:
            ++m_currentAdress;
            ui->groupBoxWorkWithDevice->setTitle(QString ("Поиск, адрес %1").arg(m_currentAdress));
            requestReadDeviceNumber(m_currentAdress);
            break;

        case ReadMaskRequest:
            m_currentStatusOfRequest = None;
            break;

        case WriteRegisterAdrRz1Request:
            m_currentStatusOfRequest = None;

            switch (m_writtingRegisterAdrRz1) {
            case ForWriteAdress:
                m_writtingRegisterAdrRz1 = DontUseAdrRz1;
                requestWriteAdress(takeInfoFromGui(Adress));
                return;

            case ForWriteMask:
                m_writtingRegisterAdrRz1 = DontUseAdrRz1;
                requestWriteMask(takeInfoFromGui(Mask));
                return;

            default:
                break;
            }
        case WriteAdressRequest:
            m_currentStatusOfRequest = None;
            ui->pushButton_writeAdress->setEnabled(true);

            break;
        case WriteMaskRequest:
            m_currentStatusOfRequest = None;
            ui->pushButton_writeMask ->setEnabled(true);

            break;

        default:
            break;
        }

    });

    m_portCommunication = new Port(this);//Создаем обьект по классу
    connect(m_portCommunication, &Port::sendReceivedData, this, &MainWindow::receivedDataFromPort);
    connect(ui->BaudRateBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkCustomBaudRatePolicy(int)));

    auto connectPortLambda =  [this] () {
        m_portCommunication->connectPort(ui->PortNameBox->currentText(), ui->BaudRateBox->currentText().toInt(), QSerialPort::Data8,
                                       QSerialPort::EvenParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);
    };

    connect(ui->BaudRateBox, &QComboBox::currentTextChanged, connectPortLambda);
    connect(ui->PortNameBox, &QComboBox::currentTextChanged, connectPortLambda);

    connect(ui->about_programm, SIGNAL(triggered()), SLOT(about()));

    ui->BaudRateBox->addItem(QLatin1String("9600"), QSerialPort::Baud9600);
    ui->BaudRateBox->addItem(QLatin1String("19200"), QSerialPort::Baud19200);
    ui->BaudRateBox->addItem(QLatin1String("38400"), QSerialPort::Baud38400);
    ui->BaudRateBox->addItem(QLatin1String("115200"), QSerialPort::Baud115200);
    ui->BaudRateBox->addItem(QLatin1String("460800"), 460800);
    ui->BaudRateBox->addItem(QLatin1String("1500000"), 1500000);
    ui->BaudRateBox->addItem(QLatin1String("Custom"));
    ui->BaudRateBox->setCurrentIndex(5);

//---------------Подключение к порту сразу при запуске программы----------------------------------
    ui->PortNameBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->PortNameBox->addItem(info.portName());
    }
    //вызовется connect(ui->PortNameBox, &QComboBox::currentTextChanged, connectPortLambda);
//----------------------------------------------------------------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------Старт алгоритма по посику портов--------------------
void MainWindow::on_Btn_Serch_clicked()
{
    ui->PortNameBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->PortNameBox->addItem(info.portName());
    }
}

void MainWindow::checkCustomBaudRatePolicy(int idx)
{
    bool isCustomBaudRate = !ui->BaudRateBox->itemData(idx).isValid();
    ui->BaudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        ui->BaudRateBox->clearEditText();
    }
}

//-----------------Старт алгоритма по записи маски--------------------
void MainWindow::on_button_search_clicked()
{ 
    if ((ui->button_search->text()) == "Стоп")
    {
        m_timerWaitForReply->stop();
        ui->button_search->setText("Поиск прибора");
        ui->groupBoxWorkWithDevice->setTitle("Работа с прибором");
        return;
    }
    ui->lineEditAdress->setText("");
    ui->lineEditMask->setText("");

    m_currentAdress = 0;
    ++m_currentAdress;
    ui->groupBoxWorkWithDevice->setTitle(QString ("Поиск, адрес %1").arg(m_currentAdress));
    ui->button_search->setText("Стоп");
    requestReadDeviceNumber(m_currentAdress);
}

//-----------------Старт алгоритма по записи адреса--------------------
void MainWindow::on_pushButton_writeAdress_clicked()
{
    m_writtingRegisterAdrRz1 = ForWriteAdress;
    requestWriteRegisterAdrRz1();
}

//-----------------Запись адреса--------------------
void MainWindow::requestWriteAdress(ushort adress)
{
    m_currentStatusOfRequest = WriteAdressRequest;
    ui->pushButton_writeAdress->setEnabled(false);

    QByteArray send_modb;
    send_modb.append((char)0x00);
    send_modb.append(0x10);
    send_modb.append((char)0x00);
    send_modb.append((char)0x00);
    send_modb.append((char)0x00);
    send_modb.append(0x01);
    send_modb.append(0x02);

    send_modb.append((uchar)(adress >> 8));
    send_modb.append((uchar)(adress));

    m_timerWaitForReply->start();
    m_portCommunication->writeToPort(send_modb);
}

//-----------------Обработка пришедшего сообщения--------------------
void MainWindow::receivedDataFromPort(const QByteArray &data)
{
    m_timerWaitForReply->stop();
    if (data.count() < 6)
        return;

    uchar adress = data.at(0);

    switch (m_currentStatusOfRequest)
    {
    case ReadNumberRequest:
        m_currentStatusOfRequest = None;
        ui->lineEditAdress->setText(QString::number(adress));
        requestReadMask(adress);
        break;

    case ReadMaskRequest:
        m_currentStatusOfRequest = None;
        ushort mask;
        mask = (unsigned char)data[4];
        ui->button_search->setText("Поиск прибора");
        ui->groupBoxWorkWithDevice->setTitle("Работа с прибором");
        ui->lineEditMask->setText(QString("%1").arg(mask));
        m_currentAdress = adress;
        break;

    default:
        break;
    }
}

//-----------------Запись номера--------------------
void MainWindow::requestReadDeviceNumber(uchar adress)
{
    m_timerWaitForReply->start();
    m_currentStatusOfRequest = ReadNumberRequest;

    QByteArray send_modb;
    send_modb.append(adress);
    send_modb.append(0x03);
    send_modb.append((char)0x00);
    send_modb.append(0x01);
    send_modb.append((char)0x00);
    send_modb.append(0x01);

    m_portCommunication->writeToPort(send_modb);
}

//-----------------Чтение маски--------------------
void MainWindow::requestReadMask(uchar adress)
{
    m_timerWaitForReply->start();
    m_currentStatusOfRequest = ReadMaskRequest;

    QByteArray send_modb;
    send_modb.append(adress);
    send_modb.append(0x03);
    send_modb.append((char)0x00);
    send_modb.append(0x03);
    send_modb.append((char)0x00);
    send_modb.append(0x01);

    m_portCommunication->writeToPort(send_modb);
}

//-----------------Старт алгоритма по записи маски--------------------
void MainWindow::on_pushButton_writeMask_clicked()
{
    m_writtingRegisterAdrRz1 = ForWriteMask;
    requestWriteRegisterAdrRz1();
}

//-----------------Получение данных с интерфейса--------------------
ushort MainWindow::takeInfoFromGui(AvalibleInfoInGui currentInfoNeedToTake)
{
    ushort data = 0;
    switch (currentInfoNeedToTake){
    case Adress:
        data = ui->lineEditAdress->text().toUShort();
        break;

    case Mask:
        data = ui->lineEditMask->text().toUShort();
        break;

    default:
        break;
    }
    return data;
}

//-----------------Запись маски--------------------
void MainWindow::requestWriteMask(ushort mask)
{    
    m_currentStatusOfRequest = WriteMaskRequest;
    ui->pushButton_writeMask ->setEnabled(false);

    QByteArray send_modb;
    send_modb.append((char)0x00);
    send_modb.append(0x10);
    send_modb.append((char)0x00);
    send_modb.append(0x03);
    send_modb.append((char)0x00);
    send_modb.append(0x01);
    send_modb.append(0x02);
    send_modb.append((uchar)(mask >> 8));
    send_modb.append((uchar)(mask));

    m_timerWaitForReply->start();
    m_portCommunication->writeToPort(send_modb);
}

QString MainWindow::converterDecToHex(ushort data)
{
    return QString("%1$%2").arg((uchar)(data >> 8), 0, 16).arg((uchar)data, 0, 16);
}

//-----------------Запись регистра AdrRz1--------------------
void MainWindow::requestWriteRegisterAdrRz1()
{
    m_timerWaitForReply->start();
    m_currentStatusOfRequest = WriteRegisterAdrRz1Request;
    QByteArray send_modb;
    send_modb.append((char)0x00);
    send_modb.append(0x10);
    send_modb.append(0x10);
    send_modb.append(0x06);
    send_modb.append((char)0x00);
    send_modb.append(0x01);
    send_modb.append(0x02);
    send_modb.append(0x57);
    send_modb.append(0x68);
    m_portCommunication->writeToPort(send_modb);
}
