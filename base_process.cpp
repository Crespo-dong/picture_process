#include "base_process.h"
#include "ui_base_process.h"
#include "time.h"

base_process::base_process(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::base_process)
{
    ui->setupUi(this);
    //设置边框颜色
//    ui->labelsrc->setStyleSheet("border:2px solid red;");
//    ui->labeldst->setStyleSheet("border:2px solid red;");

    //当前机器的屏幕的尺寸
    QRect desk_rect = QApplication::desktop()->availableGeometry();
    //根据屏幕尺寸调节窗口和控件的尺寸、位置
    setFixedSize(int(desk_rect.width() * 0.83), int(desk_rect.height() * 0.9));
    ui->labelsrc->setGeometry(int(width() * 0.01),
                              15,
                              int(desk_rect.width() * 0.4),
                              int(desk_rect.height() * 0.5));
    ui->labeldst->setGeometry(int(width() * 0.51),
                              15,
                              int(desk_rect.width() * 0.4),
                              int(desk_rect.height() * 0.5));
    ui->labelsrctxt->setGeometry(QRect(int(width() * 0.01),
                                            int(desk_rect.height() * 0.51),
                                            int(desk_rect.width() * 0.4),
                                            int(desk_rect.height() * 0.05)));
    ui->labelsrctxt_2->setGeometry(QRect(int(width() * 0.51),
                                            int(desk_rect.height() * 0.51),
                                            int(desk_rect.width() * 0.4),
                                            int(desk_rect.height() * 0.05)));
    ui->is_single_process->setGeometry(QRect(int(width() * 0.9),
                                             int(desk_rect.height() * 0.53),
                                             int(desk_rect.width() * 0.1),
                                             int(desk_rect.height() * 0.05)));
    ui->is_single_process->raise();//确保此控件不被其他控件覆盖
    ui->widget_for_layout->setGeometry(QRect(int(width() * 0.01),
                                             int(desk_rect.height() * 0.46),
                                             int(desk_rect.width() * 0.82),
                                             int(desk_rect.height() * 0.45)));

    setAcceptDrops(true);//窗口可接收拖放事件
    widget_layout = new QGridLayout(ui->widget_for_layout);
}

base_process::~base_process()
{
    delete ui;
}

//打开图片
void base_process::on_action_openpic_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开图片文件"),
                                                    openfilepath.isNull() ? "/" : openfilepath, //QDir::currentPath(),//默认路径，静态方法调用，使用程序运行的当前路径
                                                    tr("Image File(*.bmp *.jpg *.jpeg *.png)"));

    qDebug()<<tr("源图像路径：")<<fileName;
    if(fileName.isEmpty())//比如打开选择对话框了随即点取消 fileName为空
    {
        return;
    }
    close_other_obj();

    QFile *file = new QFile;
    file->setFileName(fileName);
    QFileInfo filetemp = QFileInfo(fileName);
    imgfilename = filetemp.baseName();
    openfilepath = filetemp.path();
    savefilepath.clear();

    QTextCodec *code = QTextCodec::codecForName("gb18030");//确保中文不乱码
    std::string name = code->fromUnicode(fileName).data();
    //读取文件
    srcImage = cv::imread(name);
    srcImage.copyTo(srcImage_copy);

    //将BGR转换为RGB，方便操作习惯
    cv::cvtColor(srcImage,srcImage,CV_BGR2RGB);
    img = QImage((const unsigned char*)(srcImage.data),
                 srcImage.cols,
                 srcImage.rows,
                 srcImage.cols*srcImage.channels(),
                 QImage::Format_RGB888);
    ui->labelsrc->clear();
    ui->labeldst->clear();
    img = img.scaled(ui->labelsrc->width(), ui->labelsrc->height());
    ui->labelsrc->setPixmap(QPixmap::fromImage(img));
    qDebug()<<"srcImage type "<<srcImage.type();
    save_num = 1;
}

//把处理后的图像dstImage放到labeldst标签显示
void base_process::dstlabel_show(Mat dstImage_temp)
{
    img = QImage((const unsigned char*)(dstImage_temp.data),
                 dstImage_temp.cols,
                 dstImage_temp.rows,
                 dstImage_temp.cols * dstImage_temp.channels(),
                 QImage::Format_RGB888);
    img = img.scaled(ui->labeldst->width(), ui->labeldst->height());
    ui->labeldst->setPixmap(QPixmap::fromImage(img));
}

//标签上显示二值图
void base_process::dstlabel_indexed8_show(Mat dstImage_temp)
{
    //图片格式 Format_Indexed8
    Mat dsttemp;
    dstImage_temp.copyTo(dsttemp);//二值图只有改了大小才能显示 中间变量用于输出效果 转为标签的大小
    cv::resize(dsttemp,dsttemp,Size(ui->labeldst->width(), ui->labeldst->height()),0,0,3);

    img = QImage((const unsigned char*)(dsttemp.data),
                 dsttemp.cols,
                 dsttemp.rows,
                 dsttemp.cols * dsttemp.channels(),
                 QImage::Format_Indexed8);
    img = img.scaled(ui->labeldst->width(), ui->labeldst->height());
    ui->labeldst->setPixmap(QPixmap::fromImage(img));
}

//保存效果图
void base_process::on_action_savedst_triggered()
{
    if (srcImage.empty()||dstImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开图片，并在处理后保存");
        return;
    }

    QString saveFileName = QFileDialog::getSaveFileName(this,
                                                        tr("保存文件"),
                                                        QString(savefilepath.isNull() ? "/" : savefilepath) + "/" + imgfilename + tr("-效果图-%1.png").arg(save_num),
                                                        tr("image(*.png);;image(*.jpg);;image(*.jpeg);;image(*.bmp)"));
    if(saveFileName.isEmpty())//比如打开选择对话框了随即点取消 saveFileName为空
    {
        return;
    }
    QTextCodec *code = QTextCodec::codecForName("gb18030");//确保保存中文名不乱码
    std::string fileAsSave = code->fromUnicode(saveFileName).data();

    QFileInfo filetemp = QFileInfo(saveFileName);
    savefilepath = filetemp.path();
    //string fileAsSave = saveFileName.toStdString();
    //格式不同 保存方法不同
    if(dstImage.type() == CV_8UC3)//三通道图
    {
        cv::cvtColor(dstImage,dstImage,CV_BGR2RGB);//保存所见的的/保存实际的
        imwrite(fileAsSave,dstImage);
    }
    else if(dstImage.type() == CV_8UC1)//二值化图
    {
        std::vector<int> compression_params;
        compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
        imwrite(fileAsSave,dstImage,compression_params);
    }
    save_num++;
}

//清除图像
void base_process::on_action_clear_triggered()
{
    ui->labelsrc->clear();
    ui->labeldst->clear();
    srcImage.release();
    dstImage.release();
    close_other_obj();
}

void base_process::on_action_exit_triggered()
{
    exit(0);//关闭程序
    //this->close();//关闭当前窗口
}

//还原图像
void base_process::on_action_huanyuan_triggered()
{
    if (srcImage_copy.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }

    srcImage_copy.copyTo(srcImage);
    cv::cvtColor(srcImage,srcImage,CV_BGR2RGB);
    dstImage.release();
    img = QImage((const unsigned char*)(srcImage.data),
                 srcImage.cols,
                 srcImage.rows,
                 srcImage.cols*srcImage.channels(),
                 QImage::Format_RGB888);
    ui->labelsrc->clear();
    ui->labeldst->clear();
    img = img.scaled(ui->labelsrc->width(), ui->labelsrc->height());
    ui->labelsrc->setPixmap(QPixmap::fromImage(img));
}

