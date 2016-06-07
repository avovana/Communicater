#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//-----------------Задействованные классы--------------------
class Port;          
class QSerialPort;

//-----------------Описание флагов и их возможных состояний--
enum StatusOfRequest {
    None,
    ReadNumberRequest,
    ReadMaskRequest,
    WriteRegisterAdrRz1Request,
    WriteAdressRequest,
    WriteMaskRequest
};

enum PorposeOfWrittingRegisterAdrRz1 {
    DontUseAdrRz1,
    ForWriteAdress,
    ForWriteMask
};

enum AvalibleInfoInGui {
    DontUseInfo,
    Adress,
    Mask
};

//-----------------Задействованный интерфейс--------------------
namespace Ui {
class MainWindow;
}

//-----------------Описание основного рабочего класса-----------
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	//-----------------Функции взаимодейтсвия с интерфейсом------
    void 	on_Btn_Serch_clicked();
    void 	checkCustomBaudRatePolicy(int idx);
    void 	on_button_search_clicked();
    void 	on_pushButton_writeMask_clicked();
    void 	on_pushButton_writeAdress_clicked();

    void 	receivedDataFromPort(const QByteArray &data);
    void 	about();

private:
    Ui::MainWindow *ui;

	//-----------------Основные запросы---------------------
    void 	requestReadMask(uchar adress);
    void 	requestReadDeviceNumber(uchar adress);
    void 	requestWriteRegisterAdrRz1();
    void 	requestWriteAdress(ushort adress);
    void 	requestWriteMask(ushort number);
	
	//-----------------Управляющие флаги--------------------
    StatusOfRequest 				m_currentStatusOfRequest = None;
    PorposeOfWrittingRegisterAdrRz1 m_writtingRegisterAdrRz1 = DontUseAdrRz1;
    AvalibleInfoInGui 				m_takeInfoFromGui = DontUseInfo;

	//-----------------Переменные для общения ПК с прибором-
    QTimer  *m_timerWaitForReply = nullptr;
    uchar    m_currentAdress = 0;
    Port    *m_portCommunication = nullptr;

	//-----------------Дополнительные функции---------------
    QString  converterDecToHex(ushort data);
    ushort 	 takeInfoFromGui(AvalibleInfoInGui currentInfoNeedToTake);
};

#endif // MAINWINDOW_H
