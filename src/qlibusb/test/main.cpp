#include "mainwindow.h"
#include <QApplication>
#include "qlibusb.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    qDebug()<<"Started";
    QLibUsb::monitor(0x0483, 0x3748);

    /*
    QList<QLibUsbInfo> infos = QLibUsb::enumDevices(0x0483, 0x3748);
    foreach(QLibUsbInfo i, infos){
        qDebug()<<"Device:";
        qDebug()<<i.toString();
        QLibUsb usb;
        if(usb.open(i)){
            qDebug()<<"Interfaces";
            qDebug()<<usb.interfaceInfo();
            qDebug()<<"Open eps:";
            foreach(QLibUsbEPInfo i, usb.readEndpoints()){
                qDebug()<<i.toString();
            }
            usb.close();
        }
    }
    */
    return a.exec();
}
