#include "mainwindow.h"
#include "hexeditor/qhexedit.h"
void run_script_init(MainWindow* mainwindow);
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setWindowIcon(QIcon(":res/123.png"));
    mdi = new QMdiArea(this);
    this->setCentralWidget(mdi);
    QDockWidget* dock = new QDockWidget(tr("Script log"),this);
    dock->setObjectName(tr("ScriptLogDock"));
    logText = new QTextEdit(dock);
    logText->setReadOnly(true);
    logText->setLineWrapMode(QTextEdit::NoWrap);
    dock->setWidget(logText);
    this->addDockWidget(Qt::BottomDockWidgetArea, dock);

    run_script_init(this);

    // make sure the about menu is shown
    QMenu* menu = 0;
    QList<QAction*> list = menuBar()->actions();
    foreach(QAction* act, list){
        if( act->text().contains(tr("help"),Qt::CaseInsensitive) ){
            menu = act->menu();
        }
    }

    if(menu == 0){
        menu = menuBar()->addMenu(tr("&Help"));
    }
    menu->addAction(dock->toggleViewAction());
    menu->addSeparator();
    QAction* act = menu->addAction(tr("&About..."));
    connect(act,SIGNAL(triggered()),this,SLOT(my_about()));
    this->setWindowTitle(tr("X Toolbox (Beta)"));
}

MainWindow::~MainWindow()
{

}

void MainWindow::addLog(const QString& log)
{
    logText->append(log);
}

void MainWindow::my_about()
{
    QMessageBox::about(this,
        tr("X Toolbox (Beta 0.0.6 20190730)"),
        tr("<p><b>X Toolbox(Beta)</b></p><p><a href=\"http://xtoolbox.org\">xtoolbox.org</a></p><a href=\"mailto:admin@xtoolbox.org\">admin@xtoolbox.org</a>"));
}
