#include <QApplication>
#include <QDialog>
#include "base_process.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    base_process baseprocessdialog;
    //启动时窗口居中
    baseprocessdialog.move((a.desktop()->width() - baseprocessdialog.width()) / 2,
                           (a.desktop()->height() - baseprocessdialog.height()) / 2);
    baseprocessdialog.setWindowTitle("便捷式OpenCV图像处理工具");
    baseprocessdialog.show();
    return a.exec();
}
