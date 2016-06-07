#ifndef PORT_H
#define PORT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

//-----------------Настройки порта---------
struct Settings {
    QString name;
    qint32 baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;
    QSerialPort::FlowControl flowControl;
};

class Port : public QObject
{
    Q_OBJECT

public:
    explicit Port(QObject *parent = 0);
    ~Port();

signals:
    void 	   sendReceivedData (const QByteArray &data);

public slots:

    void 	   readInPort();
    void 	   writeSettingsPort(QString, int, int, int, int, int);
    void 	   writeToPort(QByteArray send_modb);
    void 	   writeToPortString(QString send_modb);
    void 	   connectPort(QString name, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, 
			       QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl);

private:

    QSerialPort   *m_serialPort;
    Settings 	   m_settingsPort;
    void 	   convHex(QByteArray *data, QString str);
    unsigned short calcCrcModbus (const QByteArray &data);
    bool 	   isReceivedDataCorrect(QByteArray data);
};

#endif // PORT_H