//形态学处理
void base_process::on_action_select_scope_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    element_show_01 = new QLabel(tr("形态学处理--操作核类型："),ui->widget_for_layout);

    element_type = new QComboBox(ui->widget_for_layout);
    element_type->addItem(tr("矩形"),cv::MORPH_RECT);
    element_type->addItem(tr("椭圆形"),cv::MORPH_ELLIPSE);
    element_type->addItem(tr("十字形"),cv::MORPH_CROSS);
    connect(element_type, SIGNAL(activated(const QString &)),this, SLOT(domorphfun()));

    element_show_02 = new QLabel(tr("操作核尺寸："),ui->widget_for_layout);

    element_size_w = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        element_size_w->addItem(tr("核宽：%1").arg(i),i);
    }

    connect(element_size_w, SIGNAL(activated(const QString &)),this, SLOT(domorphfun()));

    element_size_h = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        element_size_h->addItem(tr("核高：%1").arg(i),i);
    }
    connect(element_size_h, SIGNAL(activated(const QString &)),this, SLOT(domorphfun()));

    element_show_03 = new QLabel(tr("形态学操作类型："),ui->widget_for_layout);

    morphfun = new QComboBox(ui->widget_for_layout);
    morphfun->addItem(tr("腐蚀"),cv::MORPH_ERODE);
    morphfun->addItem(tr("膨胀"),cv::MORPH_DILATE);
    morphfun->addItem(tr("开运算"),cv::MORPH_OPEN);
    morphfun->addItem(tr("闭运算"),cv::MORPH_CLOSE);
    morphfun->addItem(tr("形态学梯度"),cv::MORPH_GRADIENT);
    morphfun->addItem(tr("顶帽"),cv::MORPH_TOPHAT);
    morphfun->addItem(tr("黑帽"),cv::MORPH_BLACKHAT);
    connect(morphfun, SIGNAL(activated(const QString &)),this, SLOT(domorphfun()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(element_show_01,0,0);
    widget_layout->addWidget(element_type,0,1);
    widget_layout->addWidget(element_show_02,0,2);
    widget_layout->addWidget(element_size_w,0,3);
    widget_layout->addWidget(element_size_h,0,4);
    widget_layout->addWidget(element_show_03,0,5);
    widget_layout->addWidget(morphfun,0,6);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    domorphfun();
}

//形态学处理操作函数
void base_process::domorphfun()
{
    Mat element = getStructuringElement(element_type->currentData(Qt::UserRole).toInt(),
                                        Size(element_size_w->currentData(Qt::UserRole).toInt(),element_size_h->currentData(Qt::UserRole).toInt()),
                                        Point(-1,-1));
    morphologyEx(srcImage,dstImage,morphfun->currentData(Qt::UserRole).toInt(),element);
    dstlabel_show(dstImage);
}

//均值滤波
void base_process::on_action_bulr_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    blursize_w = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        blursize_w->addItem(tr("核宽：%1").arg(i),i);
    }
    connect(blursize_w, SIGNAL(activated(const QString &)),this, SLOT(blur_process_pic()));

    blursize_h = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        blursize_h->addItem(tr("核高：%1").arg(i),i);
    }
    connect(blursize_h, SIGNAL(activated(const QString &)),this, SLOT(blur_process_pic()));

    blur_show = new QLabel(tr("均值滤波--操作核尺寸："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(blur_show,0,0);
    widget_layout->addWidget(blursize_w,0,1);
    widget_layout->addWidget(blursize_h,0,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    blur_process_pic();
}

//使用均值滤波处理图像
void base_process::blur_process_pic()
{    
    blur(srcImage,dstImage,Size(blursize_w->currentData(Qt::UserRole).toInt(),blursize_h->currentData(Qt::UserRole).toInt()),Point(-1,-1));
    dstlabel_show(dstImage);
}

//方框滤波
void base_process::on_action_boxFilter_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    boxFiltersize_w = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        boxFiltersize_w->addItem(tr("核宽：%1").arg(i),i);
    }
    connect(boxFiltersize_w, SIGNAL(activated(const QString &)),this, SLOT(boxFilter_process_pic()));

    boxFiltersize_h = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        boxFiltersize_h->addItem(tr("核高：%1").arg(i),i);
    }
    connect(boxFiltersize_h, SIGNAL(activated(const QString &)),this, SLOT(boxFilter_process_pic()));

    isNormalize = new QCheckBox(ui->widget_for_layout);
    isNormalize->setText(tr("进行归一化处理"));
    isNormalize->setChecked(true);//选中
    connect(isNormalize, SIGNAL(stateChanged(int)), this, SLOT(boxFilter_process_pic()));

    boxFilter_show = new QLabel(tr("方框滤波--操作核尺寸："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(boxFilter_show,0,0);
    widget_layout->addWidget(boxFiltersize_w,0,1);
    widget_layout->addWidget(boxFiltersize_h,0,2);
    widget_layout->addWidget(isNormalize,0,3);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    boxFilter_process_pic();
}

//使用方框滤波处理图像
void base_process::boxFilter_process_pic()
{
    bool bNormalize = false;
    if(isNormalize->checkState() == Qt::Checked)//选中进行归一化处理
    {
        bNormalize = true;
    }
    else
    {
        bNormalize = false;
    }

    boxFilter(srcImage,
              dstImage,
              -1,
              Size(boxFiltersize_w->currentData(Qt::UserRole).toInt(),boxFiltersize_h->currentData(Qt::UserRole).toInt()),
              Point(-1,-1),
              bNormalize);
    dstlabel_show(dstImage);
}

//高斯滤波
void base_process::on_action_GaussFilter_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    gausssizewith = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        gausssizewith->addItem(tr("核宽：%1").arg(i),i);
    }
    connect(gausssizewith, SIGNAL(activated(const QString &)),this, SLOT(gussFilter_process_pic()));

    gusssizehight = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        gusssizehight->addItem(tr("核高：%1").arg(i),i);
    }
    connect(gusssizehight, SIGNAL(activated(const QString &)),this, SLOT(gussFilter_process_pic()));

    gauss_show = new QLabel(tr("高斯滤波--操作核尺寸："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(gauss_show,0,0);
    widget_layout->addWidget(gausssizewith,0,1);
    widget_layout->addWidget(gusssizehight,0,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    gussFilter_process_pic();
}

//高斯滤波处理图像
void base_process::gussFilter_process_pic()
{
    GaussianBlur(srcImage,dstImage,Size(gausssizewith->currentData(Qt::UserRole).toInt(),gusssizehight->currentData(Qt::UserRole).toInt()),0,0);
    dstlabel_show(dstImage);
}

//中值滤波
void base_process::on_action_midian_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    mediansize = new QComboBox(ui->widget_for_layout);
    for(int i = 3;i <= 20;i += 2)
    {
        mediansize->addItem(tr("%1 × %1").arg(i),i);
    }
    connect(mediansize, SIGNAL(activated(const QString &)),this, SLOT(median_process_pic()));

    median_show = new QLabel(tr("中值滤波--操作核尺寸："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(median_show,0,0);
    widget_layout->addWidget(mediansize,0,1);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    median_process_pic();
}

//中值滤波处理
void base_process::median_process_pic()
{
    medianBlur(srcImage,dstImage,mediansize->currentData(Qt::UserRole).toInt());
    dstlabel_show(dstImage);
}

//双边滤波
void base_process::on_action_bilateral_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    colorsigma_SpinBox = new QSpinBox(ui->widget_for_layout);
    colorsigma_SpinBox->setMinimum(1);
    colorsigma_SpinBox->setMaximum(254);
    colorsigma_SpinBox->setValue(1);//设置一个默认值

    colorsigma_slider = new QSlider(ui->widget_for_layout);
    colorsigma_slider->setOrientation(Qt::Horizontal);//水平方向
    colorsigma_slider->setMinimum(1);
    colorsigma_slider->setMaximum(254);
    colorsigma_slider->setSingleStep(10);//步长 动一下移动的距离
    colorsigma_slider->setTickInterval(15); // 设置刻度间隔
    colorsigma_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    colorsigma_slider->setValue(1);//设置一个默认值

    connect(colorsigma_SpinBox, SIGNAL(valueChanged(int)), colorsigma_slider, SLOT(setValue(int)));
    connect(colorsigma_slider, SIGNAL(valueChanged(int)), colorsigma_SpinBox, SLOT(setValue(int)));

    spacesigma_SpinBox = new QSpinBox(ui->widget_for_layout);
    spacesigma_SpinBox->setMinimum(3);
    spacesigma_SpinBox->setMaximum(200);
    spacesigma_SpinBox->setValue(3);//设置一个默认值

    spacesigma_slider = new QSlider(ui->widget_for_layout);
    spacesigma_slider->setOrientation(Qt::Horizontal);//水平方向
    spacesigma_slider->setMinimum(3);
    spacesigma_slider->setMaximum(200);
    spacesigma_slider->setSingleStep(10);//步长 动一下移动的距离
    spacesigma_slider->setTickInterval(15); // 设置刻度间隔
    spacesigma_slider->setTickPosition(QSlider::TicksAbove);//刻度在上方
    spacesigma_slider->setValue(3);//设置一个默认值

    connect(spacesigma_SpinBox, SIGNAL(valueChanged(int)), spacesigma_slider, SLOT(setValue(int)));
    connect(spacesigma_slider, SIGNAL(valueChanged(int)), spacesigma_SpinBox, SLOT(setValue(int)));

    showinputcolorsigma = new QLabel(tr("颜色范围标准差，范围：1~254"),ui->widget_for_layout);
    showinputspacesigma = new QLabel(tr("空间距离标准差，范围：3~200"),ui->widget_for_layout);
    showtaking = new QLabel(tr("注意：空间距离标准差值较大时会很耗时"),ui->widget_for_layout);

    is_chose_bilateral = new QPushButton("确定",ui->widget_for_layout);
    connect(is_chose_bilateral, SIGNAL(clicked()), this, SLOT(bilateral_process_pic()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(showinputcolorsigma,0,0);
    widget_layout->addWidget(colorsigma_SpinBox,0,1);
    widget_layout->addWidget(colorsigma_slider,0,2);
    widget_layout->addWidget(showtaking,0,3);
    widget_layout->addWidget(showinputspacesigma,1,0);
    widget_layout->addWidget(spacesigma_SpinBox,1,1);
    widget_layout->addWidget(spacesigma_slider,1,2);
    widget_layout->addWidget(is_chose_bilateral,1,3);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    bilateral_process_pic();
}

//双边滤波处理
void base_process::bilateral_process_pic()
{    
    bilateralFilter(srcImage,dstImage,-1,colorsigma_SpinBox->text().toInt(),spacesigma_SpinBox->text().toInt());
    dstlabel_show(dstImage);
}

//直方图均衡化
void base_process::on_action_equalizehist_triggered()
{    
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    vector<Mat> splitBGR(srcImage.channels());
    split(srcImage,splitBGR);
    for(int i = 0;i < srcImage.channels();i++)
    {
        equalizeHist(splitBGR[i],splitBGR[i]);
    }
    Mat mergeImg;
    merge(splitBGR,mergeImg);
    mergeImg.copyTo(dstImage);
    dstlabel_show(dstImage);
}

//调节亮度、对比度
void base_process::on_action_tiaojie_liangdu_duibidu_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    //调节亮度
    liangdu_SpinBox = new QSpinBox(ui->widget_for_layout);
    liangdu_SpinBox->setMinimum(-200);
    liangdu_SpinBox->setMaximum(200);
    liangdu_SpinBox->setValue(0);//设置一个默认值

    liangdu_slider = new QSlider(ui->widget_for_layout);
    liangdu_slider->setOrientation(Qt::Horizontal);//水平方向
    liangdu_slider->setMinimum(-200);
    liangdu_slider->setMaximum(200);
    liangdu_slider->setSingleStep(10);//步长 动一下移动的距离
    liangdu_slider->setTickInterval(10); // 设置刻度间隔
    liangdu_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    liangdu_slider->setValue(0);//设置一个默认值

    connect(liangdu_slider, SIGNAL(valueChanged(int)), liangdu_SpinBox, SLOT(setValue(int)));
    connect(liangdu_SpinBox, SIGNAL(valueChanged(int)), liangdu_slider, SLOT(setValue(int)));

    connect(liangdu_slider, SIGNAL(valueChanged(int)), this, SLOT(change_liangdu_duibidu()));
    connect(liangdu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(change_liangdu_duibidu()));

    //调节对比度
    duibidu_LineEdit = new QLineEdit(ui->widget_for_layout);
    duibidu_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    duibidu_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    duibidu_slider = new QSlider(ui->widget_for_layout);
    duibidu_slider->setOrientation(Qt::Horizontal);//水平方向
    duibidu_slider->setMinimum(1);
    duibidu_slider->setMaximum(1000);
    duibidu_slider->setSingleStep(5);//步长 动一下移动的距离
    duibidu_slider->setTickInterval(40); // 设置刻度间隔
    duibidu_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    duibidu_slider->setValue(100);//设置一个默认值
    connect(duibidu_slider, SIGNAL(valueChanged(int)), this, SLOT(change_liangdu_duibidu()));

    show_liangdu =  new QLabel(tr("设置亮度："),ui->widget_for_layout);
    show_duibidu =  new QLabel(tr("设置对比度："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(show_liangdu,0,0);
    widget_layout->addWidget(liangdu_SpinBox,0,1);
    widget_layout->addWidget(liangdu_slider,0,2);
    widget_layout->addWidget(show_duibidu,1,0);
    widget_layout->addWidget(duibidu_LineEdit,1,1);
    widget_layout->addWidget(duibidu_slider,1,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    change_liangdu_duibidu();
}

//调节亮度 对比度函数
void base_process::change_liangdu_duibidu()
{
    double temp = double(duibidu_slider->value())/100;
    duibidu_LineEdit->setText(QString::number(temp,10,2));

    srcImage.convertTo(dstImage,-1,temp,liangdu_slider->value());
    dstlabel_show(dstImage);
}

//仿射变换
void base_process::on_action_warpAffine_triggered()
{
    if (srcImage.empty())
    {
       QMessageBox::information(this,"提示","请先打开一张图片");
       return;
    }
    close_other_obj();

    warpAffine_center_x_SpinBox = new QSpinBox(ui->widget_for_layout);
    warpAffine_center_x_SpinBox->setMinimum(0);
    warpAffine_center_x_SpinBox->setMaximum(99);
    warpAffine_center_x_SpinBox->setValue(50);//设置一个默认值
    warpAffine_center_x_SpinBox->setPrefix("0.");//前缀

    warpAffine_center_x_slider = new QSlider(ui->widget_for_layout);
    warpAffine_center_x_slider->setOrientation(Qt::Horizontal);//水平方向
    warpAffine_center_x_slider->setMinimum(0);
    warpAffine_center_x_slider->setMaximum(99);
    warpAffine_center_x_slider->setSingleStep(10);//步长 动一下移动的距离
    warpAffine_center_x_slider->setTickInterval(5); // 设置刻度间隔
    warpAffine_center_x_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    warpAffine_center_x_slider->setValue(50);//设置一个默认值

    connect(warpAffine_center_x_slider, SIGNAL(valueChanged(int)), warpAffine_center_x_SpinBox, SLOT(setValue(int)));
    connect(warpAffine_center_x_SpinBox, SIGNAL(valueChanged(int)), warpAffine_center_x_slider, SLOT(setValue(int)));

    connect(warpAffine_center_x_slider, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));
    connect(warpAffine_center_x_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));

    warpAffine_center_y_SpinBox = new QSpinBox(ui->widget_for_layout);
    warpAffine_center_y_SpinBox->setMinimum(0);
    warpAffine_center_y_SpinBox->setMaximum(99);
    warpAffine_center_y_SpinBox->setValue(50);//设置一个默认值
    warpAffine_center_y_SpinBox->setPrefix("0.");//前缀

    warpAffine_center_y_slider = new QSlider(ui->widget_for_layout);
    warpAffine_center_y_slider->setOrientation(Qt::Horizontal);//水平方向
    warpAffine_center_y_slider->setMinimum(0);
    warpAffine_center_y_slider->setMaximum(99);
    warpAffine_center_y_slider->setSingleStep(10);//步长 动一下移动的距离
    warpAffine_center_y_slider->setTickInterval(5); // 设置刻度间隔
    warpAffine_center_y_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    warpAffine_center_y_slider->setValue(50);//设置一个默认值

    connect(warpAffine_center_y_slider, SIGNAL(valueChanged(int)), warpAffine_center_y_SpinBox, SLOT(setValue(int)));
    connect(warpAffine_center_y_SpinBox, SIGNAL(valueChanged(int)), warpAffine_center_y_slider, SLOT(setValue(int)));

    connect(warpAffine_center_y_slider, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));
    connect(warpAffine_center_y_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));

    warpAffine_angle_SpinBox = new QSpinBox(ui->widget_for_layout);
    warpAffine_angle_SpinBox->setMinimum(-180);
    warpAffine_angle_SpinBox->setMaximum(180);
    warpAffine_angle_SpinBox->setValue(0);//设置一个默认值

    warpAffine_angle_slider = new QSlider(ui->widget_for_layout);
    warpAffine_angle_slider->setOrientation(Qt::Horizontal);//水平方向
    warpAffine_angle_slider->setMinimum(-180);
    warpAffine_angle_slider->setMaximum(180);
    warpAffine_angle_slider->setSingleStep(10);//步长 动一下移动的距离
    warpAffine_angle_slider->setTickInterval(20); // 设置刻度间隔
    warpAffine_angle_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    warpAffine_angle_slider->setValue(0);//设置一个默认值

    connect(warpAffine_angle_slider, SIGNAL(valueChanged(int)), warpAffine_angle_SpinBox, SLOT(setValue(int)));
    connect(warpAffine_angle_SpinBox, SIGNAL(valueChanged(int)), warpAffine_angle_slider, SLOT(setValue(int)));

    connect(warpAffine_angle_slider, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));
    connect(warpAffine_angle_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));

    warpAffine_scale_LineEdit = new QLineEdit(ui->widget_for_layout);
    warpAffine_scale_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    warpAffine_scale_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    warpAffine_scale_LineEdit->setText("1.00");

    warpAffine_scale_Slider = new QSlider(ui->widget_for_layout);
    warpAffine_scale_Slider->setOrientation(Qt::Horizontal);//水平方向
    warpAffine_scale_Slider->setMinimum(1);
    warpAffine_scale_Slider->setMaximum(1000);
    warpAffine_scale_Slider->setSingleStep(50);//步长 动一下移动的距离
    warpAffine_scale_Slider->setTickInterval(50); // 设置刻度间隔
    warpAffine_scale_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    warpAffine_scale_Slider->setValue(100);//设置一个默认值
    connect(warpAffine_scale_Slider, SIGNAL(valueChanged(int)), this, SLOT(process_warpAffine()));

    warpAffine_show01 = new QLabel(tr("旋转中心--X轴："),ui->widget_for_layout);
    warpAffine_show02 = new QLabel(tr("旋转中心--Y轴："),ui->widget_for_layout);
    warpAffine_show03 = new QLabel(tr("旋转角度："),ui->widget_for_layout);
    warpAffine_show04 = new QLabel(tr("缩放比例："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(warpAffine_show01,0,0);
    widget_layout->addWidget(warpAffine_center_x_SpinBox,0,1);
    widget_layout->addWidget(warpAffine_center_x_slider,0,2);
    widget_layout->addWidget(warpAffine_show02,1,0);
    widget_layout->addWidget(warpAffine_center_y_SpinBox,1,1);
    widget_layout->addWidget(warpAffine_center_y_slider,1,2);
    widget_layout->addWidget(warpAffine_show03,2,0);
    widget_layout->addWidget(warpAffine_angle_SpinBox,2,1);
    widget_layout->addWidget(warpAffine_angle_slider,2,2);
    widget_layout->addWidget(warpAffine_show04,3,0);
    widget_layout->addWidget(warpAffine_scale_LineEdit,3,1);
    widget_layout->addWidget(warpAffine_scale_Slider,3,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    process_warpAffine();
}

//仿射变换处理
void base_process::process_warpAffine()
{
    if(warpAffine_center_x_slider->value() >=10)
    {
        warpAffine_center_x_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        warpAffine_center_x_SpinBox->setPrefix("0.0");//前缀
    }

    if(warpAffine_center_y_slider->value() >=10)
    {
        warpAffine_center_y_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        warpAffine_center_y_SpinBox->setPrefix("0.0");//前缀
    }

    double temp = double(warpAffine_scale_Slider->value())/100;
    warpAffine_scale_LineEdit->setText(QString::number(temp,10,2));

    Point2f center(int(srcImage.cols * warpAffine_center_x_SpinBox->text().toCaseFolded().toDouble()),
                   int(srcImage.rows * warpAffine_center_y_SpinBox->text().toCaseFolded().toDouble()));

    Mat M = getRotationMatrix2D(center,
                                warpAffine_angle_SpinBox->value(),
                                temp);//计算旋转加缩放的变换矩阵
    warpAffine(srcImage, dstImage, M, srcImage.size(),1,0,Scalar(rand()&255,rand()&255,rand()&255));
    dstlabel_show(dstImage);
}

//笛卡尔坐标与极坐标间转换
void base_process::on_action_logPolar_triggered()
{
    if (srcImage.empty())
    {
       QMessageBox::information(this,"提示","请先打开一张图片");
       return;
    }
    close_other_obj();

    logPolar_type = new QComboBox(ui->widget_for_layout);
    logPolar_type->addItem(tr("笛卡尔坐标 --> 极坐标"),1);
    logPolar_type->addItem(tr("极坐标 --> 笛卡尔坐标"),2);
    connect(logPolar_type, SIGNAL(activated(const QString &)),this, SLOT(process_logPolar()));

    logPolar_center_x_SpinBox = new QSpinBox(ui->widget_for_layout);
    logPolar_center_x_SpinBox->setMinimum(1);
    logPolar_center_x_SpinBox->setMaximum(99);
    logPolar_center_x_SpinBox->setValue(50);//设置一个默认值
    logPolar_center_x_SpinBox->setPrefix("0.");//前缀

    logPolar_center_x_slider = new QSlider(ui->widget_for_layout);
    logPolar_center_x_slider->setOrientation(Qt::Horizontal);//水平方向
    logPolar_center_x_slider->setMinimum(1);
    logPolar_center_x_slider->setMaximum(99);
    logPolar_center_x_slider->setSingleStep(10);//步长 动一下移动的距离
    logPolar_center_x_slider->setTickInterval(5); // 设置刻度间隔
    logPolar_center_x_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    logPolar_center_x_slider->setValue(50);//设置一个默认值

    connect(logPolar_center_x_slider, SIGNAL(valueChanged(int)), logPolar_center_x_SpinBox, SLOT(setValue(int)));
    connect(logPolar_center_x_SpinBox, SIGNAL(valueChanged(int)), logPolar_center_x_slider, SLOT(setValue(int)));

    connect(logPolar_center_x_slider, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));
    connect(logPolar_center_x_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));

    logPolar_center_y_SpinBox = new QSpinBox(ui->widget_for_layout);
    logPolar_center_y_SpinBox->setMinimum(1);
    logPolar_center_y_SpinBox->setMaximum(99);
    logPolar_center_y_SpinBox->setValue(50);//设置一个默认值
    logPolar_center_y_SpinBox->setPrefix("0.");//前缀

    logPolar_center_y_slider = new QSlider(ui->widget_for_layout);
    logPolar_center_y_slider->setOrientation(Qt::Horizontal);//水平方向
    logPolar_center_y_slider->setMinimum(1);
    logPolar_center_y_slider->setMaximum(99);
    logPolar_center_y_slider->setSingleStep(10);//步长 动一下移动的距离
    logPolar_center_y_slider->setTickInterval(5); // 设置刻度间隔
    logPolar_center_y_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    logPolar_center_y_slider->setValue(50);//设置一个默认值

    connect(logPolar_center_y_slider, SIGNAL(valueChanged(int)), logPolar_center_y_SpinBox, SLOT(setValue(int)));
    connect(logPolar_center_y_SpinBox, SIGNAL(valueChanged(int)), logPolar_center_y_slider, SLOT(setValue(int)));

    connect(logPolar_center_y_slider, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));
    connect(logPolar_center_y_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));

    logPolar_Magnitude_SpinBox = new QSpinBox(ui->widget_for_layout);
    logPolar_Magnitude_SpinBox->setMinimum(1);
    logPolar_Magnitude_SpinBox->setMaximum(800);
    logPolar_Magnitude_SpinBox->setValue(80);//设置一个默认值

    logPolar_Magnitude_slider = new QSlider(ui->widget_for_layout);
    logPolar_Magnitude_slider->setOrientation(Qt::Horizontal);//水平方向
    logPolar_Magnitude_slider->setMinimum(1);
    logPolar_Magnitude_slider->setMaximum(800);
    logPolar_Magnitude_slider->setSingleStep(10);//步长 动一下移动的距离
    logPolar_Magnitude_slider->setTickInterval(20); // 设置刻度间隔
    logPolar_Magnitude_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    logPolar_Magnitude_slider->setValue(80);//设置一个默认值

    connect(logPolar_Magnitude_slider, SIGNAL(valueChanged(int)), logPolar_Magnitude_SpinBox, SLOT(setValue(int)));
    connect(logPolar_Magnitude_SpinBox, SIGNAL(valueChanged(int)), logPolar_Magnitude_slider, SLOT(setValue(int)));

    connect(logPolar_Magnitude_slider, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));
    connect(logPolar_Magnitude_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(process_logPolar()));

    logPolar_show01 = new QLabel(tr("变换中心--X轴："),ui->widget_for_layout);
    logPolar_show02 = new QLabel(tr("变换中心--Y轴："),ui->widget_for_layout);
    logPolar_show03 = new QLabel(tr("变换幅度尺度参数："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(logPolar_type,0,0,1,2);
    widget_layout->addWidget(logPolar_show01,1,0);
    widget_layout->addWidget(logPolar_center_x_SpinBox,1,1);
    widget_layout->addWidget(logPolar_center_x_slider,1,2);
    widget_layout->addWidget(logPolar_show02,2,0);
    widget_layout->addWidget(logPolar_center_y_SpinBox,2,1);
    widget_layout->addWidget(logPolar_center_y_slider,2,2);
    widget_layout->addWidget(logPolar_show03,3,0);
    widget_layout->addWidget(logPolar_Magnitude_SpinBox,3,1);
    widget_layout->addWidget(logPolar_Magnitude_slider,3,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    process_logPolar();
}

//笛卡尔坐标与极坐标间转换处理
void base_process::process_logPolar()
{
    if(logPolar_center_x_slider->value() >=10)
    {
        logPolar_center_x_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        logPolar_center_x_SpinBox->setPrefix("0.0");//前缀
    }

    if(logPolar_center_y_slider->value() >=10)
    {
        logPolar_center_y_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        logPolar_center_y_SpinBox->setPrefix("0.0");//前缀
    }

    Point2f center(int(srcImage.cols * logPolar_center_x_SpinBox->text().toCaseFolded().toDouble()),
                   int(srcImage.rows * logPolar_center_y_SpinBox->text().toCaseFolded().toDouble()));

    int flags;
    if(logPolar_type->currentData(Qt::UserRole).toInt() == 1)
    {
        flags = WARP_FILL_OUTLIERS;
    }
    else
    {
        flags = WARP_INVERSE_MAP;
    }

    logPolar(srcImage,dstImage,center,logPolar_Magnitude_SpinBox->value(),flags);
    dstlabel_show(dstImage);
}

//线性变换
void base_process::on_action_kx_b_triggered()
{
    if (srcImage.empty())
    {
       QMessageBox::information(this,"提示","请先打开一张图片");
       return;
    }
    close_other_obj();

    kx_b_k_slider = new QSlider(ui->widget_for_layout);
    kx_b_k_slider->setOrientation(Qt::Horizontal);//水平方向
    kx_b_k_slider->setMinimum(-100);
    kx_b_k_slider->setMaximum(100);
    kx_b_k_slider->setSingleStep(10);//步长 动一下移动的距离
    kx_b_k_slider->setTickInterval(10); // 设置刻度间隔
    kx_b_k_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    kx_b_k_slider->setValue(10);//设置一个默认值
    connect(kx_b_k_slider, SIGNAL(valueChanged(int)), this, SLOT(kx_b()));

    kx_b_k_LineEdit = new QLineEdit(ui->widget_for_layout);
    kx_b_k_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    kx_b_k_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    kx_b_b_slider = new QSlider(ui->widget_for_layout);
    kx_b_b_slider->setOrientation(Qt::Horizontal);//水平方向
    kx_b_b_slider->setMinimum(-2000);
    kx_b_b_slider->setMaximum(2000);
    kx_b_b_slider->setSingleStep(100);//步长 动一下移动的距离
    kx_b_b_slider->setTickInterval(100); // 设置刻度间隔
    kx_b_b_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    kx_b_b_slider->setValue(100);//设置一个默认值
    connect(kx_b_b_slider, SIGNAL(valueChanged(int)), this, SLOT(kx_b()));

    kx_b_b_LineEdit = new QLineEdit(ui->widget_for_layout);
    kx_b_b_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    kx_b_b_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    kx_b_show01 = new QLabel(tr("缩放因数k："),ui->widget_for_layout);
    kx_b_show02 = new QLabel(tr("偏移量b："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(kx_b_show01,0,0);
    widget_layout->addWidget(kx_b_k_LineEdit,0,1);
    widget_layout->addWidget(kx_b_k_slider,0,2);
    widget_layout->addWidget(kx_b_show02,1,0);
    widget_layout->addWidget(kx_b_b_LineEdit,1,1);
    widget_layout->addWidget(kx_b_b_slider,1,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    kx_b();
}

//线性变换处理
void base_process::kx_b()
{
    double tempk = double(kx_b_k_slider->value())/10;
    kx_b_k_LineEdit->setText(QString::number(tempk,10,2));

    double tempb = double(kx_b_b_slider->value())/10;
    kx_b_b_LineEdit->setText(QString::number(tempb,10,2));

    convertScaleAbs(srcImage,dstImage,tempk,tempb);
    dstlabel_show(dstImage);
}

//基本二值化
void base_process::on_action_base_triggered()
{    
    if (srcImage.empty())
    {
       QMessageBox::information(this,"提示","请先打开一张图片");
       return;
    }
    close_other_obj();

    jiben_thresh_func = new QComboBox(ui->widget_for_layout);
    jiben_thresh_func->addItem(tr("THRESH_BINARY"),THRESH_BINARY);
    jiben_thresh_func->addItem(tr("THRESH_BINARY_INV"),THRESH_BINARY_INV);
    jiben_thresh_func->addItem(tr("THRESH_TRUNC"),THRESH_TRUNC);
    jiben_thresh_func->addItem(tr("THRESH_TOZERO"),THRESH_TOZERO);
    jiben_thresh_func->addItem(tr("THRESH_TOZERO_INV"),THRESH_TOZERO_INV);
    connect(jiben_thresh_func, SIGNAL(activated(const QString &)),this, SLOT(jiben_thresh_process()));

    jibenthresh_SpinBox = new QSpinBox(ui->widget_for_layout);
    jibenthresh_SpinBox->setMinimum(1);
    jibenthresh_SpinBox->setMaximum(255);
    jibenthresh_SpinBox->setValue(1);//设置一个默认值

    jibenthresh_slider = new QSlider(ui->widget_for_layout);
    jibenthresh_slider->setOrientation(Qt::Horizontal);//水平方向
    jibenthresh_slider->setMinimum(1);
    jibenthresh_slider->setMaximum(255);
    jibenthresh_slider->setSingleStep(10);//步长 动一下移动的距离
    jibenthresh_slider->setTickInterval(10); // 设置刻度间隔
    jibenthresh_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    jibenthresh_slider->setValue(1);//设置一个默认值

    connect(jibenthresh_slider, SIGNAL(valueChanged(int)), jibenthresh_SpinBox, SLOT(setValue(int)));
    connect(jibenthresh_SpinBox, SIGNAL(valueChanged(int)), jibenthresh_slider, SLOT(setValue(int)));

    connect(jibenthresh_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_thresh_process()));
    connect(jibenthresh_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_thresh_process()));

    show_input_thresh_value = new QLabel(tr("     设置当前的阈值："),ui->widget_for_layout);
    show_max_input_thresh_value = new QLabel(tr("THRESH_BINARY | THRESH_BINARY_INV的最大值："),ui->widget_for_layout);

    jibenthresh_max_SpinBox = new QSpinBox(ui->widget_for_layout);
    jibenthresh_max_SpinBox->setMinimum(1);
    jibenthresh_max_SpinBox->setMaximum(255);
    jibenthresh_max_SpinBox->setValue(255);//设置一个默认值

    jibenthresh_max_slider = new QSlider(ui->widget_for_layout);
    jibenthresh_max_slider->setOrientation(Qt::Horizontal);//水平方向
    jibenthresh_max_slider->setMinimum(1);
    jibenthresh_max_slider->setMaximum(255);
    jibenthresh_max_slider->setSingleStep(10);//步长 动一下移动的距离
    jibenthresh_max_slider->setTickInterval(10); // 设置刻度间隔
    jibenthresh_max_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    jibenthresh_max_slider->setValue(255);//设置一个默认值

    connect(jibenthresh_max_slider, SIGNAL(valueChanged(int)), jibenthresh_max_SpinBox, SLOT(setValue(int)));
    connect(jibenthresh_max_SpinBox, SIGNAL(valueChanged(int)), jibenthresh_max_slider, SLOT(setValue(int)));

    connect(jibenthresh_max_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_thresh_process()));
    connect(jibenthresh_max_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_thresh_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(jiben_thresh_func,0,0);
    widget_layout->addWidget(show_input_thresh_value,0,1);
    widget_layout->addWidget(jibenthresh_SpinBox,0,2);
    widget_layout->addWidget(jibenthresh_slider,0,3);
    widget_layout->addWidget(show_max_input_thresh_value,1,0,1,2);
    widget_layout->addWidget(jibenthresh_max_SpinBox,1,2);
    widget_layout->addWidget(jibenthresh_max_slider,1,3);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    jiben_thresh_process();
}

//基本二值化处理
void base_process::jiben_thresh_process()
{
    int select_threshway = jiben_thresh_func->currentData(Qt::UserRole).toInt();//0~4
    int maxvalue;

    if(select_threshway == 0 || select_threshway == 1)
    {
        maxvalue = jibenthresh_max_SpinBox->text().toInt();
        jibenthresh_max_slider->setEnabled(true);
        jibenthresh_max_SpinBox->setEnabled(true);
    }
    else
    {
        maxvalue = 255;
        jibenthresh_max_slider->setEnabled(false);
        jibenthresh_max_SpinBox->setEnabled(false);
    }

    int thresh_value = jibenthresh_SpinBox->text().toInt();

    Mat temp_gray;

    cvtColor(srcImage,temp_gray,COLOR_RGB2GRAY);
    threshold(temp_gray,dstImage,thresh_value,maxvalue,select_threshway);
    dstlabel_indexed8_show(dstImage);
}

//OTSU + 基本二值化
void base_process::on_actionTHRESH_OTSU_triggered()
{  
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    jiben_THRESH_OTSU_func = new QComboBox(ui->widget_for_layout);
    jiben_THRESH_OTSU_func->addItem(tr("THRESH_OTSU + THRESH_BINARY"),0);
    jiben_THRESH_OTSU_func->addItem(tr("THRESH_OTSU + THRESH_BINARY_INV"),1);
    jiben_THRESH_OTSU_func->addItem(tr("THRESH_OTSU + THRESH_TRUNC"),2);
    jiben_THRESH_OTSU_func->addItem(tr("THRESH_OTSU + THRESH_TOZERO"),3);
    jiben_THRESH_OTSU_func->addItem(tr("THRESH_OTSU + THRESH_TOZERO_INV"),4);
    connect(jiben_THRESH_OTSU_func, SIGNAL(activated(const QString &)),this, SLOT(jiben_THRESH_OTSU_process()));

    show_max_input_THRESH_OTSU_value = new QLabel(tr("THRESH_BINARY | THRESH_BINARY_INV的最大值："),ui->widget_for_layout);

    THRESH_OTSU_max_SpinBox = new QSpinBox(ui->widget_for_layout);
    THRESH_OTSU_max_SpinBox->setMinimum(1);
    THRESH_OTSU_max_SpinBox->setMaximum(255);
    THRESH_OTSU_max_SpinBox->setValue(255);//设置一个默认值

    THRESH_OTSU_max_slider = new QSlider(ui->widget_for_layout);
    THRESH_OTSU_max_slider->setOrientation(Qt::Horizontal);//水平方向
    THRESH_OTSU_max_slider->setMinimum(1);
    THRESH_OTSU_max_slider->setMaximum(255);
    THRESH_OTSU_max_slider->setSingleStep(10);//步长 动一下移动的距离
    THRESH_OTSU_max_slider->setTickInterval(10); //设置刻度间隔
    THRESH_OTSU_max_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    THRESH_OTSU_max_slider->setValue(255);//设置一个默认值

    connect(THRESH_OTSU_max_slider, SIGNAL(valueChanged(int)), THRESH_OTSU_max_SpinBox, SLOT(setValue(int)));
    connect(THRESH_OTSU_max_SpinBox, SIGNAL(valueChanged(int)), THRESH_OTSU_max_slider, SLOT(setValue(int)));

    connect(THRESH_OTSU_max_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_OTSU_process()));
    connect(THRESH_OTSU_max_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_OTSU_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(jiben_THRESH_OTSU_func,0,0);
    widget_layout->addWidget(show_max_input_THRESH_OTSU_value,1,0);
    widget_layout->addWidget(THRESH_OTSU_max_SpinBox,1,1);
    widget_layout->addWidget(THRESH_OTSU_max_slider,1,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    jiben_THRESH_OTSU_process();
}

//OTSU + 基本二值化处理
void base_process::jiben_THRESH_OTSU_process()
{
    int jiben_THRESH_OTSU_funcvarint = jiben_THRESH_OTSU_func->currentData(Qt::UserRole).toInt();//0~4
    int maxvalue;

    if(jiben_THRESH_OTSU_funcvarint == 0 || jiben_THRESH_OTSU_funcvarint == 1)
    {
        maxvalue = THRESH_OTSU_max_SpinBox->text().toInt();
        THRESH_OTSU_max_SpinBox->setEnabled(true);
        THRESH_OTSU_max_slider->setEnabled(true);
    }
    else
    {
        maxvalue = 255;
        THRESH_OTSU_max_SpinBox->setEnabled(false);
        THRESH_OTSU_max_slider->setEnabled(false);
    }

    Mat temp_gray;

    cvtColor(srcImage,temp_gray,COLOR_RGB2GRAY);

    int thresh_type;
    switch (jiben_THRESH_OTSU_funcvarint)
    {
        case 0: thresh_type = THRESH_OTSU | THRESH_BINARY; break;
        case 1: thresh_type = THRESH_OTSU | THRESH_BINARY_INV; break;
        case 2: thresh_type = THRESH_OTSU | THRESH_TRUNC;  break;
        case 3: thresh_type = THRESH_OTSU | THRESH_TOZERO;  break;
        case 4: thresh_type = THRESH_OTSU | THRESH_TOZERO_INV;  break;
    }
    //当使用OTSU的时候，第三个参数是不起作用的
    threshold(temp_gray,dstImage,0,maxvalue,thresh_type);
    dstlabel_indexed8_show(dstImage);
}

//THRESH_TRIANGLE + 基本二值化
void base_process::on_actionTHRESH_TRIANGLE_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    jiben_THRESH_TRIANGLE_func = new QComboBox(ui->widget_for_layout);
    jiben_THRESH_TRIANGLE_func->addItem(tr("THRESH_TRIANGLE + THRESH_BINARY"),0);
    jiben_THRESH_TRIANGLE_func->addItem(tr("THRESH_TRIANGLE + THRESH_BINARY_INV"),1);
    jiben_THRESH_TRIANGLE_func->addItem(tr("THRESH_TRIANGLE + THRESH_TRUNC"),2);
    jiben_THRESH_TRIANGLE_func->addItem(tr("THRESH_TRIANGLE + THRESH_TOZERO"),3);
    jiben_THRESH_TRIANGLE_func->addItem(tr("THRESH_TRIANGLE + THRESH_TOZERO_INV"),4);
    connect(jiben_THRESH_TRIANGLE_func, SIGNAL(activated(const QString &)),this, SLOT(jiben_THRESH_TRIANGLE_process()));

    show_input_THRESH_TRIANGLE_value = new QLabel(tr("THRESH_BINARY | THRESH_BINARY_INV的最大值："),ui->widget_for_layout);

    THRESH_TRIANGLE_max_SpinBox = new QSpinBox(ui->widget_for_layout);
    THRESH_TRIANGLE_max_SpinBox->setMinimum(1);
    THRESH_TRIANGLE_max_SpinBox->setMaximum(255);
    THRESH_TRIANGLE_max_SpinBox->setValue(255);//设置一个默认值

    THRESH_TRIANGLE_max_slider = new QSlider(ui->widget_for_layout);
    THRESH_TRIANGLE_max_slider->setOrientation(Qt::Horizontal);//水平方向
    THRESH_TRIANGLE_max_slider->setMinimum(1);
    THRESH_TRIANGLE_max_slider->setMaximum(255);
    THRESH_TRIANGLE_max_slider->setSingleStep(10);//步长 动一下移动的距离
    THRESH_TRIANGLE_max_slider->setTickInterval(10); // 设置刻度间隔
    THRESH_TRIANGLE_max_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    THRESH_TRIANGLE_max_slider->setValue(255);//设置一个默认值

    connect(THRESH_TRIANGLE_max_slider, SIGNAL(valueChanged(int)), THRESH_TRIANGLE_max_SpinBox, SLOT(setValue(int)));
    connect(THRESH_TRIANGLE_max_SpinBox, SIGNAL(valueChanged(int)), THRESH_TRIANGLE_max_slider, SLOT(setValue(int)));

    connect(THRESH_TRIANGLE_max_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_TRIANGLE_process()));
    connect(THRESH_TRIANGLE_max_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_TRIANGLE_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(jiben_THRESH_TRIANGLE_func,0,0);
    widget_layout->addWidget(show_input_THRESH_TRIANGLE_value,1,0);
    widget_layout->addWidget(THRESH_TRIANGLE_max_SpinBox,1,1);
    widget_layout->addWidget(THRESH_TRIANGLE_max_slider,1,2);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    jiben_THRESH_TRIANGLE_process();
}

//THRESH_TRIANGLE + 基本二值化处理
void base_process::jiben_THRESH_TRIANGLE_process()
{  
    int jiben_THRESH_TRIANGLE_funcvarint = jiben_THRESH_TRIANGLE_func->currentData(Qt::UserRole).toInt();//0~4

    int maxvalue;
    if(jiben_THRESH_TRIANGLE_funcvarint == 0 || jiben_THRESH_TRIANGLE_funcvarint == 1)
    {
        maxvalue = THRESH_TRIANGLE_max_SpinBox->text().toInt();
        THRESH_TRIANGLE_max_slider->setEnabled(true);
        THRESH_TRIANGLE_max_SpinBox->setEnabled(true);
    }
    else
    {
        maxvalue = 255;
        THRESH_TRIANGLE_max_slider->setEnabled(false);
        THRESH_TRIANGLE_max_SpinBox->setEnabled(false);
    }

    int thresh_type;
    Mat temp_gray;

    switch (jiben_THRESH_TRIANGLE_funcvarint)
    {
        case 0: thresh_type = THRESH_TRIANGLE | THRESH_BINARY; break;
        case 1: thresh_type = THRESH_TRIANGLE | THRESH_BINARY_INV; break;
        case 2: thresh_type = THRESH_TRIANGLE | THRESH_TRUNC;  break;
        case 3: thresh_type = THRESH_TRIANGLE | THRESH_TOZERO;  break;
        case 4: thresh_type = THRESH_TRIANGLE | THRESH_TOZERO_INV;  break;
    }

    cvtColor(srcImage,temp_gray,COLOR_RGB2GRAY);

    //当使用THRESH_TRIANGLE的时候，第三个参数是不起作用的
    threshold(temp_gray,dstImage,250,maxvalue,thresh_type);
    dstlabel_indexed8_show(dstImage);
}

//自适应二值化
void base_process::on_action_adapt_triggered()
{    
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    jiben_THRESH_adapt_show01 = new QLabel(tr("自适应阈值算法："),ui->widget_for_layout);
    jiben_THRESH_adapt_show02 = new QLabel(tr("阈值类型："),ui->widget_for_layout);
    jiben_THRESH_adapt_show03 = new QLabel(tr("最大阈值："),ui->widget_for_layout);
    jiben_THRESH_adapt_show04 = new QLabel(tr("偏移常量："),ui->widget_for_layout);

    jiben_THRESH_adapt_meth = new QComboBox(ui->widget_for_layout);
    jiben_THRESH_adapt_meth->addItem(tr("ADAPTIVE_THRESH_MEAN_C"),ADAPTIVE_THRESH_MEAN_C);
    jiben_THRESH_adapt_meth->addItem(tr("ADAPTIVE_THRESH_GAUSSIAN_C"),ADAPTIVE_THRESH_GAUSSIAN_C);
    connect(jiben_THRESH_adapt_meth, SIGNAL(activated(const QString &)),this, SLOT(jiben_THRESH_adapt_process()));

    jiben_THRESH_adapt_type = new QComboBox(ui->widget_for_layout);
    jiben_THRESH_adapt_type->addItem(tr("THRESH_BINARY"),THRESH_BINARY);
    jiben_THRESH_adapt_type->addItem(tr("THRESH_BINARY_INV"),THRESH_BINARY_INV);
    connect(jiben_THRESH_adapt_type, SIGNAL(activated(const QString &)),this, SLOT(jiben_THRESH_adapt_process()));

    adapt_max_SpinBox = new QSpinBox(ui->widget_for_layout);
    adapt_max_SpinBox->setMinimum(1);
    adapt_max_SpinBox->setMaximum(255);
    adapt_max_SpinBox->setValue(255);//设置一个默认值

    adapt_max_slider = new QSlider(ui->widget_for_layout);
    adapt_max_slider->setOrientation(Qt::Horizontal);//水平方向
    adapt_max_slider->setMinimum(1);
    adapt_max_slider->setMaximum(255);
    adapt_max_slider->setSingleStep(10);//步长 动一下移动的距离
    adapt_max_slider->setTickInterval(10); // 设置刻度间隔
    adapt_max_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    adapt_max_slider->setValue(255);//设置一个默认值

    connect(adapt_max_slider, SIGNAL(valueChanged(int)), adapt_max_SpinBox, SLOT(setValue(int)));
    connect(adapt_max_SpinBox, SIGNAL(valueChanged(int)), adapt_max_slider, SLOT(setValue(int)));

    connect(adapt_max_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_adapt_process()));
    connect(adapt_max_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_adapt_process()));

    adapt_blocksize = new QComboBox(ui->widget_for_layout);
    for(int i = 3;i <= 20;i += 2)
    {
        adapt_blocksize->addItem(tr("邻域范围：%1").arg(i),i);
    }
    connect(adapt_blocksize, SIGNAL(activated(const QString &)),this, SLOT(jiben_THRESH_adapt_process()));

    adapt_C_SpinBox = new QSpinBox(ui->widget_for_layout);
    adapt_C_SpinBox->setMinimum(-50);
    adapt_C_SpinBox->setMaximum(50);
    adapt_C_SpinBox->setSingleStep(1);
    adapt_C_SpinBox->setValue(1);//设置一个默认值

    adapt_C_slider = new QSlider(ui->widget_for_layout);
    adapt_C_slider->setOrientation(Qt::Horizontal);//水平方向
    adapt_C_slider->setMinimum(-50);
    adapt_C_slider->setMaximum(50);
    adapt_C_slider->setSingleStep(1);//步长 动一下移动的距离
    adapt_C_slider->setTickInterval(4); // 设置刻度间隔
    adapt_C_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    adapt_C_slider->setValue(1);//设置一个默认值

    connect(adapt_C_slider, SIGNAL(valueChanged(int)), adapt_C_SpinBox, SLOT(setValue(int)));
    connect(adapt_C_SpinBox, SIGNAL(valueChanged(int)), adapt_C_slider, SLOT(setValue(int)));

    connect(adapt_C_slider, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_adapt_process()));
    connect(adapt_C_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(jiben_THRESH_adapt_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(adapt_blocksize,0,0);
    widget_layout->addWidget(jiben_THRESH_adapt_show02,0,1);
    widget_layout->addWidget(jiben_THRESH_adapt_type,0,2);
    widget_layout->addWidget(jiben_THRESH_adapt_show01,0,3);
    widget_layout->addWidget(jiben_THRESH_adapt_meth,0,4);
    widget_layout->addWidget(jiben_THRESH_adapt_show03,1,0);
    widget_layout->addWidget(adapt_max_SpinBox,1,1);
    widget_layout->addWidget(adapt_max_slider,1,2,1,3);
    widget_layout->addWidget(jiben_THRESH_adapt_show04,2,0);
    widget_layout->addWidget(adapt_C_SpinBox,2,1);
    widget_layout->addWidget(adapt_C_slider,2,2,1,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    jiben_THRESH_adapt_process();
}

//自适应二值化处理
void base_process::jiben_THRESH_adapt_process()
{
    Mat temp_gray;

    cvtColor(srcImage,temp_gray,COLOR_RGB2GRAY);
    adaptiveThreshold(temp_gray,
                      dstImage,
                      adapt_max_SpinBox->text().toInt(),
                      jiben_THRESH_adapt_meth->currentData(Qt::UserRole).toInt(),
                      jiben_THRESH_adapt_type->currentData(Qt::UserRole).toInt(),
                      adapt_blocksize->currentData(Qt::UserRole).toInt(),
                      adapt_C_SpinBox->text().toInt());
    dstlabel_indexed8_show(dstImage);
}

//sobel算子
void base_process::on_actionSobel_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    sobel_fangxiang = new QComboBox(ui->widget_for_layout);
    sobel_fangxiang->addItem(tr("梯度方向：dx"),1);
    sobel_fangxiang->addItem(tr("梯度方向：dy"),2);
    sobel_fangxiang->addItem(tr("梯度方向：dx & dy"),3);
    connect(sobel_fangxiang, SIGNAL(activated(const QString &)),this, SLOT(sobel_process()));

    sobel_ksize = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        sobel_ksize->addItem(tr("邻域范围：%1").arg(i),i);
    }
    connect(sobel_ksize, SIGNAL(activated(const QString &)),this, SLOT(sobel_process()));

    sobel_suofangyinzi_Label = new QLabel(tr("缩放因子："),ui->widget_for_layout);
    show_liangdu =  new QLabel(tr("设置亮度："),ui->widget_for_layout);

    sobel_suofangyinzi_show = new QLabel(tr("1.00"),ui->widget_for_layout);
    sobel_suofangyinzi_show->setStyleSheet("background-color:white");

    sobel_suofangyinzi_Slider = new QSlider(ui->widget_for_layout);
    sobel_suofangyinzi_Slider->setOrientation(Qt::Horizontal);//水平方向
    sobel_suofangyinzi_Slider->setMinimum(1);
    sobel_suofangyinzi_Slider->setMaximum(100);
    sobel_suofangyinzi_Slider->setSingleStep(10);//步长 动一下移动的距离
    sobel_suofangyinzi_Slider->setTickInterval(5); // 设置刻度间隔
    sobel_suofangyinzi_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    sobel_suofangyinzi_Slider->setValue(10);//设置一个默认值
    connect(sobel_suofangyinzi_Slider, SIGNAL(valueChanged(int)), this, SLOT(sobel_process()));

    sobel_lianagdu_SpinBox = new QSpinBox(ui->widget_for_layout);
    sobel_lianagdu_SpinBox->setMinimum(-100);
    sobel_lianagdu_SpinBox->setMaximum(100);
    sobel_lianagdu_SpinBox->setValue(0);//设置一个默认值

    sobel_lianagdu_Slider = new QSlider(ui->widget_for_layout);
    sobel_lianagdu_Slider->setOrientation(Qt::Horizontal);//水平方向
    sobel_lianagdu_Slider->setMinimum(-100);
    sobel_lianagdu_Slider->setMaximum(100);
    sobel_lianagdu_Slider->setSingleStep(10);//步长 动一下移动的距离
    sobel_lianagdu_Slider->setTickInterval(10); // 设置刻度间隔
    sobel_lianagdu_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    sobel_lianagdu_Slider->setValue(0);//设置一个默认值

    connect(sobel_lianagdu_SpinBox, SIGNAL(valueChanged(int)), sobel_lianagdu_Slider, SLOT(setValue(int)));
    connect(sobel_lianagdu_Slider, SIGNAL(valueChanged(int)), sobel_lianagdu_SpinBox, SLOT(setValue(int)));

    connect(sobel_lianagdu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(sobel_process()));
    connect(sobel_lianagdu_Slider, SIGNAL(valueChanged(int)), this, SLOT(sobel_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(sobel_fangxiang,0,0);
    widget_layout->addWidget(sobel_ksize,0,1);
    widget_layout->addWidget(show_liangdu,1,0);
    widget_layout->addWidget(sobel_lianagdu_SpinBox,1,1);
    widget_layout->addWidget(sobel_lianagdu_Slider,1,2,1,5);
    widget_layout->addWidget(sobel_suofangyinzi_Label,2,0);
    widget_layout->addWidget(sobel_suofangyinzi_show,2,1);
    widget_layout->addWidget(sobel_suofangyinzi_Slider,2,2,1,5);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    sobel_process();
}

//sobel算子处理
void base_process::sobel_process()
{
    double temp = double(sobel_suofangyinzi_Slider->value())/10;
    sobel_suofangyinzi_show->setText(QString::number(temp,10,2));

    int dx,dy;
    int sobel_fangxiangvarint = sobel_fangxiang->currentData(Qt::UserRole).toInt();

    if(sobel_fangxiangvarint == 1)
    {
        dx = 1;
        dy = 0;
    }
    if(sobel_fangxiangvarint == 2)
    {
        dx = 0;
        dy = 1;
    }
    if(sobel_fangxiangvarint == 3)
    {
        dx = 1;
        dy = 1;
    }

    Sobel(srcImage,
          dstImage,
          srcImage.depth(),
          dx,
          dy,
          sobel_ksize->currentData(Qt::UserRole).toInt(),
          temp,
          sobel_lianagdu_SpinBox->value());
    dstlabel_show(dstImage);
}

//canny算子
void base_process::on_actionCanny_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    show_input_thresh1 = new QLabel(tr("低阈值：1~254"),ui->widget_for_layout);
    show_input_thresh2 = new QLabel(tr("高阈值：2~1000"),ui->widget_for_layout);

    canny_SpinBox_l = new QSpinBox(ui->widget_for_layout);
    canny_SpinBox_l->setMinimum(1);
    canny_SpinBox_l->setMaximum(254);
    canny_SpinBox_l->setValue(1);//设置一个默认值

    canny_slider_l = new QSlider(ui->widget_for_layout);
    canny_slider_l->setOrientation(Qt::Horizontal);//水平方向
    canny_slider_l->setMinimum(1);
    canny_slider_l->setMaximum(254);
    canny_slider_l->setSingleStep(10);//步长 动一下移动的距离
    canny_slider_l->setTickInterval(10); // 设置刻度间隔
    canny_slider_l->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    canny_slider_l->setValue(1);//设置一个默认值

    connect(canny_slider_l, SIGNAL(valueChanged(int)), canny_SpinBox_l, SLOT(setValue(int)));
    connect(canny_SpinBox_l, SIGNAL(valueChanged(int)), canny_slider_l, SLOT(setValue(int)));

    connect(canny_slider_l, SIGNAL(valueChanged(int)), this, SLOT(canny_process()));
    connect(canny_SpinBox_l, SIGNAL(valueChanged(int)), this, SLOT(canny_process()));

    canny_SpinBox_h = new QSpinBox(ui->widget_for_layout);
    canny_SpinBox_h->setMinimum(2);
    canny_SpinBox_h->setMaximum(1000);
    canny_SpinBox_h->setValue(2);//设置一个默认值

    canny_slider_h = new QSlider(ui->widget_for_layout);
    canny_slider_h->setOrientation(Qt::Horizontal);//水平方向
    canny_slider_h->setMinimum(2);
    canny_slider_h->setMaximum(1000);
    canny_slider_h->setSingleStep(20);//步长 动一下移动的距离
    canny_slider_h->setTickInterval(40); // 设置刻度间隔
    canny_slider_h->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    canny_slider_h->setValue(2);//设置一个默认值

    connect(canny_slider_h, SIGNAL(valueChanged(int)), canny_SpinBox_h, SLOT(setValue(int)));
    connect(canny_SpinBox_h, SIGNAL(valueChanged(int)), canny_slider_h, SLOT(setValue(int)));

    connect(canny_slider_h, SIGNAL(valueChanged(int)), this, SLOT(canny_process()));
    connect(canny_SpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(canny_process()));

    canny_ksize = new QComboBox(ui->widget_for_layout);
    canny_ksize->addItem(tr("核尺寸：3"),3);
    canny_ksize->addItem(tr("核尺寸：5"),5);
    canny_ksize->addItem(tr("核尺寸：7"),7);
    connect(canny_ksize, SIGNAL(activated(const QString &)),this, SLOT(canny_process()));

    L2gradient = new QComboBox(ui->widget_for_layout);
    L2gradient->addItem(tr("梯度强度：L1范数"),0);
    L2gradient->addItem(tr("梯度强度：L2范数"),1);
    connect(L2gradient, SIGNAL(activated(const QString &)),this, SLOT(canny_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(canny_ksize,0,0);
    widget_layout->addWidget(L2gradient,0,1);
    widget_layout->addWidget(show_input_thresh1,1,0);
    widget_layout->addWidget(canny_SpinBox_l,1,1);
    widget_layout->addWidget(canny_slider_l,1,2);
    widget_layout->addWidget(show_input_thresh2,2,0);
    widget_layout->addWidget(canny_SpinBox_h,2,1);
    widget_layout->addWidget(canny_slider_h,2,2);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    canny_process();
}

//canny算子处理
void base_process::canny_process()
{
    int lowthreshint = canny_SpinBox_l->text().toInt();
    int threshdif = lowthreshint + 1;

    QString str = "高阈值：" + QString("%1").arg(threshdif) + "~1000";
    show_input_thresh2->setText(str);

    canny_slider_h->setMinimum(threshdif);
    canny_SpinBox_h->setMinimum(threshdif);

    bool isL2gradient = false;
    if(L2gradient->currentData(Qt::UserRole).toInt() == 0)
    {
        isL2gradient = false;
    }
    else
    {
        isL2gradient = true;
    }

    Mat srctemp;
    cvtColor(srcImage,srctemp,CV_BGR2GRAY);
    Canny(srctemp,
          dstImage,
          lowthreshint,
          canny_SpinBox_h->text().toInt(),
          canny_ksize->currentData(Qt::UserRole).toInt(),
          isL2gradient);
    dstlabel_indexed8_show(dstImage);
}

//Laplacian算子
void base_process::on_actionLaplacian_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    Laplacian_ksize = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        Laplacian_ksize->addItem(tr("核尺寸：%1 × %1").arg(i),i);
    }
    connect(Laplacian_ksize, SIGNAL(activated(const QString &)),this, SLOT(Laplacian_process()));

    Laplacian_LineEdit = new QLineEdit(ui->widget_for_layout);
    Laplacian_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    Laplacian_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    Laplacian_suofangyinzi_Slider = new QSlider(ui->widget_for_layout);
    Laplacian_suofangyinzi_Slider->setOrientation(Qt::Horizontal);//水平方向
    Laplacian_suofangyinzi_Slider->setMinimum(1);
    Laplacian_suofangyinzi_Slider->setMaximum(100);
    Laplacian_suofangyinzi_Slider->setSingleStep(10);//步长 动一下移动的距离
    Laplacian_suofangyinzi_Slider->setTickInterval(5); // 设置刻度间隔
    Laplacian_suofangyinzi_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Laplacian_suofangyinzi_Slider->setValue(10);//设置一个默认值
    connect(Laplacian_suofangyinzi_Slider, SIGNAL(valueChanged(int)), this, SLOT(Laplacian_process()));

    Laplacian_lianagdu_SpinBox = new QSpinBox(ui->widget_for_layout);
    Laplacian_lianagdu_SpinBox->setMinimum(-100);
    Laplacian_lianagdu_SpinBox->setMaximum(100);
    Laplacian_lianagdu_SpinBox->setValue(0);//设置一个默认值

    Laplacian_lianagdu_Slider = new QSlider(ui->widget_for_layout);
    Laplacian_lianagdu_Slider->setOrientation(Qt::Horizontal);//水平方向
    Laplacian_lianagdu_Slider->setMinimum(-100);
    Laplacian_lianagdu_Slider->setMaximum(100);
    Laplacian_lianagdu_Slider->setSingleStep(10);//步长 动一下移动的距离
    Laplacian_lianagdu_Slider->setTickInterval(10); // 设置刻度间隔
    Laplacian_lianagdu_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Laplacian_lianagdu_Slider->setValue(0);//设置一个默认值

    connect(Laplacian_lianagdu_SpinBox, SIGNAL(valueChanged(int)), Laplacian_lianagdu_Slider, SLOT(setValue(int)));
    connect(Laplacian_lianagdu_Slider, SIGNAL(valueChanged(int)), Laplacian_lianagdu_SpinBox, SLOT(setValue(int)));

    connect(Laplacian_lianagdu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Laplacian_process()));
    connect(Laplacian_lianagdu_Slider, SIGNAL(valueChanged(int)), this, SLOT(Laplacian_process()));

    Laplacian_lianagdu_Label =  new QLabel(tr("设置亮度："),ui->widget_for_layout);
    Laplacian_suofangyinzi_Label = new QLabel(tr("缩放因子："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(Laplacian_ksize,0,0);
    widget_layout->addWidget(Laplacian_lianagdu_Label,1,0);
    widget_layout->addWidget(Laplacian_lianagdu_SpinBox,1,1);
    widget_layout->addWidget(Laplacian_lianagdu_Slider,1,2,1,5);
    widget_layout->addWidget(Laplacian_suofangyinzi_Label,2,0);
    widget_layout->addWidget(Laplacian_LineEdit,2,1);
    widget_layout->addWidget(Laplacian_suofangyinzi_Slider,2,2,1,5);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    Laplacian_process();
}

//Laplacian算子处理
void base_process::Laplacian_process()
{
    double temp = double(Laplacian_suofangyinzi_Slider->value())/10;
    Laplacian_LineEdit->setText(QString::number(temp,10,2));

    Laplacian(srcImage,
              dstImage,
              srcImage.depth(),
              Laplacian_ksize->currentData(Qt::UserRole).toInt(),
              temp,
              Laplacian_lianagdu_SpinBox->text().toInt());
    dstlabel_show(dstImage);
}

//Scharr算子
void base_process::on_actionScharr_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    Scharr_fangxiang = new QComboBox(ui->widget_for_layout);
    Scharr_fangxiang->addItem(tr("梯度方向：dx"),1);
    Scharr_fangxiang->addItem(tr("梯度方向：dy"),2);
    connect(Scharr_fangxiang, SIGNAL(activated(const QString &)),this, SLOT(Scharr_process()));

    Scharr_suofangyinzi_LineEdit = new QLineEdit(ui->widget_for_layout);
    Scharr_suofangyinzi_LineEdit->setFocusPolicy(Qt::NoFocus);//无法获得焦点，即无法编辑
    Scharr_suofangyinzi_LineEdit->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);

    Scharr_suofangyinzi_Slider = new QSlider(ui->widget_for_layout);
    Scharr_suofangyinzi_Slider->setOrientation(Qt::Horizontal);//水平方向
    Scharr_suofangyinzi_Slider->setMinimum(1);
    Scharr_suofangyinzi_Slider->setMaximum(100);
    Scharr_suofangyinzi_Slider->setSingleStep(10);//步长 动一下移动的距离
    Scharr_suofangyinzi_Slider->setTickInterval(5); // 设置刻度间隔
    Scharr_suofangyinzi_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Scharr_suofangyinzi_Slider->setValue(10);//设置一个默认值
    connect(Scharr_suofangyinzi_Slider, SIGNAL(valueChanged(int)), this, SLOT(Scharr_process()));

    Scharr_lianagdu_SpinBox = new QSpinBox(ui->widget_for_layout);
    Scharr_lianagdu_SpinBox->setMinimum(-100);
    Scharr_lianagdu_SpinBox->setMaximum(100);
    Scharr_lianagdu_SpinBox->setValue(0);//设置一个默认值

    Scharr_lianagdu_Slider = new QSlider(ui->widget_for_layout);
    Scharr_lianagdu_Slider->setOrientation(Qt::Horizontal);//水平方向
    Scharr_lianagdu_Slider->setMinimum(-100);
    Scharr_lianagdu_Slider->setMaximum(100);
    Scharr_lianagdu_Slider->setSingleStep(10);//步长 动一下移动的距离
    Scharr_lianagdu_Slider->setTickInterval(10); // 设置刻度间隔
    Scharr_lianagdu_Slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Scharr_lianagdu_Slider->setValue(0);//设置一个默认值

    connect(Scharr_lianagdu_SpinBox, SIGNAL(valueChanged(int)), Scharr_lianagdu_Slider, SLOT(setValue(int)));
    connect(Scharr_lianagdu_Slider, SIGNAL(valueChanged(int)), Scharr_lianagdu_SpinBox, SLOT(setValue(int)));

    connect(Scharr_lianagdu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Scharr_process()));
    connect(Scharr_lianagdu_Slider, SIGNAL(valueChanged(int)), this, SLOT(Scharr_process()));

    Scharr_lianagdu_Label =  new QLabel(tr("设置亮度："),ui->widget_for_layout);
    Scharr_suofangyinzi_Label = new QLabel(tr("缩放因子："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(Scharr_fangxiang,0,0);
    widget_layout->addWidget(Scharr_lianagdu_Label,1,0);
    widget_layout->addWidget(Scharr_lianagdu_SpinBox,1,1);
    widget_layout->addWidget(Scharr_lianagdu_Slider,1,2,1,5);
    widget_layout->addWidget(Scharr_suofangyinzi_Label,2,0);
    widget_layout->addWidget(Scharr_suofangyinzi_LineEdit,2,1);
    widget_layout->addWidget(Scharr_suofangyinzi_Slider,2,2,1,5);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    Scharr_process();
}

//Scharr算子处理
void base_process::Scharr_process()
{
    double temp = double(Scharr_suofangyinzi_Slider->value())/10;
    Scharr_suofangyinzi_LineEdit->setText(QString::number(temp,10,2));

    int dx,dy;
    if(Scharr_fangxiang->currentData(Qt::UserRole).toInt() == 1)
    {
        dx = 1;
        dy = 0;
    }
    else
    {
        dx = 0;
        dy = 1;
    }

    Scharr(srcImage,
           dstImage,
           srcImage.depth(),
           dx,
           dy,
           temp,
           Scharr_lianagdu_SpinBox->text().toInt());
    dstlabel_show(dstImage);
}

//颜色效果
void base_process::on_action_color_lvjing_triggered()
{  
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    color_xiaoguo = new QComboBox(ui->widget_for_layout);
    color_xiaoguo->addItem(tr("转换到HSV颜色模型"),1);
    color_xiaoguo->addItem(tr("转换到Lab颜色模型"),2);
    color_xiaoguo->addItem(tr("转换到Luv颜色模型"),3);
    color_xiaoguo->addItem(tr("转换到HLS颜色模型"),4);
    color_xiaoguo->addItem(tr("转换到YUV颜色模型"),5);
    color_xiaoguo->addItem(tr("转换到YCC颜色模型"),14);
    color_xiaoguo->addItem(tr("转换到CIEXYZ颜色模型"),15);
    color_xiaoguo->addItem(tr("灰度图"),6);
    color_xiaoguo->addItem(tr("红绿通道互换"),7);
    color_xiaoguo->addItem(tr("R通道"),8);
    color_xiaoguo->addItem(tr("G通道"),9);
    color_xiaoguo->addItem(tr("B通道"),10);
    color_xiaoguo->addItem(tr("R通道-彩色"),11);
    color_xiaoguo->addItem(tr("G通道-彩色"),12);
    color_xiaoguo->addItem(tr("B通道-彩色"),13);
    connect(color_xiaoguo, SIGNAL(activated(const QString &)),this, SLOT(color_lvjing_process()));

    color_show = new QLabel(tr("颜色转换类型："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(color_show,0,0);
    widget_layout->addWidget(color_xiaoguo,0,1);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    color_lvjing_process();
}

//颜色效果
void base_process::color_lvjing_process()
{    
    int color_xiaoguovarint = color_xiaoguo->currentData(Qt::UserRole).toInt();
    if(color_xiaoguovarint == 1)//转换到HSV颜色空间
    {
        cvtColor(srcImage,dstImage,CV_RGB2HSV);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 2)//转换到Lab颜色模型
    {
        cvtColor(srcImage,dstImage,CV_RGB2Lab);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 3)//转换到Luv颜色模型
    {
        cvtColor(srcImage,dstImage,CV_RGB2Luv);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 4)
    {
        cvtColor(srcImage,dstImage,CV_RGB2HLS);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 5)
    {
        cvtColor(srcImage,dstImage,CV_RGB2YUV);
        dstlabel_show(dstImage);
    }
    if(color_xiaoguovarint == 14)
    {
        cvtColor(srcImage,dstImage,CV_RGB2YCrCb);
        dstlabel_show(dstImage);
    }
    if(color_xiaoguovarint == 15)
    {
        cvtColor(srcImage,dstImage,CV_RGB2XYZ);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 6)
    {
        cvtColor(srcImage,dstImage,CV_RGB2GRAY);
        dstlabel_indexed8_show(dstImage);
    }
    else if(color_xiaoguovarint == 7)
    {
        cvtColor(srcImage,dstImage,CV_RGB2BGR);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 8)//R通道
    {
        Mat dst[3];
        Mat dstR(srcImage.rows,srcImage.cols,CV_8UC1);
        split(srcImage,dst);
        dst[0].copyTo(dstR);
        dstR.copyTo(dstImage);
        dstlabel_indexed8_show(dstR);
    }
    else if(color_xiaoguovarint == 9)//G通道
    {
        Mat dst[3];
        Mat dstG(srcImage.rows,srcImage.cols,CV_8UC1);
        split(srcImage,dst);
        dst[1].copyTo(dstG);
        dstG.copyTo(dstImage);
        dstlabel_indexed8_show(dstG);
    }
    else if(color_xiaoguovarint == 10)//B通道
    {
        Mat dst[3];
        Mat dstB(srcImage.rows,srcImage.cols,CV_8UC1);
        split(srcImage,dst);
        dst[2].copyTo(dstB);
        dstB.copyTo(dstImage);
        dstlabel_indexed8_show(dstB);
    }
    else if(color_xiaoguovarint == 11)//R通道-彩色
    {
        Mat dst[3];
        Mat dstG(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        Mat dstB(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        split(srcImage,dst);//分解 三个通道分别保存在数组dst
        dstB.copyTo(dst[0]);//两个为0的通道替换B、G 只保留R
        dstG.copyTo(dst[1]);
        merge(dst,3,dstImage);
        cvtColor(dstImage,dstImage,COLOR_BGR2RGB);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 12)//G通道-彩色
    {
        Mat dst[3];
        Mat dstB(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        Mat dstR(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        split(srcImage,dst);//保留G
        dstB.copyTo(dst[0]);
        dstR.copyTo(dst[2]);
        merge(dst,3,dstImage);
        cvtColor(dstImage,dstImage,COLOR_BGR2RGB);
        dstlabel_show(dstImage);
    }
    else if(color_xiaoguovarint == 13)//B通道-彩色
    {
        Mat dst[3];
        Mat dstG(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        Mat dstR(srcImage.rows,srcImage.cols,CV_8UC1,Scalar::all(0));
        split(srcImage,dst);//保留B
        dstG.copyTo(dst[1]);
        dstR.copyTo(dst[2]);
        merge(dst,3,dstImage);
        cvtColor(dstImage,dstImage,COLOR_BGR2RGB);
        dstlabel_show(dstImage);
    }
}

//彩色映射
void base_process::on_action_change_secaikongjian_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    color_kongjianzhuanhuan = new QComboBox(ui->widget_for_layout);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_AUTUMN"),COLORMAP_AUTUMN);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_BONE"),COLORMAP_BONE);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_JET"),COLORMAP_JET);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_WINTER"),COLORMAP_WINTER);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_RAINBOW"),COLORMAP_RAINBOW);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_OCEAN"),COLORMAP_OCEAN);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_SUMMER"),COLORMAP_SUMMER);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_SPRING"),COLORMAP_SPRING);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_COOL"),COLORMAP_COOL);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_HSV"),COLORMAP_HSV);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_PINK"),COLORMAP_PINK);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_HOT"),COLORMAP_HOT);
    color_kongjianzhuanhuan->addItem(tr("COLORMAP_PARULA"),COLORMAP_PARULA);
    connect(color_kongjianzhuanhuan, SIGNAL(activated(const QString &)),this, SLOT(color_kongjianzhuanhuan_process()));

    color_kongjianzhuanhuan_show = new QLabel(tr("彩色色谱："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(color_kongjianzhuanhuan_show,0,0);
    widget_layout->addWidget(color_kongjianzhuanhuan,0,1);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    color_kongjianzhuanhuan_process();
}

//彩色映射
void base_process::color_kongjianzhuanhuan_process()
{
    applyColorMap(srcImage,dstImage,color_kongjianzhuanhuan->currentData(Qt::UserRole).toInt());
    dstlabel_show(dstImage);
}

//手工画效果
void base_process::on_action_shougonghua_2_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    select_shougonghua = new QComboBox(ui->widget_for_layout);
    select_shougonghua->addItem(tr("木雕效果"),1);
    select_shougonghua->addItem(tr("铅笔画效果"),2);
    select_shougonghua->addItem(tr("素描效果"),3);
    connect(select_shougonghua, SIGNAL(activated(const QString &)),this, SLOT(change_lvjing_shougonghua()));

    shougonghua_SpinBox = new QSpinBox(ui->widget_for_layout);
    shougonghua_SpinBox->setMinimum(1);
    shougonghua_SpinBox->setMaximum(150);
    shougonghua_SpinBox->setValue(1);//设置一个默认值

    shougonghua_slider = new QSlider(ui->widget_for_layout);
    shougonghua_slider->setOrientation(Qt::Horizontal);//水平方向
    shougonghua_slider->setMinimum(1);
    shougonghua_slider->setMaximum(150);
    shougonghua_slider->setSingleStep(5);//步长 动一下移动的距离
    shougonghua_slider->setTickInterval(5); // 设置刻度间隔
    shougonghua_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    shougonghua_slider->setValue(1);//设置一个默认值

    connect(shougonghua_slider, SIGNAL(valueChanged(int)), shougonghua_SpinBox, SLOT(setValue(int)));
    connect(shougonghua_SpinBox, SIGNAL(valueChanged(int)), shougonghua_slider, SLOT(setValue(int)));

    connect(shougonghua_slider, SIGNAL(valueChanged(int)), this, SLOT(change_lvjing_shougonghua()));
    connect(shougonghua_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(change_lvjing_shougonghua()));

    show_shougonghuayuzhi =  new QLabel(tr("设置阈值："),ui->widget_for_layout);
    show_gaosi =  new QLabel(tr("高斯核尺寸："),ui->widget_for_layout);
    show_gaosi->hide();

    sumiao_gauss_size = new QComboBox(ui->widget_for_layout);
    for(int i = 1;i <= 20;i += 2)
    {
        sumiao_gauss_size->addItem(tr("%1 × %1").arg(i),i);
    }
    sumiao_gauss_size->hide();
    connect(sumiao_gauss_size, SIGNAL(activated(const QString &)),this, SLOT(change_lvjing_shougonghua()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(select_shougonghua,0,0);
    widget_layout->addWidget(show_shougonghuayuzhi,0,1);
    widget_layout->addWidget(shougonghua_SpinBox,0,2);
    widget_layout->addWidget(shougonghua_slider,0,3);
    widget_layout->addWidget(show_gaosi,0,4);
    widget_layout->addWidget(sumiao_gauss_size,0,5);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    change_lvjing_shougonghua();
}

//手工画效果处理
void base_process::change_lvjing_shougonghua()
{
    int thvar = shougonghua_SpinBox->text().toInt();

    int select_shougonghuavarint = select_shougonghua->currentData(Qt::UserRole).toInt();

    if(select_shougonghuavarint == 1 || select_shougonghuavarint == 2)
    {
        shougonghua_SpinBox->show();
        shougonghua_slider->show();
        show_shougonghuayuzhi->show();

        sumiao_gauss_size->hide();
        show_gaosi->hide();
    }
    else if(select_shougonghuavarint == 3)
    {
        shougonghua_SpinBox->hide();
        shougonghua_slider->hide();
        show_shougonghuayuzhi->hide();

        sumiao_gauss_size->show();
        show_gaosi->show();
    }

    if(select_shougonghuavarint == 1)
    {
        Mat dst(srcImage.rows,srcImage.cols,CV_8UC1);
        Mat src;
        cvtColor(srcImage,src,CV_RGB2GRAY);
        for (int i=1; i<src.rows-1; i++)
        {
            //源图像
            uchar *current = src.ptr<uchar>(i);
            uchar *currentBefore = src.ptr<uchar>(i-1);
            uchar *currentNext = src.ptr<uchar>(i+1);
            //目标图像
            uchar *dstLine = dst.ptr<uchar>(i);
            for (int j=1; j<src.cols-1; j++)
            {
                int a,b,c,d,e,f,g,h;
                a=abs(current[j]-currentBefore[(j-1)]);
                b=abs(current[j]-currentBefore[j]);
                c=abs(current[j]-currentBefore[(j+1)]);
                d=abs(current[j]-currentNext[(j-1)]);
                e=abs(current[j]-currentNext[(j)]);
                f=abs(current[j]-currentNext[(j+1)]);
                g=abs(current[j]-current[(j-1)]);
                h=abs(current[j]-current[(j+1)]);

                if(a>thvar||b>thvar||c>thvar||d>thvar||e>thvar||f>thvar||g>thvar||h>thvar)
                {
                    dstLine[j]=255;
                }
                else
                {
                    dstLine[j]=0;
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_indexed8_show(dstImage);
    }
    else if(select_shougonghuavarint == 2)
    {
        Mat dst(srcImage.rows,srcImage.cols,CV_8UC1);
        Mat src;
        cvtColor(srcImage,src,CV_RGB2GRAY);
        for (int i=1; i<src.rows-1; i++)
        {
            //源图像
            uchar *current = src.ptr<uchar>(i);
            uchar *currentBefore = src.ptr<uchar>(i-1);
            uchar *currentNext = src.ptr<uchar>(i+1);
            //目标图像
            uchar *dstLine = dst.ptr<uchar>(i);
            for (int j=1; j<src.cols-1; j++)
            {
                int a,b,c,d,e,f,g,h;
                a=abs(current[j]-currentBefore[(j-1)]);
                b=abs(current[j]-currentBefore[j]);
                c=abs(current[j]-currentBefore[(j+1)]);
                d=abs(current[j]-currentNext[(j-1)]);
                e=abs(current[j]-currentNext[(j)]);
                f=abs(current[j]-currentNext[(j+1)]);
                g=abs(current[j]-current[(j-1)]);
                h=abs(current[j]-current[(j+1)]);

                if(a>thvar||b>thvar||c>thvar||d>thvar||e>thvar||f>thvar||g>thvar||h>thvar)
                {
                    dstLine[j]=0;
                }
                else
                {
                    dstLine[j]=255;
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_indexed8_show(dstImage);
    }
    else if(select_shougonghuavarint == 3)
    {
        //手工画-素描效果处理
        Mat src;
        srcImage.copyTo(src);
        Mat gray0,gray1;

        int sumiao_gauss_sizevarint = sumiao_gauss_size->currentData(Qt::UserRole).toInt();

        cvtColor(src,gray0,CV_RGB2GRAY);   //灰度处理
        gray1 = ~gray0;//反色
        GaussianBlur(gray1,gray1,Size(sumiao_gauss_sizevarint,sumiao_gauss_sizevarint),0);   //高斯滤波
        Mat dst(gray1.size(),CV_8UC1);
        for (int y=0; y<src.rows; y++)
        {
            uchar* P0  = gray0.ptr<uchar>(y);
            uchar* P1  = gray1.ptr<uchar>(y);
            uchar* P  = dst.ptr<uchar>(y);
            for (int x=0; x<src.cols; x++)
            {
                int tmp0=P0[x];
                int tmp1=P1[x];
                P[x] =(uchar) min((tmp0+(tmp0*tmp1)/(256-tmp1)),255);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_indexed8_show(dstImage);
    }
}

//艺术效果
void base_process::on_action_art_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    select_art = new QComboBox(ui->widget_for_layout);
    select_art->addItem(tr("毛玻璃"),1);
    select_art->addItem(tr("柔化"),2);
    select_art->addItem(tr("锐化"),3);
    select_art->addItem(tr("周围像素最大值替换"),4);
    select_art->addItem(tr("周围像素最小值替换"),5);
    select_art->addItem(tr("连环画"),6);
    select_art->addItem(tr("熔铸"),7);
    select_art->addItem(tr("冰冻"),8);
    select_art->addItem(tr("老照片"),9);
    connect(select_art, SIGNAL(activated(const QString &)),this, SLOT(art_process()));

    show_art =  new QLabel(tr("艺术效果--滤镜类型："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(show_art,0,0);
    widget_layout->addWidget(select_art,0,1);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    art_process();
}

//艺术效果处理
void base_process::art_process()
{
    int select_artvarint = select_art->currentData(Qt::UserRole).toInt();

    if(select_artvarint == 1)
    {
        //扩散，用周边的一个随机像素点代替当前的点。
        Mat dst(srcImage.size(),CV_8UC3);
        RNG rng;
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 1; x<srcImage.cols-1; x++)
            {
                int myRand = rng.uniform(0,9);
                srand(time(NULL));
                myRand = rand()%9;
                dstR[3*x] = srcImage.at<uchar>(y-1+myRand/3,3*(x-1+myRand%3));
                dstR[3*x+1]= srcImage.at<uchar>(y-1+myRand/3,3*(x-1+myRand%3)+1);
                dstR[3*x+2]= srcImage.at<uchar>(y-1+myRand/3,3*(x-1+myRand%3)+2);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 2)
    {
        //柔化
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y=2; y<srcImage.rows-2; y++)
        {
            uchar* dstR  = dst.ptr<uchar>(y);
            int br,bg,bb;
            for (int x=2; x<srcImage.cols-2; x++)
            {
                br=0;
                bg=0;
                bb=0;
                for(int i=-2;i<=2;i++)
                {
                    for(int j=-2;j<=2;j++)
                    {
                        br+=srcImage.at<uchar>(y+i,3*(x+j));
                        bg+=srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bb+=srcImage.at<uchar>(y+i,3*(x+j)+2);
                    }
                }
                dstR[3*x]=br/ (4 * 2 * 2 + 4 * 2 + 1);
                dstR[3*x+1]= bg/(4 * 2 * 2 + 4 * 2 + 1);
                dstR[3*x+2]= bb/(4 * 2 * 2 + 4 * 2 + 1);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 3)
    {
        //锐化
        //锐化，搜索周围半径为R范围内的点，
        //差值 = 当前像素点 - 周围像素点的平均值
        //本点 = 当前像素点 + 差值 * 锐化系数S
        int R;
        int S;
        R=2;
        S=2;
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y=2; y<srcImage.rows-2; y++)
        {
            uchar* dstR  = dst.ptr<uchar>(y);
            int br,bg,bb;
            for (int x=2; x<srcImage.cols-2; x++)
            {
                br=0;
                bg=0;
                bb=0;
                for(int i=-2;i<=2;i++)
                {
                    for(int j=-2;j<=2;j++)
                    {
                        if(i == 0 && j == 0) continue;
                        br+=srcImage.at<uchar>(y+i,3*(x+j));
                        bg+=srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bb+=srcImage.at<uchar>(y+i,3*(x+j)+2);
                    }
                }
                br=srcImage.at<uchar>(y,3*x)* (1 + S) - br * S / (4 * R * R + 4 * R+1);
                bg=srcImage.at<uchar>(y,3*x+1) * (1 + S) - bg * S / (4 * R * R + 4 * R+1);
                bb=srcImage.at<uchar>(y,3*x+2) * (1 + S) - bb * S / (4 * R * R + 4 * R+1);
                br = br > 255? 255 : br;
                bg = bg > 255? 255 : bg;
                bb = bb > 255? 255 : bb;
                br = br < 0? 0 : br;
                bg = bg < 0? 0 : bg;
                bb = bb < 0? 0 : bb;
                dstR[3*x]=br;
                dstR[3*x+1]= bg;
                dstR[3*x+2]= bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 4)
    {
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 1; x<srcImage.cols-1; x++)
            {
                int br = srcR[3*x];
                int bg = srcR[3*x+1];
                int bb = srcR[3*x+2];
                for(int i = -1;i<=1;i++)
                {
                    for(int j = -1;j<=1;j++)
                    {
                        br = br > srcImage.at<uchar>(y+i,3*(x+j)+1) ? br:srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bg = bg > srcImage.at<uchar>(y+i,3*(x+j)+1) ? bg:srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bb = bb > srcImage.at<uchar>(y+i,3*(x+j)+1) ? bb:srcImage.at<uchar>(y+i,3*(x+j)+1);
                    }
                }
                dstR[3*x] = br;
                dstR[3*x+1] = bg;
                dstR[3*x+2] = bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 5)
    {
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 1; x<srcImage.cols-1; x++)
            {
                int br = srcR[3*x];
                int bg = srcR[3*x+1];
                int bb = srcR[3*x+2];
                for(int i = -1;i<=1;i++)
                {
                    for(int j = -1;j<=1;j++)
                    {
                        br = br < srcImage.at<uchar>(y+i,3*(x+j)+1) ? br:srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bg = bg < srcImage.at<uchar>(y+i,3*(x+j)+1) ? bg:srcImage.at<uchar>(y+i,3*(x+j)+1);
                        bb = bb < srcImage.at<uchar>(y+i,3*(x+j)+1) ? bb:srcImage.at<uchar>(y+i,3*(x+j)+1);
                    }
                }
                dstR[3*x] = br;
                dstR[3*x+1] = bg;
                dstR[3*x+2] = bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 6)
    {
        //连环画效果。比灰度亮度更明显。
        /*
         * R = |g – b + g + r| * r / 256
         G = |b – g + b + r| * r / 256;
        B = |b – g + b + r| * g / 256;
        完成后再次进行灰度化处理。
        */
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x= 1; x<srcImage.cols-1; x++)
            {
                int br = srcR[3*x];
                int bg = srcR[3*x+1];
                int bb = srcR[3*x+2];

                float brF= (abs(bg - bb + bg + br) * br) >> 8;
                float bgF= (abs(bb - bg + bb + br) * br) >> 8;
                float bbF= (abs(bb - bg + bb + br) * bg) >> 8;

                brF = brF > 255? 255 : brF;
                bgF = bgF > 255? 255 : bgF;
                bbF = bbF > 255? 255 : bbF;
                brF = brF < 0? 0 : brF;
                bgF = bgF < 0? 0 : bgF;
                bbF = bbF < 0? 0 : bbF;
                dstR[3*x] = (uchar)brF;
                dstR[3*x+1] = (uchar)bgF;
                dstR[3*x+2] = (uchar)bbF;
            }
        }
        cvtColor(dst,dst,CV_BGR2GRAY);
        normalize(dst,dst,255,0,CV_MINMAX);
        dst.copyTo(dstImage);
        dstlabel_indexed8_show(dst);
    }
    else if(select_artvarint == 7)
    {
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 1; x<srcImage.cols-1; x++)
            {
                int br = srcR[3*x];
                int bg = srcR[3*x+1];
                int bb = srcR[3*x+2];

                br = br*128/(bg+bb +1);
                bg = bg*128/(br+bb +1);
                bb = bb*128/(bg+br +1);

                br = br > 255? 255 : br;
                bg = bg > 255? 255 : bg;
                bb = bb > 255? 255 : bb;

                br = br < 0? 0 : br;
                bg = bg < 0? 0 : bg;
                bb = bb < 0? 0 : bb;

                dstR[3*x] = br;
                dstR[3*x+1] = bg;
                dstR[3*x+2] = bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 8)
    {
        //冰冻
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 1; y<srcImage.rows-1; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 1; x<srcImage.cols-1; x++)
            {
                float br = srcR[3*x];
                float bg = srcR[3*x+1];
                float bb = srcR[3*x+2];

                br = (br-bg-bb)*3/2;
                bg = (bg-br-bb)*3/2;
                bb = (bb-bg-br)*3/2;

                br = br > 255? 255 : br;
                bg = bg > 255? 255 : bg;
                bb = bb > 255? 255 : bb;

                br = br < 0? -br : br;
                bg = bg < 0? -bg : bg;
                bb = bb < 0? -bb : bb;

                dstR[3*x] = (uchar)br;
                dstR[3*x+1] = (uchar)bg;
                dstR[3*x+2] = (uchar)bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_artvarint == 9)//老照片
    {
        Mat dst_temp(srcImage.size(),CV_8UC3);
        for(int i = 0;i < srcImage.rows;i++)
        {
            uchar *psrc = srcImage.ptr<uchar>(i);
            uchar *pdst = dst_temp.ptr<uchar>(i);
            for(int j = 0;j < srcImage.cols;j++)
            {
                int srcR = psrc[3*j];
                int srcG = psrc[3*j+1];
                int srcB = psrc[3*j+2];

                int dstR = 0.393 * srcR + 0.769 * srcG + 0.189 * srcB;
                int dstG = 0.349 * srcR + 0.686 * srcG + 0.168 * srcB;
                int dstB = 0.272 * srcR + 0.534 * srcG + 0.131 * srcB;
                dstR = dstR>255?255:dstR;
                dstG = dstG>255?255:dstG;
                dstB = dstB>255?255:dstB;
                pdst[3*j] = dstR;
                pdst[3*j+1] = dstG;
                pdst[3*j+2] = dstB;
            }
        }
        dst_temp.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
}

//光线效果
void base_process::on_action_light_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    select_light = new QComboBox(ui->widget_for_layout);
    select_light->addItem(tr("反相"),1);
    select_light->addItem(tr("强光"),2);
    select_light->addItem(tr("黑白底片"),3);
    select_light->addItem(tr("高反差保留"),4);
    connect(select_light, SIGNAL(activated(const QString &)),this, SLOT(light_process()));

    show_light =  new QLabel(tr("光线效果--滤镜类型："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(show_light,0,0);
    widget_layout->addWidget(select_light,0,1);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    light_process();
}

//光线效果处理
void base_process::light_process()
{
    int select_lightvarint = select_light->currentData(Qt::UserRole).toInt();

    if(select_lightvarint == 1)
    {
        dstImage = ~srcImage;
        dstlabel_show(dstImage);
    }
    else if(select_lightvarint == 2)
    {
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 0; y<srcImage.rows; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 0; x<srcImage.cols; x++)
            {
                int br=srcR[3*x];
                int bg=srcR[3*x+1];
                int bb=srcR[3*x+2];
                br = br > 127.5? br+(255-br)*(br-127.5)/127.5:br*br/127.5;
                br = br > 255?255:br;
                br = br < 0?0:br;
                bg = bg > 127.5? bg+(255-bg)*(bg-127.5)/127.5:bg*bg/127.5;
                bg = bg > 255?255:bg;
                bg = bg < 0?0:bg;
                bb = bb > 127.5? bb+(255-bb)*(bb-127.5)/127.5:bb*bb/127.5;
                bb = bb > 255?255:bb;
                bb = bb < 0?0:bb;
                dstR[3*x] = br;
                dstR[3*x+1] = bg;
                dstR[3*x+2] = bb;
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(select_lightvarint == 3)
    {
        //黑白底片
        Mat dst(srcImage.size(),CV_8UC3);
        for (int y = 0; y<srcImage.rows; y++)
        {
            uchar* srcR  = srcImage.ptr<uchar>(y);
            uchar* dstR  = dst.ptr<uchar>(y);
            for (int x = 0; x<srcImage.cols; x++)
            {
                int br = srcR[3*x];
                int bg = srcR[3*x+1];
                int bb = srcR[3*x+2];

                dstR[3*x] = 255 - br;
                dstR[3*x+1] = 255 - bg;
                dstR[3*x+2] = 255 - bb;
            }
        }
        cv::cvtColor(dst,dstImage,CV_RGB2GRAY);
        dstImage.copyTo(dst);
        dstlabel_indexed8_show(dstImage);
    }
    else if(select_lightvarint == 4)
    {
        Mat dst(srcImage.size(),CV_8UC3);
        Mat avg;
        blur(srcImage,avg,Size(5,5));
        float tmp;

        for (int y = 0; y<srcImage.rows; y++)
        {
            uchar* imgP=srcImage.ptr<uchar>(y);
            uchar* avgP=avg.ptr<uchar>(y);
            uchar* dstP=dst.ptr<uchar>(y);
            for (int x=0;x<srcImage.cols;x++)
            {
                float r0 = abs((float)imgP[3*x]-(float)avgP[3*x])/128;
                tmp = abs( ((float)imgP[3*x]  )*r0 + 128*(1-r0) );
                tmp=tmp>255?255:tmp;
                tmp=tmp<0?0:tmp;
                dstP[3*x]=(uchar)(tmp);

                float r1 = abs((float)imgP[3*x+1]-(float)avgP[3*x+1])/128;
                tmp = (uchar)abs( ((float)imgP[3*x+1])*r1 + 128*(1-r1) );
                tmp=tmp>255?255:tmp;
                tmp=tmp<0?0:tmp;
                dstP[3*x+1]=(uchar)(tmp);

                float r2 = abs((float)imgP[3*x+2]-(float)avgP[3*x+2])/128;
                tmp = (uchar)abs( ((float)imgP[3*x+2])*r2 + 128*(1-r2) );
                tmp=tmp>255?255:tmp;
                tmp=tmp<0?0:tmp;
                dstP[3*x+2]=(uchar)(tmp);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
}

//雕刻效果
void base_process::on_action_diaoke_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    select_diaoke_func = new QComboBox(ui->widget_for_layout);
    select_diaoke_func->addItem(tr("凸雕"),1);
    select_diaoke_func->addItem(tr("凹雕"),2);
    connect(select_diaoke_func, SIGNAL(activated(const QString &)),this, SLOT(diaoke_process()));

    diaoke_canshu_SpinBox = new QSpinBox(ui->widget_for_layout);
    diaoke_canshu_SpinBox->setMinimum(-200);
    diaoke_canshu_SpinBox->setMaximum(300);
    diaoke_canshu_SpinBox->setValue(1);//设置一个默认值

    diaoke_canshu_slider = new QSlider(ui->widget_for_layout);
    diaoke_canshu_slider->setOrientation(Qt::Horizontal);//水平方向
    diaoke_canshu_slider->setMinimum(-200);
    diaoke_canshu_slider->setMaximum(300);
    diaoke_canshu_slider->setSingleStep(10);//步长 动一下移动的距离
    diaoke_canshu_slider->setTickInterval(10); // 设置刻度间隔
    diaoke_canshu_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    diaoke_canshu_slider->setValue(1);//设置一个默认值

    connect(diaoke_canshu_slider, SIGNAL(valueChanged(int)), diaoke_canshu_SpinBox, SLOT(setValue(int)));
    connect(diaoke_canshu_SpinBox, SIGNAL(valueChanged(int)), diaoke_canshu_slider, SLOT(setValue(int)));

    connect(diaoke_canshu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(diaoke_process()));
    connect(diaoke_canshu_slider, SIGNAL(valueChanged(int)), this, SLOT(diaoke_process()));

    diaoke_lab = new QLabel(tr("设置亮度："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(select_diaoke_func,0,0);
    widget_layout->addWidget(diaoke_lab,0,1);
    widget_layout->addWidget(diaoke_canshu_SpinBox,0,2);
    widget_layout->addWidget(diaoke_canshu_slider,0,3);
    widget_layout->setAlignment(Qt::AlignHCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    diaoke_process();
}

//雕刻效果处理
void base_process::diaoke_process()
{
    int select_diaoke_funcvarint = select_diaoke_func->currentData(Qt::UserRole).toInt();

    int canshu = diaoke_canshu_SpinBox->text().toInt();

    if(select_diaoke_funcvarint == 1)
    {
        //凸雕
        Mat dst(srcImage.size(),CV_8UC3);
        for (int i = 1; i<srcImage.rows-1; i++)
        {
            uchar *currentBefore = srcImage.ptr<uchar>(i-1);
            uchar *currentNext = srcImage.ptr<uchar>(i+1);
            uchar *dstLine = dst.ptr<uchar>(i);
            for (int j = 1; j<srcImage.cols-1; j++)
            {
                for (int k = 0; k<3; k++)
                {
                    int tmp0 = currentNext[3*(j+1)+k]-currentBefore[3*(j-1)+k] + canshu;
                    if (tmp0 < 0)
                        dstLine[3*j+k] = 0;
                    else if(tmp0>255)
                        dstLine[3*j+k] = 255;
                    else
                        dstLine[3*j+k] = tmp0;
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else
    {
        //凹雕
        Mat dst(srcImage.size(),CV_8UC3);
        for (int i = 1; i<srcImage.rows-1; i++)
        {
            uchar *currentBefore = srcImage.ptr<uchar>(i-1);
            uchar *currentNext = srcImage.ptr<uchar>(i+1);
            uchar *dstLine = dst.ptr<uchar>(i);
            for (int j = 1; j<srcImage.cols-1; j++)
            {
                for (int k = 0; k<3; k++)
                {
                    int tmp0 = currentNext[3*(j-1)+k]-currentBefore[3*(j+1)+k] + canshu;
                    if (tmp0 < 0)
                        dstLine[3*j+k] = 0;
                    else if(tmp0>255)
                        dstLine[3*j+k] = 255;
                    else
                        dstLine[3*j+k] = tmp0;
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
}

//特殊效果
void base_process::on_action_teshuxiaoguo_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    teshuxiaoguo_select = new QComboBox(ui->widget_for_layout);
    teshuxiaoguo_select->addItem(tr("凸透镜"),1);
    teshuxiaoguo_select->addItem(tr("挤压"),2);
    teshuxiaoguo_select->addItem(tr("波浪"),3);
    teshuxiaoguo_select->addItem(tr("漩涡"),4);
    teshuxiaoguo_select->addItem(tr("径向模糊-缩放（计算量大，慎用）"),5);
    teshuxiaoguo_select->addItem(tr("径向模糊-旋转（计算量大，慎用）"),6);
    connect(teshuxiaoguo_select, SIGNAL(activated(const QString &)),this, SLOT(teshuxiaoguo_process()));

    teshuxiaoguo_canshu_SpinBox = new QSpinBox(ui->widget_for_layout);
    teshuxiaoguo_canshu_SpinBox->setMinimum(1);
    teshuxiaoguo_canshu_SpinBox->setMaximum(300);
    teshuxiaoguo_canshu_SpinBox->setValue(200);//设置一个默认值

    teshuxiaoguo_canshu_slider = new QSlider(ui->widget_for_layout);
    teshuxiaoguo_canshu_slider->setOrientation(Qt::Horizontal);//水平方向
    teshuxiaoguo_canshu_slider->setMinimum(1);
    teshuxiaoguo_canshu_slider->setMaximum(300);
    teshuxiaoguo_canshu_slider->setSingleStep(10);//步长 动一下移动的距离
    teshuxiaoguo_canshu_slider->setTickInterval(20); // 设置刻度间隔
    teshuxiaoguo_canshu_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    teshuxiaoguo_canshu_slider->setValue(200);//设置一个默认值

    connect(teshuxiaoguo_canshu_slider, SIGNAL(valueChanged(int)), teshuxiaoguo_canshu_SpinBox, SLOT(setValue(int)));
    connect(teshuxiaoguo_canshu_SpinBox, SIGNAL(valueChanged(int)), teshuxiaoguo_canshu_slider, SLOT(setValue(int)));

    connect(teshuxiaoguo_canshu_slider, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));
    connect(teshuxiaoguo_canshu_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));

    teshuxiaoguo_lab = new QLabel(tr("凸透镜放大力度："),ui->widget_for_layout);
    teshuxiaoguo_lab_x = new QLabel(tr("操作中心点--X轴："),ui->widget_for_layout);
    teshuxiaoguo_lab_y = new QLabel(tr("操作中心点--Y轴："),ui->widget_for_layout);

    teshuxiaoguo_center_x_SpinBox = new QSpinBox(ui->widget_for_layout);
    teshuxiaoguo_center_x_SpinBox->setMinimum(1);
    teshuxiaoguo_center_x_SpinBox->setMaximum(99);
    teshuxiaoguo_center_x_SpinBox->setValue(50);//设置一个默认值
    teshuxiaoguo_center_x_SpinBox->setPrefix("0.");//前缀

    teshuxiaoguo_center_x_slider = new QSlider(ui->widget_for_layout);
    teshuxiaoguo_center_x_slider->setOrientation(Qt::Horizontal);//水平方向
    teshuxiaoguo_center_x_slider->setMinimum(1);
    teshuxiaoguo_center_x_slider->setMaximum(99);
    teshuxiaoguo_center_x_slider->setSingleStep(10);//步长 动一下移动的距离
    teshuxiaoguo_center_x_slider->setTickInterval(5); // 设置刻度间隔
    teshuxiaoguo_center_x_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    teshuxiaoguo_center_x_slider->setValue(50);//设置一个默认值

    connect(teshuxiaoguo_center_x_slider, SIGNAL(valueChanged(int)), teshuxiaoguo_center_x_SpinBox, SLOT(setValue(int)));
    connect(teshuxiaoguo_center_x_SpinBox, SIGNAL(valueChanged(int)), teshuxiaoguo_center_x_slider, SLOT(setValue(int)));

    connect(teshuxiaoguo_center_x_slider, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));
    connect(teshuxiaoguo_center_x_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));

    teshuxiaoguo_center_y_SpinBox = new QSpinBox(ui->widget_for_layout);
    teshuxiaoguo_center_y_SpinBox->setMinimum(1);
    teshuxiaoguo_center_y_SpinBox->setMaximum(99);
    teshuxiaoguo_center_y_SpinBox->setValue(50);//设置一个默认值
    teshuxiaoguo_center_y_SpinBox->setPrefix("0.");//前缀

    teshuxiaoguo_center_y_slider = new QSlider(ui->widget_for_layout);
    teshuxiaoguo_center_y_slider->setOrientation(Qt::Horizontal);//水平方向
    teshuxiaoguo_center_y_slider->setMinimum(1);
    teshuxiaoguo_center_y_slider->setMaximum(99);
    teshuxiaoguo_center_y_slider->setSingleStep(10);//步长 动一下移动的距离
    teshuxiaoguo_center_y_slider->setTickInterval(5); // 设置刻度间隔
    teshuxiaoguo_center_y_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    teshuxiaoguo_center_y_slider->setValue(50);//设置一个默认值

    connect(teshuxiaoguo_center_y_slider, SIGNAL(valueChanged(int)), teshuxiaoguo_center_y_SpinBox, SLOT(setValue(int)));
    connect(teshuxiaoguo_center_y_SpinBox, SIGNAL(valueChanged(int)), teshuxiaoguo_center_y_slider, SLOT(setValue(int)));

    connect(teshuxiaoguo_center_y_slider, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));
    connect(teshuxiaoguo_center_y_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(teshuxiaoguo_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(teshuxiaoguo_select,0,0,1,2);
    widget_layout->addWidget(teshuxiaoguo_lab,1,0);
    widget_layout->addWidget(teshuxiaoguo_canshu_SpinBox,1,1);
    widget_layout->addWidget(teshuxiaoguo_canshu_slider,1,2,1,5);
    widget_layout->addWidget(teshuxiaoguo_lab_x,2,0);
    widget_layout->addWidget(teshuxiaoguo_center_x_SpinBox,2,1);
    widget_layout->addWidget(teshuxiaoguo_center_x_slider,2,2,1,5);
    widget_layout->addWidget(teshuxiaoguo_lab_y,3,0);
    widget_layout->addWidget(teshuxiaoguo_center_y_SpinBox,3,1);
    widget_layout->addWidget(teshuxiaoguo_center_y_slider,3,2,1,5);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    teshuxiaoguo_process();
}

//特殊效果
void base_process::teshuxiaoguo_process()
{
    if(teshuxiaoguo_center_x_slider->value() >=10)
    {
        teshuxiaoguo_center_x_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        teshuxiaoguo_center_x_SpinBox->setPrefix("0.0");//前缀
    }

    if(teshuxiaoguo_center_y_slider->value() >=10)
    {
        teshuxiaoguo_center_y_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        teshuxiaoguo_center_y_SpinBox->setPrefix("0.0");//前缀
    }

    int teshuxiaoguo_selectvarint = teshuxiaoguo_select->currentData(Qt::UserRole).toInt();
    int width = srcImage.cols;
    int heigh = srcImage.rows;
    //Point center(width/2,heigh/2);
    //操作中心
    Point center(int(width * teshuxiaoguo_center_x_SpinBox->text().toCaseFolded().toDouble()),
                 int(heigh * teshuxiaoguo_center_y_SpinBox->text().toCaseFolded().toDouble()));

    if(teshuxiaoguo_selectvarint == 1)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(sqrtf(width*width+heigh*heigh)/2);
        teshuxiaoguo_canshu_slider->setMaximum(sqrtf(width*width+heigh*heigh)/2);
        teshuxiaoguo_canshu_slider->setTickInterval(sqrtf(width*width+heigh*heigh)/80); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("凸透镜放大力度："));
    }
    else if(teshuxiaoguo_selectvarint == 2)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(30);
        teshuxiaoguo_canshu_slider->setMaximum(30);
        teshuxiaoguo_canshu_slider->setTickInterval(2); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("挤压力度："));
    }
    else if(teshuxiaoguo_selectvarint == 3)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(100);
        teshuxiaoguo_canshu_slider->setMaximum(100);
        teshuxiaoguo_canshu_SpinBox->setMinimum(-100);
        teshuxiaoguo_canshu_slider->setMinimum(-100);
        teshuxiaoguo_canshu_slider->setTickInterval(4); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("波浪幅度："));
    }
    else if(teshuxiaoguo_selectvarint == 4)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(100);
        teshuxiaoguo_canshu_slider->setMaximum(100);
        teshuxiaoguo_canshu_SpinBox->setMinimum(-100);
        teshuxiaoguo_canshu_slider->setMinimum(-100);
        teshuxiaoguo_canshu_slider->setTickInterval(4); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("旋涡幅度："));
    }
    else if(teshuxiaoguo_selectvarint == 5)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(40);
        teshuxiaoguo_canshu_slider->setMaximum(40);
        teshuxiaoguo_canshu_SpinBox->setMinimum(10);
        teshuxiaoguo_canshu_slider->setMinimum(10);
        teshuxiaoguo_canshu_slider->setTickInterval(4); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("模糊力度："));
    }
    else if(teshuxiaoguo_selectvarint == 6)
    {
        teshuxiaoguo_canshu_SpinBox->setMaximum(40);
        teshuxiaoguo_canshu_slider->setMaximum(40);
        teshuxiaoguo_canshu_SpinBox->setMinimum(10);
        teshuxiaoguo_canshu_slider->setMinimum(10);
        teshuxiaoguo_canshu_slider->setTickInterval(4); // 设置刻度间隔
        teshuxiaoguo_lab->setText(tr("模糊力度："));
    }

    if(teshuxiaoguo_selectvarint == 1)
    {
        //凸透镜放大
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        int R1 = teshuxiaoguo_canshu_SpinBox->value();//直接关系到放大的力度,与R1成正比;
        for (int y=0; y<heigh; y++)
        {
            uchar *dst_p = dst.ptr<uchar>(y);
            for (int x=0; x<width; x++)
            {
                int dis = norm(Point(x,y)-center);
                if (dis<R1)
                {
                    int newX = (x-center.x)*dis/R1+center.x;
                    int newY = (y-center.y)*dis/R1+center.y;

                    dst_p[3*x] = srcImage.at<uchar>(newY,newX*3);
                    dst_p[3*x+1] = srcImage.at<uchar>(newY,newX*3+1);
                    dst_p[3*x+2] = srcImage.at<uchar>(newY,newX*3+2);
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(teshuxiaoguo_selectvarint == 2)
    {
        //挤压
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        for (int y=0; y<heigh; y++)
        {
            uchar *dst_p = dst.ptr<uchar>(y);
            for (int x=0; x<width; x++)
            {
                double theta = atan2((double)(y-center.y),(double)(x-center.x));

                int R2 = sqrtf(norm(Point(x,y)-center))*teshuxiaoguo_canshu_SpinBox->value(); //直接关系到挤压的力度，与R2成反比;

                int newX = center.x+(int)(R2*cos(theta));
                int newY = center.y+(int)(R2*sin(theta));

                if(newX < 0)
                {
                    newX = 0;
                }
                else if(newX >= width)
                {
                    newX = width-1;
                }

                if(newY < 0)
                {
                    newY = 0;
                }
                else if(newY >= heigh)
                {
                    newY = heigh-1;
                }

                dst_p[3*x] = srcImage.at<uchar>(newY,newX*3);
                dst_p[3*x+1] = srcImage.at<uchar>(newY,newX*3+1);
                dst_p[3*x+2] = srcImage.at<uchar>(newY,newX*3+2);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(teshuxiaoguo_selectvarint == 3)
    {
        //波浪
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        int N = teshuxiaoguo_canshu_SpinBox->value(); //幅度
        double pi = 3.1415926;

        float new_x,new_y;
        float p,q,x1,y1,x0,y0;

        for (int y=0; y<heigh; y++)
        {
            for (int x=0; x<width; x++)
            {
                y0 = center.y-y;
                x0 = x-center.x;
                new_x = N * sin(2*pi*y0/128.0) + x0;
                new_y = N * cos(2*pi*x0/128.0) + y0;
                new_x = center.x + new_x;
                new_y = center.y - new_y;

                if(new_x < 0)
                {
                    new_x = 0;
                }
                if(new_x >= width-1)
                {
                    new_x = width-2;
                }
                if(new_y < 0)
                {
                    new_y = 0;
                }
                if(new_y >= heigh-1)
                {
                    new_y = heigh-2;
                }

                x1 = (int)new_x;
                y1 = (int)new_y;

                p = new_x-x1;
                q = new_y-y1;

                for (int k=0; k<3; k++)
                {
                    dst.at<Vec3b>(y, x)[k] = (1-p) * (1-q) * srcImage.at<Vec3b>(y1, x1)[k]+
                                             (p)   * (1-q) * srcImage.at<Vec3b>(y1,x1+1)[k]+
                                             (1-p) *  (q)  * srcImage.at<Vec3b>(y1+1,x1)[k]+
                                             (p)   *  (q)  * srcImage.at<Vec3b>(y1+1,x1+1)[k];
                }
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(teshuxiaoguo_selectvarint == 4)
    {
        //漩涡
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        double pi = 3.1415926;
        double Para = teshuxiaoguo_canshu_SpinBox->value();

        Mat src1u[3];
        split(srcImage,src1u);

        for (int y=0; y<heigh; y++)
        {
            uchar* dst_P = dst.ptr<uchar>(y);
            for(int x = 0; x < width; x++)
            {
                int R = norm(Point(x,y) - center);
                double angle = atan2((double)(y-center.y),(double)(x-center.x));
                double delta = pi*Para/sqrtf(R+1);
                int newX = R*cos(angle+delta) + center.x;
                int newY = R*sin(angle+delta) + center.y;

                if(newX < 0)
                {
                    newX = 0;
                }
                if(newX > width-1)
                {
                    newX = width-1;
                }
                if(newY < 0)
                {
                    newY = 0;
                }
                if(newY > heigh-1)
                {
                    newY = heigh-1;
                }

                dst_P[3*x] = src1u[0].at<uchar>(newY,newX);
                dst_P[3*x+1] = src1u[1].at<uchar>(newY,newX);
                dst_P[3*x+2] = src1u[2].at<uchar>(newY,newX);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(teshuxiaoguo_selectvarint == 5)//径向模糊-缩放
    {
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        Mat src1u[3];
        split(srcImage,src1u);
        int num = teshuxiaoguo_canshu_SpinBox->value();

        for (int y=0; y<heigh; y++)
        {
            uchar *imgP  = dst.ptr<uchar>(y);

            for (int x=0; x<width; x++)
            {
                int R = norm(Point(x,y)-center);
                double angle = atan2((double)(y-center.y),(double)(x-center.x));

                int tmp0=0,tmp1=0,tmp2=0;

                for (int i=0;i<num;i++)
                {
                    int tmpR = (R-i)>0?(R-i):0;

                    int newX = tmpR*cos(angle) + center.x;
                    int newY = tmpR*sin(angle) + center.y;

                    if(newX<0)
                    {
                        newX=0;
                    }
                    if(newX>width-1)
                    {
                        newX=width-1;
                    }
                    if(newY<0)
                    {
                        newY=0;
                    }
                    if(newY>heigh-1)
                    {
                        newY=heigh-1;
                    }

                    tmp0 += src1u[0].at<uchar>(newY,newX);
                    tmp1 += src1u[1].at<uchar>(newY,newX);
                    tmp2 += src1u[2].at<uchar>(newY,newX);

                }
                imgP[3*x]=(uchar)(tmp0/num);
                imgP[3*x+1]=(uchar)(tmp1/num);
                imgP[3*x+2]=(uchar)(tmp2/num);
            }

        }

        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
    else if(teshuxiaoguo_selectvarint == 6)//径向模糊-旋转
    {
        Mat dst(srcImage.size(),CV_8UC3);
        srcImage.copyTo(dst);

        Mat src1u[3];
        split(srcImage,src1u);
        int num = teshuxiaoguo_canshu_SpinBox->value();

        for (int y=0; y<heigh; y++)
        {
            uchar *imgP  = dst.ptr<uchar>(y);

            for (int x=0; x<width; x++)
            {
                int R = norm(Point(x,y)-center);
                double angle = atan2((double)(y-center.y),(double)(x-center.x));

                int tmp0=0,tmp1=0,tmp2=0;

                for (int i=0;i<num;i++)	//均值力度;
                {
                    angle+=0.01;//0.01控制变化频率，步长

                    int newX = R*cos(angle) + center.x;
                    int newY = R*sin(angle) + center.y;

                    if(newX<0)
                    {
                        newX=0;
                    }
                    if(newX>width-1)
                    {
                        newX=width-1;
                    }
                    if(newY<0)
                    {
                        newY=0;
                    }
                    if(newY>heigh-1)
                    {
                        newY=heigh-1;
                    }

                    tmp0 += src1u[0].at<uchar>(newY,newX);
                    tmp1 += src1u[1].at<uchar>(newY,newX);
                    tmp2 += src1u[2].at<uchar>(newY,newX);
                }
                imgP[3*x]=(uchar)(tmp0/num);
                imgP[3*x+1]=(uchar)(tmp1/num);
                imgP[3*x+2]=(uchar)(tmp2/num);
            }
        }
        dst.copyTo(dstImage);
        dstlabel_show(dstImage);
    }
}

//轮廓检测
void base_process::on_action_lunkuojiance_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj(); 

    canny_ksize_lunkuojiance = new QComboBox(ui->widget_for_layout);
    canny_ksize_lunkuojiance->addItem(tr("Canny算子核尺寸：3"),3);
    canny_ksize_lunkuojiance->addItem(tr("Canny算子核尺寸：5"),5);
    canny_ksize_lunkuojiance->addItem(tr("Canny算子核尺寸：7"),7);
    connect(canny_ksize_lunkuojiance, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    L2gradient_lunkuojiance = new QComboBox(ui->widget_for_layout);
    L2gradient_lunkuojiance->addItem(tr("Canny算子梯度强度：L1范数"),0);
    L2gradient_lunkuojiance->addItem(tr("Canny算子梯度强度：L2范数"),1);
    connect(L2gradient_lunkuojiance, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    lunkuojiance_SpinBox_l = new QSpinBox(ui->widget_for_layout);
    lunkuojiance_SpinBox_l->setMinimum(1);
    lunkuojiance_SpinBox_l->setMaximum(254);
    lunkuojiance_SpinBox_l->setValue(150);//设置一个默认值

    lunkuojiance_slider_l = new QSlider(ui->widget_for_layout);
    lunkuojiance_slider_l->setOrientation(Qt::Horizontal);//水平方向
    lunkuojiance_slider_l->setMinimum(1);
    lunkuojiance_slider_l->setMaximum(254);
    lunkuojiance_slider_l->setSingleStep(10);//步长 动一下移动的距离
    lunkuojiance_slider_l->setTickInterval(10); // 设置刻度间隔
    lunkuojiance_slider_l->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    lunkuojiance_slider_l->setValue(150);//设置一个默认值

    connect(lunkuojiance_slider_l, SIGNAL(valueChanged(int)), lunkuojiance_SpinBox_l, SLOT(setValue(int)));
    connect(lunkuojiance_SpinBox_l, SIGNAL(valueChanged(int)), lunkuojiance_slider_l, SLOT(setValue(int)));

    connect(lunkuojiance_slider_l, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));
    connect(lunkuojiance_SpinBox_l, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_SpinBox_h = new QSpinBox(ui->widget_for_layout);
    lunkuojiance_SpinBox_h->setMinimum(2);
    lunkuojiance_SpinBox_h->setMaximum(1000);
    lunkuojiance_SpinBox_h->setValue(180);//设置一个默认值

    lunkuojiance_slider_h = new QSlider(ui->widget_for_layout);
    lunkuojiance_slider_h->setOrientation(Qt::Horizontal);//水平方向
    lunkuojiance_slider_h->setMinimum(2);
    lunkuojiance_slider_h->setMaximum(1000);
    lunkuojiance_slider_h->setSingleStep(20);//步长 动一下移动的距离
    lunkuojiance_slider_h->setTickInterval(20); // 设置刻度间隔
    lunkuojiance_slider_h->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    lunkuojiance_slider_h->setValue(180);//设置一个默认值

    connect(lunkuojiance_slider_h, SIGNAL(valueChanged(int)), lunkuojiance_SpinBox_h, SLOT(setValue(int)));
    connect(lunkuojiance_SpinBox_h, SIGNAL(valueChanged(int)), lunkuojiance_slider_h, SLOT(setValue(int)));

    connect(lunkuojiance_slider_h, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));
    connect(lunkuojiance_SpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_RETR_QComboBox = new QComboBox(ui->widget_for_layout);
    lunkuojiance_RETR_QComboBox->addItem(tr("RETR_EXTERNAL"),RETR_EXTERNAL);
    lunkuojiance_RETR_QComboBox->addItem(tr("RETR_LIST"),RETR_LIST);
    lunkuojiance_RETR_QComboBox->addItem(tr("RETR_CCOMP"),RETR_CCOMP);
    lunkuojiance_RETR_QComboBox->addItem(tr("RETR_TREE"),RETR_TREE);
    connect(lunkuojiance_RETR_QComboBox, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    lunkuojiance_APPROX_QComboBox = new QComboBox(ui->widget_for_layout);
    lunkuojiance_APPROX_QComboBox->addItem(tr("CHAIN_APPROX_NONE"),CHAIN_APPROX_NONE);
    lunkuojiance_APPROX_QComboBox->addItem(tr("CHAIN_APPROX_SIMPLE"),CHAIN_APPROX_SIMPLE);
    lunkuojiance_APPROX_QComboBox->addItem(tr("CHAIN_APPROX_TC89_L1"),CHAIN_APPROX_TC89_L1);
    lunkuojiance_APPROX_QComboBox->addItem(tr("CHAIN_APPROX_TC89_KCOS"),CHAIN_APPROX_TC89_KCOS);
    connect(lunkuojiance_APPROX_QComboBox, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    lunkuojiance_color_QComboBox = new QComboBox(ui->widget_for_layout);
    lunkuojiance_color_QComboBox->addItem(tr("黑白轮廓"),0);
    lunkuojiance_color_QComboBox->addItem(tr("彩色轮廓"),1);
    lunkuojiance_color_QComboBox->addItem(tr("黑白轮廓色，彩色绘制色"),2);
    connect(lunkuojiance_color_QComboBox, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    lunkuojiance_lunkuocuxi_QComboBox = new QComboBox(ui->widget_for_layout);
    lunkuojiance_lunkuocuxi_QComboBox->addItem(tr("轮廓线粗细：1"),1);
    lunkuojiance_lunkuocuxi_QComboBox->addItem(tr("轮廓线粗细：2"),2);
    lunkuojiance_lunkuocuxi_QComboBox->addItem(tr("轮廓线粗细：3"),3);
    lunkuojiance_lunkuocuxi_QComboBox->addItem(tr("轮廓填充"),-1);
    connect(lunkuojiance_lunkuocuxi_QComboBox, SIGNAL(activated(const QString &)),this, SLOT(lunkuojiance_process()));

    lunkuojiance_isdraw_ju = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_isdraw_ju->setText(tr("绘制图像矩"));
    connect(lunkuojiance_isdraw_ju, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_hull = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_hull->setText(tr("绘制轮廓凸包"));
    connect(lunkuojiance_draw_hull, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_poly = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_poly->setText(tr("绘制轮廓逼近多边形"));
    connect(lunkuojiance_draw_poly, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_in_src = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_in_src->setText(tr("在源图上绘制"));
    connect(lunkuojiance_draw_in_src, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_rectangle = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_rectangle->setText(tr("绘制轮廓外接矩形"));
    connect(lunkuojiance_draw_rectangle, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_circle = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_circle->setText(tr("绘制轮廓外接圆"));
    connect(lunkuojiance_draw_circle, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_AreaRect = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_AreaRect->setText(tr("绘制轮廓可旋转外接矩形"));
    connect(lunkuojiance_draw_AreaRect, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    lunkuojiance_draw_Ellipse = new QCheckBox(ui->widget_for_layout);
    lunkuojiance_draw_Ellipse->setText(tr("绘制轮廓外接椭圆"));
    connect(lunkuojiance_draw_Ellipse, SIGNAL(stateChanged(int)), this, SLOT(lunkuojiance_process()));

    show_info_lunkuojiance = new QLabel(tr(" 注意：处理大图会比较耗时"),ui->widget_for_layout);
    show_input_lunkuojiance_thresh1 = new QLabel(tr("Canny算子低阈值(1~254)："),ui->widget_for_layout);
    show_input_lunkuojiance_thresh2 = new QLabel(tr("Canny算子高阈值(2~1000)："),ui->widget_for_layout);
    show_lunkuojiance_mianji_shaixuan = new QLabel(tr("轮廓按面积筛选："),ui->widget_for_layout);

    lunkuojiance_mianji_shaixuan_SpinBox = new QSpinBox(ui->widget_for_layout);
    lunkuojiance_mianji_shaixuan_SpinBox->setMinimum(1);
    lunkuojiance_mianji_shaixuan_SpinBox->setMaximum(10000);
    lunkuojiance_mianji_shaixuan_SpinBox->setValue(1);//设置一个默认值

    lunkuojiance_mianji_shaixuan_slider = new QSlider(ui->widget_for_layout);
    lunkuojiance_mianji_shaixuan_slider->setOrientation(Qt::Horizontal);//水平方向
    lunkuojiance_mianji_shaixuan_slider->setMinimum(1);
    lunkuojiance_mianji_shaixuan_slider->setMaximum(10000);
    lunkuojiance_mianji_shaixuan_slider->setSingleStep(200);//步长 动一下移动的距离
    lunkuojiance_mianji_shaixuan_slider->setTickInterval(400); // 设置刻度间隔
    lunkuojiance_mianji_shaixuan_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    lunkuojiance_mianji_shaixuan_slider->setValue(1);//设置一个默认值

    connect(lunkuojiance_mianji_shaixuan_slider, SIGNAL(valueChanged(int)), lunkuojiance_mianji_shaixuan_SpinBox, SLOT(setValue(int)));
    connect(lunkuojiance_mianji_shaixuan_SpinBox, SIGNAL(valueChanged(int)), lunkuojiance_mianji_shaixuan_slider, SLOT(setValue(int)));

    connect(lunkuojiance_mianji_shaixuan_slider, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));
    connect(lunkuojiance_mianji_shaixuan_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(lunkuojiance_process()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(canny_ksize_lunkuojiance,0,0);
    widget_layout->addWidget(L2gradient_lunkuojiance,0,1);
    widget_layout->addWidget(show_info_lunkuojiance,0,2,1,2);
    widget_layout->addWidget(show_input_lunkuojiance_thresh1,1,0);
    widget_layout->addWidget(lunkuojiance_SpinBox_l,1,1);
    widget_layout->addWidget(lunkuojiance_slider_l,1,2,1,3);
    widget_layout->addWidget(show_input_lunkuojiance_thresh2,2,0);
    widget_layout->addWidget(lunkuojiance_SpinBox_h,2,1);
    widget_layout->addWidget(lunkuojiance_slider_h,2,2,1,3);
    widget_layout->addWidget(show_lunkuojiance_mianji_shaixuan,3,0);
    widget_layout->addWidget(lunkuojiance_mianji_shaixuan_SpinBox,3,1);
    widget_layout->addWidget(lunkuojiance_mianji_shaixuan_slider,3,2,1,3);
    widget_layout->addWidget(lunkuojiance_RETR_QComboBox,4,0);
    widget_layout->addWidget(lunkuojiance_APPROX_QComboBox,4,1);
    widget_layout->addWidget(lunkuojiance_lunkuocuxi_QComboBox,4,2);
    widget_layout->addWidget(lunkuojiance_color_QComboBox,4,3);
    widget_layout->addWidget(lunkuojiance_isdraw_ju,5,0);
    widget_layout->addWidget(lunkuojiance_draw_hull,5,1);
    widget_layout->addWidget(lunkuojiance_draw_poly,5,2);
    widget_layout->addWidget(lunkuojiance_draw_in_src,5,3);
    widget_layout->addWidget(lunkuojiance_draw_rectangle,6,0);
    widget_layout->addWidget(lunkuojiance_draw_AreaRect,6,1);
    widget_layout->addWidget(lunkuojiance_draw_circle,6,2);
    widget_layout->addWidget(lunkuojiance_draw_Ellipse,6,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    lunkuojiance_process();
}

//轮廓检测
void base_process::lunkuojiance_process()
{
    int cannylowthreshint = lunkuojiance_SpinBox_l->text().toInt();
    int cannythreshdif = cannylowthreshint + 1;

    QString str = "Canny算子高阈值(" + QString("%1").arg(cannythreshdif) + "~1000)：";

    show_input_lunkuojiance_thresh2->setText(str);

    lunkuojiance_slider_h->setMinimum(cannythreshdif);
    lunkuojiance_SpinBox_h->setMinimum(cannythreshdif);

    int line_cuxi = lunkuojiance_lunkuocuxi_QComboBox->currentData(Qt::UserRole).toInt();

    bool isL2gradient = false;
    int l2gradientvarint = L2gradient_lunkuojiance->currentData(Qt::UserRole).toInt();
    if(l2gradientvarint == 0)
    {
        isL2gradient = false;
    }
    else
    {
        isL2gradient = true;
    }

    Mat grayImage,cannygrayImage;
    cvtColor(srcImage,grayImage,CV_BGR2GRAY);

    Canny(grayImage,
          cannygrayImage,
          cannylowthreshint,
          lunkuojiance_SpinBox_h->text().toInt(),
          canny_ksize_lunkuojiance->currentData(Qt::UserRole).toInt(),//核尺寸
          isL2gradient);

    vector<vector<Point> > g_vContours_front;//筛选之前的轮廓
    vector<vector<Point> > g_vContours;//筛选之后的轮廓
    vector<Vec4i> g_vHierarchy;
    Mat cannyfindContours;
    cannygrayImage.copyTo(cannyfindContours);

    findContours(cannyfindContours,
                 g_vContours_front,
                 g_vHierarchy,
                 lunkuojiance_RETR_QComboBox->currentData(Qt::UserRole).toInt(),
                 lunkuojiance_APPROX_QComboBox->currentData(Qt::UserRole).toInt());

    for (size_t i = 0; i < g_vContours_front.size(); i++)//轮廓筛选 面积小于阈值的剔除
    {
         if (fabs(contourArea(Mat(g_vContours_front[i]))) > lunkuojiance_mianji_shaixuan_SpinBox->value())
         {
             g_vContours.push_back(g_vContours_front[i]);
         }
    }

    vector<Moments> mu(g_vContours.size());   //根据轮廓计算矩
    for (unsigned int i = 0; i < g_vContours.size(); i++)
    {
        mu[i] = moments(g_vContours[i], false);
    }

    vector<Point2f> mc(g_vContours.size());  //根据轮廓计算中心矩
    for (unsigned int i = 0; i < g_vContours.size(); i++)
    {
        mc[i] = Point2f(static_cast<float>(mu[i].m10/mu[i].m00), static_cast<float>(mu[i].m01 / mu[i].m00));
    }

    vector<vector<Point> > hull(g_vContours.size());//轮廓凸包
    vector<vector<Point> >contours_poly(g_vContours.size());//轮廓外接多边形
    vector<Rect>boundRect(g_vContours.size());//轮廓外接矩形
    vector<Point2f>center(g_vContours.size());//轮廓外接圆-圆心
    vector<float>radius(g_vContours.size());//轮廓外接圆-半径
    vector<RotatedRect>minRect(g_vContours.size());//轮廓外接可旋转矩形
    vector<RotatedRect>minEllipse(g_vContours.size());//轮廓外接椭圆

    bool hull_fangxiang = rand()&1 == 1? true:false;//凸包方向随机
    for (unsigned int i = 0; i < g_vContours.size(); i++)
    {
        convexHull(Mat(g_vContours[i]),hull[i],hull_fangxiang);//凸包

        approxPolyDP(Mat(g_vContours[i]), contours_poly[i], 3, true);//本轮廓的外接多边形
        boundRect[i] = boundingRect(Mat(contours_poly[i]));//从外接多边形获取外接矩形
        minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);//从外接多边形获取外接圆

        minRect[i] = minAreaRect(Mat(g_vContours[i]));

        if (g_vContours[i].size()>5)
        {
            minEllipse[i] = fitEllipse(Mat(g_vContours[i]));//此函数要求轮廓至少有6个点
        }
    }

    Scalar Contourscolor;
    Mat drawing = Mat::zeros(cannyfindContours.size(), CV_8UC3);

    if(lunkuojiance_draw_in_src->checkState() == Qt::Checked)//在源图绘制
    {
        srcImage.copyTo(drawing);
    }

    for ( unsigned int i = 0; i < g_vContours.size(); i++)
    {
        if(lunkuojiance_color_QComboBox->currentData(Qt::UserRole).toInt() == 0)
        {
            Contourscolor = Scalar(255,255,255);
        }
        else
        {
            Contourscolor = Scalar(rand()&255,rand()&255,rand()&255);
        }

        //绘制轮廓
        if(lunkuojiance_color_QComboBox->currentData(Qt::UserRole).toInt() == 2)
        {
            drawContours(drawing,
                         g_vContours,
                         i,
                         Scalar(255,255,255),
                         line_cuxi);
        }
        else
        {
            drawContours(drawing,
                         g_vContours,
                         i,
                         Contourscolor,
                         line_cuxi);
        }

        //绘制矩
        if(lunkuojiance_isdraw_ju->checkState() == Qt::Checked)
        {
            circle(drawing,
                   mc[i],
                   line_cuxi > 0 ? line_cuxi * 3 : 3,
                   Contourscolor,
                   -1);
        }

        //绘制凸包
        if(lunkuojiance_draw_hull->checkState() == Qt::Checked)
        {
            drawContours(drawing,hull,i,Contourscolor,line_cuxi);
        }

        //绘制逼近多边形
        if(lunkuojiance_draw_poly->checkState() == Qt::Checked)
        {
            drawContours(drawing, contours_poly, i, Contourscolor, line_cuxi);
        }

        //绘制外接矩形
        if(lunkuojiance_draw_rectangle->checkState() == Qt::Checked)
        {
            rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), Contourscolor, line_cuxi);
        }

        //绘制外接圆
        if(lunkuojiance_draw_circle->checkState() == Qt::Checked)
        {
            circle(drawing, center[i], (int)radius[i], Contourscolor, line_cuxi);
        }

        //绘制可旋转外接矩形
        if(lunkuojiance_draw_AreaRect->checkState() == Qt::Checked)
        {
            Point2f rect_points[4];
            minRect[i].points(rect_points);
            if(line_cuxi != -1)//非填充
            {
                for (int j = 0; j < 4; j++)
                {
                    line(drawing, rect_points[j], rect_points[(j + 1) % 4], Contourscolor, line_cuxi);
                }
            }
            else //填充
            {
                Point rookPoints[1][4];
                for (int j = 0; j < 4; j++)
                {
                    rookPoints[0][j] = rect_points[j];
                }
                const Point* ppt[1]={rookPoints[0]};
                int npt[]={4};
                fillPoly(drawing, ppt, npt, 1, Contourscolor);//多边形
            }
        }

        //绘制外接椭圆
        if(lunkuojiance_draw_Ellipse->checkState() == Qt::Checked)
        {
            ellipse(drawing, minEllipse[i], Contourscolor, line_cuxi);
        }
    }

    drawing.copyTo(dstImage);
    dstlabel_show(dstImage);
}

//霍夫线变换
void base_process::on_action_huofu_line_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    Hough_canny_SpinBox_l = new QSpinBox(ui->widget_for_layout);
    Hough_canny_SpinBox_l->setMinimum(1);
    Hough_canny_SpinBox_l->setMaximum(254);
    Hough_canny_SpinBox_l->setValue(50);//设置一个默认值

    Hough_canny_slider_l = new QSlider(ui->widget_for_layout);
    Hough_canny_slider_l->setOrientation(Qt::Horizontal);//水平方向
    Hough_canny_slider_l->setMinimum(1);
    Hough_canny_slider_l->setMaximum(254);
    Hough_canny_slider_l->setSingleStep(10);//步长 动一下移动的距离
    Hough_canny_slider_l->setTickInterval(10); // 设置刻度间隔
    Hough_canny_slider_l->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_canny_slider_l->setValue(50);//设置一个默认值

    connect(Hough_canny_slider_l, SIGNAL(valueChanged(int)), Hough_canny_SpinBox_l, SLOT(setValue(int)));
    connect(Hough_canny_SpinBox_l, SIGNAL(valueChanged(int)), Hough_canny_slider_l, SLOT(setValue(int)));

    connect(Hough_canny_SpinBox_l, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));
    connect(Hough_canny_slider_l, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_canny_SpinBox_h = new QSpinBox(ui->widget_for_layout);
    Hough_canny_SpinBox_h->setMinimum(2);
    Hough_canny_SpinBox_h->setMaximum(1000);
    Hough_canny_SpinBox_h->setValue(200);//设置一个默认值

    Hough_canny_slider_h = new QSlider(ui->widget_for_layout);
    Hough_canny_slider_h->setOrientation(Qt::Horizontal);//水平方向
    Hough_canny_slider_h->setMinimum(2);
    Hough_canny_slider_h->setMaximum(1000);
    Hough_canny_slider_h->setSingleStep(20);//步长 动一下移动的距离
    Hough_canny_slider_h->setTickInterval(40); // 设置刻度间隔
    Hough_canny_slider_h->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_canny_slider_h->setValue(200);//设置一个默认值

    connect(Hough_canny_slider_h, SIGNAL(valueChanged(int)), Hough_canny_SpinBox_h, SLOT(setValue(int)));
    connect(Hough_canny_SpinBox_h, SIGNAL(valueChanged(int)), Hough_canny_slider_h, SLOT(setValue(int)));

    connect(Hough_canny_slider_h, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));
    connect(Hough_canny_SpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_line_SpinBox->setMinimum(1);
    Hough_line_SpinBox->setMaximum(255);
    Hough_line_SpinBox->setValue(200);//设置一个默认值

    Hough_line_slider = new QSlider(ui->widget_for_layout);
    Hough_line_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_line_slider->setMinimum(1);
    Hough_line_slider->setMaximum(255);
    Hough_line_slider->setSingleStep(10);//步长 动一下移动的距离
    Hough_line_slider->setTickInterval(10); // 设置刻度间隔
    Hough_line_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_line_slider->setValue(200);//设置一个默认值

    connect(Hough_line_slider, SIGNAL(valueChanged(int)), Hough_line_SpinBox, SLOT(setValue(int)));
    connect(Hough_line_SpinBox, SIGNAL(valueChanged(int)), Hough_line_slider, SLOT(setValue(int)));

    connect(Hough_line_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));
    connect(Hough_line_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_minlength_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_line_minlength_SpinBox->setMinimum(20);
    Hough_line_minlength_SpinBox->setMaximum(1000);
    Hough_line_minlength_SpinBox->setValue(50);//设置一个默认值

    Hough_line_minlength_slider = new QSlider(ui->widget_for_layout);
    Hough_line_minlength_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_line_minlength_slider->setMinimum(20);
    Hough_line_minlength_slider->setMaximum(100);
    Hough_line_minlength_slider->setSingleStep(20);//步长 动一下移动的距离
    Hough_line_minlength_slider->setTickInterval(25); // 设置刻度间隔
    Hough_line_minlength_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_line_minlength_slider->setValue(50);//设置一个默认值

    connect(Hough_line_minlength_slider, SIGNAL(valueChanged(int)), Hough_line_minlength_SpinBox, SLOT(setValue(int)));
    connect(Hough_line_minlength_SpinBox, SIGNAL(valueChanged(int)), Hough_line_minlength_slider, SLOT(setValue(int)));

    connect(Hough_line_minlength_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));
    connect(Hough_line_minlength_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_maxLineGap_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_line_maxLineGap_SpinBox->setMinimum(1);
    Hough_line_maxLineGap_SpinBox->setMaximum(100);
    Hough_line_maxLineGap_SpinBox->setValue(10);//设置一个默认值

    Hough_line_maxLineGap_slider = new QSlider(ui->widget_for_layout);
    Hough_line_maxLineGap_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_line_maxLineGap_slider->setMinimum(1);
    Hough_line_maxLineGap_slider->setMaximum(100);
    Hough_line_maxLineGap_slider->setSingleStep(10);//步长 动一下移动的距离
    Hough_line_maxLineGap_slider->setTickInterval(10); // 设置刻度间隔
    Hough_line_maxLineGap_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_line_maxLineGap_slider->setValue(10);//设置一个默认值

    connect(Hough_line_maxLineGap_slider, SIGNAL(valueChanged(int)), Hough_line_maxLineGap_SpinBox, SLOT(setValue(int)));
    connect(Hough_line_maxLineGap_SpinBox, SIGNAL(valueChanged(int)), Hough_line_maxLineGap_slider, SLOT(setValue(int)));

    connect(Hough_line_maxLineGap_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));
    connect(Hough_line_maxLineGap_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_thickness = new QSpinBox(ui->widget_for_layout);
    Hough_line_thickness->setMinimum(1);
    Hough_line_thickness->setMaximum(12);
    Hough_line_thickness->setValue(2);//设置一个默认值
    connect(Hough_line_thickness, SIGNAL(valueChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_color = new QCheckBox(ui->widget_for_layout);
    Hough_line_color->setText(tr("单色/多色"));
    connect(Hough_line_color, SIGNAL(stateChanged(int)), this, SLOT(Hough_line_process()));

    Hough_line_draw_in_src = new QCheckBox(ui->widget_for_layout);
    Hough_line_draw_in_src->setText(tr("在源图上绘制直线"));
    connect(Hough_line_draw_in_src, SIGNAL(stateChanged(int)), this, SLOT(Hough_line_process()));

    Hough_canny_show1 = new QLabel(tr("Canny算子低阈值：1~254"),ui->widget_for_layout);
    Hough_canny_show2 = new QLabel(tr("Canny算子高阈值：2~1000"),ui->widget_for_layout);
    Hough_canny_show3 = new QLabel(tr("线段检测阈值："),ui->widget_for_layout);
    Hough_canny_show4 = new QLabel(tr("被检测出线段的最小长度："),ui->widget_for_layout);
    Hough_canny_show5 = new QLabel(tr("允许连接两点间的最大距离"),ui->widget_for_layout);
    Hough_canny_show6 = new QLabel(tr("直线粗细："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(Hough_canny_show1,0,0);
    widget_layout->addWidget(Hough_canny_SpinBox_l,0,1);
    widget_layout->addWidget(Hough_canny_slider_l,0,2,1,3);
    widget_layout->addWidget(Hough_canny_show2,1,0);
    widget_layout->addWidget(Hough_canny_SpinBox_h,1,1);
    widget_layout->addWidget(Hough_canny_slider_h,1,2,1,3);
    widget_layout->addWidget(Hough_canny_show3,2,0);
    widget_layout->addWidget(Hough_line_SpinBox,2,1);
    widget_layout->addWidget(Hough_line_slider,2,2,1,3);
    widget_layout->addWidget(Hough_canny_show4,3,0);
    widget_layout->addWidget(Hough_line_minlength_SpinBox,3,1);
    widget_layout->addWidget(Hough_line_minlength_slider,3,2,1,3);
    widget_layout->addWidget(Hough_canny_show5,4,0);
    widget_layout->addWidget(Hough_line_maxLineGap_SpinBox,4,1);
    widget_layout->addWidget(Hough_line_maxLineGap_slider,4,2,1,3);
    widget_layout->addWidget(Hough_canny_show6,5,0);
    widget_layout->addWidget(Hough_line_thickness,5,1);
    widget_layout->addWidget(Hough_line_color,5,2);
    widget_layout->addWidget(Hough_line_draw_in_src,5,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    Hough_line_process();
}

//霍夫线变换处理
void base_process::Hough_line_process()
{
    int cannylowthreshint = Hough_canny_SpinBox_l->text().toInt();
    int cannythreshdif = cannylowthreshint + 1;

    QString str = "Canny算子高阈值(" + QString("%1").arg(cannythreshdif) + "~1000)：";

    Hough_canny_show2->setText(str);

    Hough_canny_slider_h->setMinimum(cannythreshdif);
    Hough_canny_SpinBox_h->setMinimum(cannythreshdif);

    Mat midImage;

    Canny(srcImage,
          midImage,
          Hough_canny_SpinBox_l->text().toInt(),
          Hough_canny_SpinBox_h->text().toInt(),
          3,
          true);//进行一此canny边缘检测
    cvtColor(midImage,dstImage, CV_GRAY2BGR);

    vector<Vec4i> lines;//定义一个矢量结构lines用于存放得到的线段矢量集合
    HoughLinesP(midImage,
                lines,
                1,
                CV_PI/180,
                Hough_line_SpinBox->text().toInt(),
                Hough_line_minlength_SpinBox->text().toDouble(),
                Hough_line_maxLineGap_SpinBox->text().toDouble());

    if(Hough_line_draw_in_src->checkState() == Qt::Checked)//选中则在源图绘制
    {
        srcImage.copyTo(dstImage);
    }

    Scalar linescolor_single = Scalar(rand()&255,rand()&255,rand()&255);
    Scalar draw_color;
    for( size_t i = 0; i < lines.size(); i++ )
    {

        Vec4i l = lines[i];
        if(Hough_line_color->checkState() == Qt::Checked)
        {
            draw_color = Scalar(rand()&255,rand()&255,rand()&255);//多色
        }
        else
        {
            draw_color = linescolor_single;
        }
        line(dstImage, Point(l[0], l[1]), Point(l[2], l[3]), draw_color, Hough_line_thickness->text().toInt(), CV_AA);
    }

    dstlabel_show(dstImage);
}

//霍夫圆变换
void base_process::on_action_huofu_yuan_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    Hough_circles_yuanxin_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_circles_yuanxin_SpinBox->setMinimum(1);
    Hough_circles_yuanxin_SpinBox->setMaximum(20);
    Hough_circles_yuanxin_SpinBox->setValue(5);//设置一个默认值

    Hough_circles_yuanxin_slider = new QSlider(ui->widget_for_layout);
    Hough_circles_yuanxin_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_circles_yuanxin_slider->setMinimum(1);
    Hough_circles_yuanxin_slider->setMaximum(20);
    Hough_circles_yuanxin_slider->setSingleStep(1);//步长 动一下移动的距离
    Hough_circles_yuanxin_slider->setTickInterval(1); // 设置刻度间隔
    Hough_circles_yuanxin_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_circles_yuanxin_slider->setValue(5);//设置一个默认值

    connect(Hough_circles_yuanxin_slider, SIGNAL(valueChanged(int)), Hough_circles_yuanxin_SpinBox, SLOT(setValue(int)));
    connect(Hough_circles_yuanxin_SpinBox, SIGNAL(valueChanged(int)), Hough_circles_yuanxin_slider, SLOT(setValue(int)));

    connect(Hough_circles_yuanxin_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));
    connect(Hough_circles_yuanxin_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_circles_canny_SpinBox_h = new QSpinBox(ui->widget_for_layout);
    Hough_circles_canny_SpinBox_h->setMinimum(2);
    Hough_circles_canny_SpinBox_h->setMaximum(600);
    Hough_circles_canny_SpinBox_h->setValue(300);//设置一个默认值

    Hough_circles_canny_slider_h = new QSlider(ui->widget_for_layout);
    Hough_circles_canny_slider_h->setOrientation(Qt::Horizontal);//水平方向
    Hough_circles_canny_slider_h->setMinimum(2);
    Hough_circles_canny_slider_h->setMaximum(600);
    Hough_circles_canny_slider_h->setSingleStep(10);//步长 动一下移动的距离
    Hough_circles_canny_slider_h->setTickInterval(20); // 设置刻度间隔
    Hough_circles_canny_slider_h->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_circles_canny_slider_h->setValue(300);//设置一个默认值

    connect(Hough_circles_canny_slider_h, SIGNAL(valueChanged(int)), Hough_circles_canny_SpinBox_h, SLOT(setValue(int)));
    connect(Hough_circles_canny_SpinBox_h, SIGNAL(valueChanged(int)), Hough_circles_canny_slider_h, SLOT(setValue(int)));

    connect(Hough_circles_canny_slider_h, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));
    connect(Hough_circles_canny_SpinBox_h, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_circles_point_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_circles_point_SpinBox->setMinimum(1);
    Hough_circles_point_SpinBox->setMaximum(255);
    Hough_circles_point_SpinBox->setValue(100);//设置一个默认值

    Hough_circles_point_slider = new QSlider(ui->widget_for_layout);
    Hough_circles_point_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_circles_point_slider->setMinimum(1);
    Hough_circles_point_slider->setMaximum(255);
    Hough_circles_point_slider->setSingleStep(10);//步长 动一下移动的距离
    Hough_circles_point_slider->setTickInterval(10); // 设置刻度间隔
    Hough_circles_point_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_circles_point_slider->setValue(100);//设置一个默认值

    connect(Hough_circles_point_slider, SIGNAL(valueChanged(int)), Hough_circles_point_SpinBox, SLOT(setValue(int)));
    connect(Hough_circles_point_SpinBox, SIGNAL(valueChanged(int)), Hough_circles_point_slider, SLOT(setValue(int)));

    connect(Hough_circles_point_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));
    connect(Hough_circles_point_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_minRadius_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_minRadius_SpinBox->setMinimum(0);
    Hough_minRadius_SpinBox->setMaximum(1000);
    Hough_minRadius_SpinBox->setValue(0);//设置一个默认值

    Hough_minRadius_slider = new QSlider(ui->widget_for_layout);
    Hough_minRadius_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_minRadius_slider->setMinimum(0);
    Hough_minRadius_slider->setMaximum(1000);
    Hough_minRadius_slider->setSingleStep(50);//步长 动一下移动的距离
    Hough_minRadius_slider->setTickInterval(40); // 设置刻度间隔
    Hough_minRadius_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_minRadius_slider->setValue(0);//设置一个默认值

    connect(Hough_minRadius_slider, SIGNAL(valueChanged(int)), Hough_minRadius_SpinBox, SLOT(setValue(int)));
    connect(Hough_minRadius_SpinBox, SIGNAL(valueChanged(int)), Hough_minRadius_slider, SLOT(setValue(int)));

    connect(Hough_minRadius_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));
    connect(Hough_minRadius_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_maxRadius_SpinBox = new QSpinBox(ui->widget_for_layout);
    Hough_maxRadius_SpinBox->setMinimum(0);
    Hough_maxRadius_SpinBox->setMaximum(1000);
    Hough_maxRadius_SpinBox->setValue(0);//设置一个默认值

    Hough_maxRadius_slider = new QSlider(ui->widget_for_layout);
    Hough_maxRadius_slider->setOrientation(Qt::Horizontal);//水平方向
    Hough_maxRadius_slider->setMinimum(0);
    Hough_maxRadius_slider->setMaximum(1000);
    Hough_maxRadius_slider->setSingleStep(50);//步长 动一下移动的距离
    Hough_maxRadius_slider->setTickInterval(40); // 设置刻度间隔
    Hough_maxRadius_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Hough_maxRadius_slider->setValue(0);//设置一个默认值

    connect(Hough_maxRadius_slider, SIGNAL(valueChanged(int)), Hough_maxRadius_SpinBox, SLOT(setValue(int)));
    connect(Hough_maxRadius_SpinBox, SIGNAL(valueChanged(int)), Hough_maxRadius_slider, SLOT(setValue(int)));

    connect(Hough_maxRadius_slider, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));
    connect(Hough_maxRadius_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_circles_thickness = new QSpinBox(ui->widget_for_layout);
    Hough_circles_thickness->setMinimum(-1);
    Hough_circles_thickness->setMaximum(12);
    Hough_circles_thickness->setValue(3);//设置一个默认值
    connect(Hough_circles_thickness, SIGNAL(valueChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_Radius_draw_in_src = new QCheckBox(ui->widget_for_layout);
    Hough_Radius_draw_in_src->setText(tr("绘制半径               注意：处理大图会比较耗时"));
    connect(Hough_Radius_draw_in_src, SIGNAL(stateChanged(int)), this, SLOT(Hough_circle_process()));

    Hough_circles_show02 = new QLabel(tr("圆心间最小距离："),ui->widget_for_layout);
    Hough_circles_show03 = new QLabel(tr("霍夫梯度法边缘检测Canny算子的高阈值："),ui->widget_for_layout);
    Hough_circles_show04 = new QLabel(tr("霍夫梯度法检测圆心的累加器阈值："),ui->widget_for_layout);
    Hough_circles_show05 = new QLabel(tr("最小半径："),ui->widget_for_layout);
    Hough_circles_show06 = new QLabel(tr("最大半径："),ui->widget_for_layout);
    Hough_circles_show07 = new QLabel(tr("圆线粗细："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(Hough_circles_show02,0,0);
    widget_layout->addWidget(Hough_circles_yuanxin_SpinBox,0,1);
    widget_layout->addWidget(Hough_circles_yuanxin_slider,0,2,1,3);
    widget_layout->addWidget(Hough_circles_show03,1,0);
    widget_layout->addWidget(Hough_circles_canny_SpinBox_h,1,1);
    widget_layout->addWidget(Hough_circles_canny_slider_h,1,2,1,3);
    widget_layout->addWidget(Hough_circles_show04,2,0);
    widget_layout->addWidget(Hough_circles_point_SpinBox,2,1);
    widget_layout->addWidget(Hough_circles_point_slider,2,2,1,3);
    widget_layout->addWidget(Hough_circles_show05,3,0);
    widget_layout->addWidget(Hough_minRadius_SpinBox,3,1);
    widget_layout->addWidget(Hough_minRadius_slider,3,2,1,3);
    widget_layout->addWidget(Hough_circles_show06,4,0);
    widget_layout->addWidget(Hough_maxRadius_SpinBox,4,1);
    widget_layout->addWidget(Hough_maxRadius_slider,4,2,1,3);
    widget_layout->addWidget(Hough_circles_show07,5,0);
    widget_layout->addWidget(Hough_circles_thickness,5,1);
    widget_layout->addWidget(Hough_Radius_draw_in_src,5,2,1,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    Hough_circle_process();
}

//霍夫圆变换
void base_process::Hough_circle_process()
{
    Mat midImage;

    cvtColor(srcImage,midImage, CV_BGR2GRAY);
    //GaussianBlur(midImage, midImage, Size(9, 9), 0, 0);
    medianBlur(midImage,midImage,9);

    vector<Vec3f> circles;//Vec3f记录3个值 圆心坐标、半径
    HoughCircles(midImage,
                 circles,
                 CV_HOUGH_GRADIENT,//霍夫梯度法
                 1,
                 Hough_circles_yuanxin_SpinBox->text().toDouble(),
                 Hough_circles_canny_SpinBox_h->text().toDouble(),
                 Hough_circles_point_SpinBox->text().toDouble(), //大于这个值才被当做圆心（猜测）
                 Hough_minRadius_SpinBox->text().toInt(),
                 Hough_maxRadius_SpinBox->text().toInt());

    srcImage.copyTo(dstImage);
    Scalar linescolor;

    for( size_t i = 0; i < circles.size(); i++ )
    {
        linescolor = Scalar(rand()&255,rand()&255,rand()&255);
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        //绘制圆轮廓
        circle(dstImage,
               center,
               radius,
               linescolor,
               Hough_circles_thickness->text().toInt(),
               8);
        //绘制圆心
        circle(dstImage, center, 5, Scalar(255, 0, 0), -1, 8);

        if(Hough_Radius_draw_in_src->checkState() == Qt::Checked)//选中绘制半径
        {
            int angle = qrand() % 360;//随机角度
            //根据角度、半径、圆心求另一点的坐标
            int x1 = int(center.x + radius * cos(angle * CV_PI/180));
            int y1 = int(center.y + radius * sin(angle * CV_PI/180));
            if(Hough_circles_thickness->text().toInt() == -1)
            {
                line(dstImage, center, Point(x1,y1), Scalar(rand()&255,rand()&255,rand()&255), 2);
            }
            else
            {
                line(dstImage, center, Point(x1,y1), linescolor, Hough_circles_thickness->text().toInt());
            }            
        }
    }
    dstlabel_show(dstImage);
}

//角点检测
void base_process::on_action_jiaodianjiance_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    Features_pixel_show01 = new QLabel(tr("能检测出的最大角点数目："),ui->widget_for_layout);
    Features_pixel_show02 = new QLabel(tr("角点强度系数："),ui->widget_for_layout);
    Features_pixel_show03 = new QLabel(tr("角点间的最小距离："),ui->widget_for_layout);
    Features_pixel_show04 = new QLabel(tr("检测角点时参与计算的区域大小："),ui->widget_for_layout);
    Features_pixel_show05 = new QLabel(tr("绘制圆线的粗细："),ui->widget_for_layout);
    Features_pixel_show06 = new QLabel(tr("     绘制圆的半径："),ui->widget_for_layout);
    Features_pixel_show07 = new QLabel(tr("角点检测类型："),ui->widget_for_layout);

    Features_pixel_maxCorners_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_maxCorners_SpinBox->setMinimum(50);
    Features_pixel_maxCorners_SpinBox->setMaximum(5000);
    Features_pixel_maxCorners_SpinBox->setValue(100);//设置一个默认值

    Features_pixel_maxCorners_slider = new QSlider(ui->widget_for_layout);
    Features_pixel_maxCorners_slider->setOrientation(Qt::Horizontal);//水平方向
    Features_pixel_maxCorners_slider->setMinimum(50);
    Features_pixel_maxCorners_slider->setMaximum(5000);
    Features_pixel_maxCorners_slider->setSingleStep(50);//步长 动一下移动的距离
    Features_pixel_maxCorners_slider->setTickInterval(100); // 设置刻度间隔
    Features_pixel_maxCorners_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Features_pixel_maxCorners_slider->setValue(100);//设置一个默认值

    connect(Features_pixel_maxCorners_slider, SIGNAL(valueChanged(int)), Features_pixel_maxCorners_SpinBox, SLOT(setValue(int)));
    connect(Features_pixel_maxCorners_SpinBox, SIGNAL(valueChanged(int)), Features_pixel_maxCorners_slider, SLOT(setValue(int)));

    connect(Features_pixel_maxCorners_slider, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));
    connect(Features_pixel_maxCorners_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_qualityLevel_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_qualityLevel_SpinBox->setMinimum(1);
    Features_pixel_qualityLevel_SpinBox->setMaximum(99);
    Features_pixel_qualityLevel_SpinBox->setValue(1);//设置一个默认值
    Features_pixel_qualityLevel_SpinBox->setPrefix("0.0");//前缀

    Features_pixel_qualityLevel_slider = new QSlider(ui->widget_for_layout);
    Features_pixel_qualityLevel_slider->setOrientation(Qt::Horizontal);//水平方向
    Features_pixel_qualityLevel_slider->setMinimum(1);
    Features_pixel_qualityLevel_slider->setMaximum(99);
    Features_pixel_qualityLevel_slider->setSingleStep(10);//步长 动一下移动的距离
    Features_pixel_qualityLevel_slider->setTickInterval(10); // 设置刻度间隔
    Features_pixel_qualityLevel_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Features_pixel_qualityLevel_slider->setValue(1);//设置一个默认值

    connect(Features_pixel_qualityLevel_slider, SIGNAL(valueChanged(int)), Features_pixel_qualityLevel_SpinBox, SLOT(setValue(int)));
    connect(Features_pixel_qualityLevel_SpinBox, SIGNAL(valueChanged(int)), Features_pixel_qualityLevel_slider, SLOT(setValue(int)));

    connect(Features_pixel_qualityLevel_slider, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));
    connect(Features_pixel_qualityLevel_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_minDistance_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_minDistance_SpinBox->setMinimum(3);
    Features_pixel_minDistance_SpinBox->setMaximum(20);
    Features_pixel_minDistance_SpinBox->setValue(3);//设置一个默认值

    Features_pixel_minDistance_slider = new QSlider(ui->widget_for_layout);
    Features_pixel_minDistance_slider->setOrientation(Qt::Horizontal);//水平方向
    Features_pixel_minDistance_slider->setMinimum(3);
    Features_pixel_minDistance_slider->setMaximum(20);
    Features_pixel_minDistance_slider->setSingleStep(1);//步长 动一下移动的距离
    Features_pixel_minDistance_slider->setTickInterval(1); // 设置刻度间隔
    Features_pixel_minDistance_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Features_pixel_minDistance_slider->setValue(3);//设置一个默认值

    connect(Features_pixel_minDistance_slider, SIGNAL(valueChanged(int)), Features_pixel_minDistance_SpinBox, SLOT(setValue(int)));
    connect(Features_pixel_minDistance_SpinBox, SIGNAL(valueChanged(int)), Features_pixel_minDistance_slider, SLOT(setValue(int)));

    connect(Features_pixel_minDistance_slider, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));
    connect(Features_pixel_minDistance_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_blockSize_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_blockSize_SpinBox->setMinimum(3);
    Features_pixel_blockSize_SpinBox->setMaximum(15);
    Features_pixel_blockSize_SpinBox->setValue(3);//设置一个默认值

    Features_pixel_blockSize_slider = new QSlider(ui->widget_for_layout);
    Features_pixel_blockSize_slider->setOrientation(Qt::Horizontal);//水平方向
    Features_pixel_blockSize_slider->setMinimum(3);
    Features_pixel_blockSize_slider->setMaximum(15);
    Features_pixel_blockSize_slider->setSingleStep(1);//步长 动一下移动的距离
    Features_pixel_blockSize_slider->setTickInterval(1); // 设置刻度间隔
    Features_pixel_blockSize_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    Features_pixel_blockSize_slider->setValue(3);//设置一个默认值

    connect(Features_pixel_blockSize_slider, SIGNAL(valueChanged(int)), Features_pixel_blockSize_SpinBox, SLOT(setValue(int)));
    connect(Features_pixel_blockSize_SpinBox, SIGNAL(valueChanged(int)), Features_pixel_blockSize_slider, SLOT(setValue(int)));

    connect(Features_pixel_blockSize_slider, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));
    connect(Features_pixel_blockSize_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_draw_color = new QCheckBox(ui->widget_for_layout);
    Features_pixel_draw_color->setText(tr("单色/多色"));
    connect(Features_pixel_draw_color, SIGNAL(stateChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_line_cuxi_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_line_cuxi_SpinBox->setMinimum(-1);
    Features_pixel_line_cuxi_SpinBox->setMaximum(8);
    Features_pixel_line_cuxi_SpinBox->setValue(-1);
    connect(Features_pixel_line_cuxi_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_pixel_line_banjing_SpinBox = new QSpinBox(ui->widget_for_layout);
    Features_pixel_line_banjing_SpinBox->setMinimum(3);
    Features_pixel_line_banjing_SpinBox->setMaximum(20);
    Features_pixel_line_banjing_SpinBox->setValue(3);
    connect(Features_pixel_line_banjing_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(Features_pixel()));

    Features_type = new QComboBox(ui->widget_for_layout);
    Features_type->addItem(tr("像素级角点检测"),1);
    Features_type->addItem(tr("亚像素级角点检测"),2);
    connect(Features_type, SIGNAL(activated(const QString &)),this, SLOT(Features_pixel()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(Features_pixel_show07,0,0);
    widget_layout->addWidget(Features_type,0,1);
    widget_layout->addWidget(Features_pixel_show01,1,0);
    widget_layout->addWidget(Features_pixel_maxCorners_SpinBox,1,1);
    widget_layout->addWidget(Features_pixel_maxCorners_slider,1,2,1,3);
    widget_layout->addWidget(Features_pixel_show02,2,0);
    widget_layout->addWidget(Features_pixel_qualityLevel_SpinBox,2,1);
    widget_layout->addWidget(Features_pixel_qualityLevel_slider,2,2,1,3);
    widget_layout->addWidget(Features_pixel_show03,3,0);
    widget_layout->addWidget(Features_pixel_minDistance_SpinBox,3,1);
    widget_layout->addWidget(Features_pixel_minDistance_slider,3,2,1,3);
    widget_layout->addWidget(Features_pixel_show04,4,0);
    widget_layout->addWidget(Features_pixel_blockSize_SpinBox,4,1);
    widget_layout->addWidget(Features_pixel_blockSize_slider,4,2,1,3);
    widget_layout->addWidget(Features_pixel_draw_color,5,0);
    widget_layout->addWidget(Features_pixel_show05,5,1);
    widget_layout->addWidget(Features_pixel_line_cuxi_SpinBox,5,2);
    widget_layout->addWidget(Features_pixel_show06,5,3);
    widget_layout->addWidget(Features_pixel_line_banjing_SpinBox,5,4);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    Features_pixel();
}

//角点检测处理
void base_process::Features_pixel()
{
    if(Features_pixel_qualityLevel_slider->value() >=10)
    {
        Features_pixel_qualityLevel_SpinBox->setPrefix("0.");//前缀
    }
    else
    {
        Features_pixel_qualityLevel_SpinBox->setPrefix("0.0");//前缀
    }

    std::vector<cv::Point2f> corners;
    cv::Mat image_gray;

    cv::cvtColor(srcImage, image_gray, cv::COLOR_BGR2GRAY);

    cv::goodFeaturesToTrack(image_gray,
                            corners,
                            Features_pixel_maxCorners_SpinBox->text().toInt(),
                            Features_pixel_qualityLevel_SpinBox->text().toCaseFolded().toDouble(),
                            Features_pixel_minDistance_SpinBox->text().toDouble(),
                            cv::Mat(),
                            Features_pixel_blockSize_SpinBox->text().toInt());

    srcImage.copyTo(dstImage);

    Scalar color_single = Scalar(rand()&255,rand()&255,rand()&255);
    Scalar color;

    if(Features_type->currentData(Qt::UserRole).toInt() == 1)//像素级角点检测
    {
        for (int i = 0; i < corners.size(); i++)
        {
            if(Features_pixel_draw_color->checkState() == Qt::Checked)
            {
                color = Scalar(rand()&255,rand()&255,rand()&255);
            }
            else
            {
                color = color_single;
            }

            cv::circle(dstImage,
                       corners[i],
                       Features_pixel_line_banjing_SpinBox->text().toInt(),
                       color,
                       Features_pixel_line_cuxi_SpinBox->text().toInt());

            //绘制圆心
            cv::circle(dstImage,
                       corners[i],
                       2,
                       Scalar(255,0,0),
                       -1);
        }
        dstlabel_show(dstImage);
    }
    else //亚像素级角点检测
    {
        cv::cornerSubPix(image_gray,
                         corners,
                         cv::Size(Features_pixel_blockSize_SpinBox->text().toInt(), Features_pixel_blockSize_SpinBox->text().toInt()),
                         cv::Size(-1, -1),
                         cv::TermCriteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 40, 0.01));

        for (int i = 0; i < corners.size(); i++)
        {
            if(Features_pixel_draw_color->checkState() == Qt::Checked)
            {
                color = Scalar(rand()&255,rand()&255,rand()&255);
            }
            else
            {
                color = color_single;
            }

            cv::circle(dstImage,
                       corners[i],
                       Features_pixel_line_banjing_SpinBox->text().toInt(),
                       color,
                       Features_pixel_line_cuxi_SpinBox->text().toInt());

            //绘制圆心
            cv::circle(dstImage,
                       corners[i],
                       2,
                       Scalar(255,0,0),
                       -1);
        }
        dstlabel_show(dstImage);
    }
}

//k近邻图像分割
void base_process::on_actionK_Means_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    kmeans_num_SpinBox = new QSpinBox(ui->widget_for_layout);
    kmeans_num_SpinBox->setMinimum(2);
    kmeans_num_SpinBox->setMaximum(20);
    kmeans_num_SpinBox->setValue(3);//设置一个默认值

    kmeans_num_slider = new QSlider(ui->widget_for_layout);
    kmeans_num_slider->setOrientation(Qt::Horizontal);//水平方向
    kmeans_num_slider->setMinimum(2);
    kmeans_num_slider->setMaximum(20);
    kmeans_num_slider->setSingleStep(1);//步长 动一下移动的距离
    kmeans_num_slider->setTickInterval(1); // 设置刻度间隔
    kmeans_num_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    kmeans_num_slider->setValue(3);//设置一个默认值

    connect(kmeans_num_slider, SIGNAL(valueChanged(int)), kmeans_num_SpinBox, SLOT(setValue(int)));
    connect(kmeans_num_SpinBox, SIGNAL(valueChanged(int)), kmeans_num_slider, SLOT(setValue(int)));

    kmeans_time_SpinBox = new QSpinBox(ui->widget_for_layout);
    kmeans_time_SpinBox->setMinimum(3);
    kmeans_time_SpinBox->setMaximum(20);
    kmeans_time_SpinBox->setValue(10);//设置一个默认值

    kmeans_time_slider = new QSlider(ui->widget_for_layout);
    kmeans_time_slider->setOrientation(Qt::Horizontal);//水平方向
    kmeans_time_slider->setMinimum(3);
    kmeans_time_slider->setMaximum(20);
    kmeans_time_slider->setSingleStep(1);//步长 动一下移动的距离
    kmeans_time_slider->setTickInterval(1); // 设置刻度间隔
    kmeans_time_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    kmeans_time_slider->setValue(10);//设置一个默认值

    connect(kmeans_time_slider, SIGNAL(valueChanged(int)), kmeans_time_SpinBox, SLOT(setValue(int)));
    connect(kmeans_time_SpinBox, SIGNAL(valueChanged(int)), kmeans_time_slider, SLOT(setValue(int)));

    kmeans_show01 = new QLabel(tr("聚类分类数目："),ui->widget_for_layout);
    kmeans_show02 = new QLabel(tr("K-Means算法执行次数："),ui->widget_for_layout);
    kmeans_show03 = new QLabel(tr("提示：参数设置较大时会比较耗时"),ui->widget_for_layout);

    kmeans_isok = new QPushButton("确定",ui->widget_for_layout);
    connect(kmeans_isok, SIGNAL(clicked()), this, SLOT(process_keans()));

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(kmeans_show01,0,0);
    widget_layout->addWidget(kmeans_num_SpinBox,0,1);
    widget_layout->addWidget(kmeans_num_slider,0,2);
    widget_layout->addWidget(kmeans_show03,0,3);
    widget_layout->addWidget(kmeans_show02,1,0);
    widget_layout->addWidget(kmeans_time_SpinBox,1,1);
    widget_layout->addWidget(kmeans_time_slider,1,2);
    widget_layout->addWidget(kmeans_isok,1,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    process_keans();
}

//k近邻图像分割处理
void base_process::process_keans()
{
    int width = srcImage.cols;
    int height = srcImage.rows;

    //将图像像素转换为数据点
    Mat points(width * height, srcImage.channels(), CV_32F);
    int index = 0;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            index = i * width + j;
            points.at<float>(index, 0) = srcImage.at<Vec3b>(i, j)[0];
            points.at<float>(index, 1) = srcImage.at<Vec3b>(i, j)[1];
            points.at<float>(index, 2) = srcImage.at<Vec3b>(i, j)[2];
        }
    }

    //进行KMeans聚类
    int times = kmeans_num_SpinBox->value();

    Mat bestLabels;
    Mat centers(3, 3, CV_32F);
    //迭代参数
    TermCriteria criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 1000, 0.001);
    kmeans(points, times, bestLabels, criteria, kmeans_time_SpinBox->value(), KMEANS_PP_CENTERS, centers);

    Scalar color[times];
    for(int i = 0;i < times;i++)
    {
        color[i] = CV_RGB( rand()&255, rand()&255, rand()&255 );
    }

    dstImage = Mat::zeros(srcImage.size(), srcImage.type());
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            index = i * width + j;
            int lable = bestLabels.at<int>(index, 0);
            dstImage.at<Vec3b>(i, j)[0] = color[lable][0];//B
            dstImage.at<Vec3b>(i, j)[1] = color[lable][1];//G
            dstImage.at<Vec3b>(i, j)[2] = color[lable][2];//R
        }
    }
    dstlabel_show(dstImage);
}

//漫水填充分割
void base_process::on_action_floodFill_triggered()
{
    if (srcImage.empty())
    {
        QMessageBox::information(this,"提示","请先打开一张图片");
        return;
    }
    close_other_obj();

    floodfill_lowDifference_SpinBox = new QSpinBox(ui->widget_for_layout);
    floodfill_lowDifference_SpinBox->setMinimum(0);
    floodfill_lowDifference_SpinBox->setMaximum(255);
    floodfill_lowDifference_SpinBox->setValue(0);//设置一个默认值

    floodfill_lowDifference_slider = new QSlider(ui->widget_for_layout);
    floodfill_lowDifference_slider->setOrientation(Qt::Horizontal);//水平方向
    floodfill_lowDifference_slider->setMinimum(0);
    floodfill_lowDifference_slider->setMaximum(255);
    floodfill_lowDifference_slider->setSingleStep(10);//步长 动一下移动的距离
    floodfill_lowDifference_slider->setTickInterval(10); // 设置刻度间隔
    floodfill_lowDifference_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    floodfill_lowDifference_slider->setValue(0);//设置一个默认值

    connect(floodfill_lowDifference_slider, SIGNAL(valueChanged(int)), floodfill_lowDifference_SpinBox, SLOT(setValue(int)));
    connect(floodfill_lowDifference_SpinBox, SIGNAL(valueChanged(int)), floodfill_lowDifference_slider, SLOT(setValue(int)));

    connect(floodfill_lowDifference_slider, SIGNAL(valueChanged(int)), this, SLOT(do_floodfill()));
    connect(floodfill_lowDifference_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(do_floodfill()));

    floodfill_upDifference_SpinBox = new QSpinBox(ui->widget_for_layout);
    floodfill_upDifference_SpinBox->setMinimum(0);
    floodfill_upDifference_SpinBox->setMaximum(255);
    floodfill_upDifference_SpinBox->setValue(0);//设置一个默认值

    floodfill_upDifference_slider = new QSlider(ui->widget_for_layout);
    floodfill_upDifference_slider->setOrientation(Qt::Horizontal);//水平方向
    floodfill_upDifference_slider->setMinimum(0);
    floodfill_upDifference_slider->setMaximum(255);
    floodfill_upDifference_slider->setSingleStep(10);//步长 动一下移动的距离
    floodfill_upDifference_slider->setTickInterval(10); // 设置刻度间隔
    floodfill_upDifference_slider->setTickPosition(QSlider::TicksAbove);  //刻度在上方
    floodfill_upDifference_slider->setValue(0);//设置一个默认值

    connect(floodfill_upDifference_slider, SIGNAL(valueChanged(int)), floodfill_upDifference_SpinBox, SLOT(setValue(int)));
    connect(floodfill_upDifference_SpinBox, SIGNAL(valueChanged(int)), floodfill_upDifference_slider, SLOT(setValue(int)));

    connect(floodfill_upDifference_slider, SIGNAL(valueChanged(int)), this, SLOT(do_floodfill()));
    connect(floodfill_upDifference_SpinBox, SIGNAL(valueChanged(int)), this, SLOT(do_floodfill()));

    floodfill_liantongxing = new QComboBox(ui->widget_for_layout);
    floodfill_liantongxing->addItem(tr("4连通"),4);
    floodfill_liantongxing->addItem(tr("8连通"),8);
    connect(floodfill_liantongxing, SIGNAL(activated(const QString &)),this, SLOT(do_floodfill()));

    floodfill_show01 = new QLabel(tr("当前点与种子点间颜色负差最大值："),ui->widget_for_layout);
    floodfill_show02 = new QLabel(tr("当前点与种子点间颜色正差最大值："),ui->widget_for_layout);
    floodfill_show03 = new QLabel(tr("像素连通性："),ui->widget_for_layout);

    if(widget_layout != NULL)
    {
        delete widget_layout;
        widget_layout = NULL;
    }
    widget_layout = new QGridLayout(ui->widget_for_layout);
    widget_layout->addWidget(floodfill_show03,0,0);
    widget_layout->addWidget(floodfill_liantongxing,0,1);
    widget_layout->addWidget(floodfill_show01,1,0);
    widget_layout->addWidget(floodfill_lowDifference_SpinBox,1,1);
    widget_layout->addWidget(floodfill_lowDifference_slider,1,2,1,3);
    widget_layout->addWidget(floodfill_show02,2,0);
    widget_layout->addWidget(floodfill_upDifference_SpinBox,2,1);
    widget_layout->addWidget(floodfill_upDifference_slider,2,2,1,3);
    widget_layout->setAlignment(Qt::AlignVCenter);
    ui->widget_for_layout->setLayout(widget_layout);

    do_floodfill();
}

//漫水填充处理
void base_process::do_floodfill()
{
    bool bIsColor;//是否彩色图
    //dstImage = Mat::zeros(srcImage.size(), CV_8UC3);
    srcImage.copyTo(dstImage);

    int low_diff_int = floodfill_lowDifference_slider->value();
    int up_diff_int = floodfill_upDifference_slider->value();

    Scalar low_diff = Scalar(low_diff_int,low_diff_int,low_diff_int);
    Scalar up_diff = Scalar(up_diff_int,up_diff_int,up_diff_int);

    int flags = floodfill_liantongxing->currentData(Qt::UserRole).toInt() + (255<<8) + FLOODFILL_FIXED_RANGE;

    Rect ccomp;//定义重绘区域的最小边界矩形区域

    floodFill(dstImage,
              Point(rand()&(dstImage.cols - 2) + 1,rand()&(dstImage.rows - 2) + 1), //随机种子点
              Scalar((unsigned)theRNG() &255,(unsigned)theRNG() &255,(unsigned)theRNG() &255),
              &ccomp,
              low_diff,
              up_diff,
              flags);
    dstlabel_show(dstImage);
}

//拖动事件 拖动打开图片
void base_process::dragEnterEvent(QDragEnterEvent* event)
{
    //文件名以xx结尾
    if(!event->mimeData()->urls()[0].fileName().right(3).compare("jpg")
       ||!event->mimeData()->urls()[0].fileName().right(3).compare("JPG")
       ||!event->mimeData()->urls()[0].fileName().right(3).compare("png")
       ||!event->mimeData()->urls()[0].fileName().right(3).compare("PNG")
       ||!event->mimeData()->urls()[0].fileName().right(3).compare("bmp")
       ||!event->mimeData()->urls()[0].fileName().right(3).compare("BMP")
       ||!event->mimeData()->urls()[0].fileName().right(4).compare("jpeg")
       ||!event->mimeData()->urls()[0].fileName().right(4).compare("JPEG"))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();//不是这几种类型的图片不接受此事件
    }
}

//放下事件
void base_process::dropEvent(QDropEvent* event)
{    
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
    {
        return;
    }

    //打开图片
    QString fileName = urls.first().toLocalFile();

    qDebug()<<tr("源图像路径：")<<fileName;
    if(fileName.isEmpty())
    {
        return;
    }
    close_other_obj();

    QFile *file = new QFile;
    file->setFileName(fileName);
    QFileInfo filetemp = QFileInfo(fileName);
    imgfilename = filetemp.baseName();
    openfilepath = filetemp.path();
    savefilepath.clear();

    QTextCodec *code = QTextCodec::codecForName("gb18030");//确保中文不乱码
    std::string name = code->fromUnicode(fileName).data();
    //读取文件
    srcImage = cv::imread(name);
    srcImage.copyTo(srcImage_copy);

    //将BGR转换为RGB，方便操作习惯
    cv::cvtColor(srcImage,srcImage,CV_BGR2RGB);
    img = QImage((const unsigned char*)(srcImage.data),
                 srcImage.cols,
                 srcImage.rows,
                 srcImage.cols*srcImage.channels(),
                 QImage::Format_RGB888);
    ui->labelsrc->clear();
    ui->labeldst->clear();
    img = img.scaled(ui->labelsrc->width(), ui->labelsrc->height());
    ui->labelsrc->setPixmap(QPixmap::fromImage(img));
    save_num = 1;
}

//关于
void base_process::on_action_help_triggered()
{    
    QMessageBox::information(this,"说明","基于Qt5.9 & OpenCV3.0"
                                       "\n版本：2.0.1"
                                       "\n更新时间：2019年3月4日"
                                       "\n作者QQ：709579619 | 微信：siyuan7095"
                                       "\n本软件不定时更新，更新发布地址："
                                       "\nhttps://github.com/siyuan7095/picture_process");
    return;
}

//清除布局中的全部控件，清除后控件在界面上消失
void base_process::close_other_obj()
{
    QLayoutItem *child;
    while ((child = widget_layout->takeAt(0)) != 0)
    {
        if(child->widget())
        {
            child->widget()->setParent(NULL);//setParent为NULL，防止删除之后界面不消失
        }
        delete child;
    }

    if(ui->is_single_process->checkState() == Qt::Checked)//是否连续处理图像，是则将当前的效果图作为下次处理的源图
    {
        if(!dstImage.empty())
        {
            if(dstImage.type() == CV_8UC1)//如果处理后得到的是单通道图像直接复制给srcImage，进行其他操作可能会出错
            {
                //将单通道的内容复制到3个通道
                Mat three_channel = Mat::zeros(dstImage.rows,dstImage.cols,CV_8UC3);
                vector<Mat> channels;
                for (int i=0;i<3;i++)
                {
                    channels.push_back(dstImage);
                }
                merge(channels,three_channel);
                three_channel.copyTo(srcImage);
            }
            else
            {
                dstImage.copyTo(srcImage);
            }
            dstImage.release();
            img = QImage((const unsigned char*)(srcImage.data),
                         srcImage.cols,
                         srcImage.rows,
                         srcImage.cols*srcImage.channels(),
                         QImage::Format_RGB888);
            ui->labelsrc->clear();
            ui->labeldst->clear();
            img = img.scaled(ui->labelsrc->width(), ui->labelsrc->height());
            ui->labelsrc->setPixmap(QPixmap::fromImage(img));
        }
    }
}
