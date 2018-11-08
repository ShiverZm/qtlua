#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QLibUsb.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnEnumDevice_clicked();

    void on_btnOpen_clicked();

    void on_btnSend_clicked();

    void on_btnClose_clicked();

    void onData(int ep, const QByteArray& data);

private:
    Ui::MainWindow *ui;
    QList<QLibUsbInfo> m_devs;
    QLibUsb* m_dev;
};

#endif // MAINWINDOW_H
