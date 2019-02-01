#include <QApplication>
#include <QDialog>
#include "base_process.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    base_process* baseprocessdialog = new base_process;
    baseprocessdialog->setFixedWidth(912);
//    baseprocessdialog->setMinimumSize(912,700);
//    baseprocessdialog->setMaximumSize(912,700);
    baseprocessdialog->setWindowTitle("便捷式OpenCV图像处理工具");
    baseprocessdialog->show();
    return a.exec();
}
