#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qlibusb.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_dev = 0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnEnumDevice_clicked()
{
    bool ok;
    int vid = ui->editVID->text().toInt(&ok, 16);
    if(!ok)vid = 0;
    int pid = ui->editPID->text().toInt(&ok, 16);
    if(!ok)pid = 0;
    QList<QLibUsbInfo> infos = QLibUsb::enumDevices(vid, pid);
    ui->editDeviceList->clear();
    m_devs.clear();
    foreach(const QLibUsbInfo& i, infos){
        ui->editDeviceList->appendPlainText(i.toString());
        m_devs.append(i);
    }
}

void MainWindow::on_btnOpen_clicked()
{
    if(m_devs.size()>0){
        if(m_dev)return;
        m_dev = new QLibUsb(this);
        if(m_dev->open(m_devs.at(0))){
            ui->editDeviceList->appendPlainText(QString("Open success"));
            ui->editDeviceList->appendPlainText(QString("EndPoint for read"));
            foreach(const QLibUsbEPInfo& ep, m_dev->readEndpoints()){
                ui->editDeviceList->appendPlainText(ep.toString());
            }
            ui->editDeviceList->appendPlainText(QString("EndPoint for write"));
            foreach(const QLibUsbEPInfo& ep, m_dev->writeEndpoints()){
                ui->editDeviceList->appendPlainText(ep.toString());
            }
            connect(m_dev, SIGNAL(epDataReady(int,QByteArray)), this, SLOT(onData(int,QByteArray)));
        }else{
            ui->editDeviceList->appendPlainText(QString("Open fail"));
        }
    }
}

void MainWindow::on_btnSend_clicked()
{
    if(!m_dev)return;
    QByteArray data;
    bool ok;
    int len = ui->editLength->text().toInt(&ok, 10);
    if(!ok) len = 12;
    data.prepend(len);
    for(int i=1;i<len; i++){
        data.append(char(i));
    }
    const QList<QLibUsbEPInfo>& weps = m_dev->writeEndpoints();
    if(weps.size()>0){
        int r = m_dev->write(weps.at(0).bEndpointAddress, data, false);
        if(r >= 0){
            qDebug()<<"Write "<<data.size()<<" bytes data success, actual write " << r << "bytes";
        }else{
            qDebug()<<"Write "<<data.size()<<" bytes data fail, reason:"<<r;
        }
    }
}

void MainWindow::on_btnClose_clicked()
{
    if(m_dev){
        m_dev->close();
        delete m_dev;
        m_dev = 0;
    }
}

void MainWindow::onData(int ep, const QByteArray& data)
{
    qDebug()<<"Ep"<<ep<<" Got "<<data.size()<<" bytes data";
}
