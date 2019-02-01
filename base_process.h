#ifndef BASE_PROCESS_H
#define BASE_PROCESS_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <opencv/cv.h>
#include <QTextCodec>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QTdebug>
#include <QImage>
#include <QInputDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QBuffer>
#include <QPainter>
#include <time.h>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMetaEnum>
#include <QLineEdit>
#include <QTime>

using namespace std;
using namespace cv;
namespace Ui {
class base_process;
}

class base_process : public QMainWindow
{
    Q_OBJECT
    //Q_ENUM(cv::ColorConversionCodes);

public:
    explicit base_process(QWidget *parent = 0);
    ~base_process();
    //变量
    Mat srcImage;//图像
    Mat dstImage;
    Mat srcImage_copy;//还原图像时使用
    QImage img;//QT的图像对象

    //形态学操作
    QLabel* element_show_01;
    QLabel* element_show_02;
    QLabel* element_show_03;
    QComboBox* element_size_w;//下拉框 用于选择形态学操作的核大小
    QComboBox* element_size_h;
    QComboBox* element_type;//下拉框 用于选择形态学操作的核类型
    QComboBox* morphfun;//下拉框 用于选择形态学操作类型
    //均值滤波
    QComboBox* blursize_w;
    QComboBox* blursize_h;
    //方框滤波
    QComboBox* boxFiltersize_w;
    QComboBox* boxFiltersize_h;
    QCheckBox* isNormalize;
    //高斯滤波
    QComboBox* gusssizehight;
    QComboBox* gausssizewith;
    //中值滤波
    QComboBox* mediansize;
    //双边滤波
    QSlider* colorsigma_slider;//颜色范围标准差
    QSpinBox* colorsigma_SpinBox;//
    QSlider* spacesigma_slider;//颜色范围标准差
    QSpinBox* spacesigma_SpinBox;//
    QLabel* showinputcolorsigma;
    QLabel* showinputspacesigma;
    QLabel* showtaking;
    QPushButton* is_chose_bilateral;
    //调节亮度、对比度
    QSlider* liangdu_slider;//亮度滑动条
    QSpinBox* liangdu_SpinBox;//亮度微调框
    QLabel* show_liangdu;
    QSlider* duibidu_slider;//对比度滑动条
    QLineEdit* duibidu_LineEdit;
    QLabel* show_duibidu;
    //仿射变换
    QSpinBox* warpAffine_center_x_SpinBox;
    QSlider* warpAffine_center_x_slider;
    QSpinBox* warpAffine_center_y_SpinBox;
    QSlider* warpAffine_center_y_slider;
    QSpinBox* warpAffine_angle_SpinBox;
    QSlider* warpAffine_angle_slider;
    QLineEdit* warpAffine_scale_LineEdit;
    QSlider* warpAffine_scale_Slider;
    QLabel* warpAffine_show01;
    QLabel* warpAffine_show02;
    QLabel* warpAffine_show03;
    QLabel* warpAffine_show04;
    //极坐标与笛卡尔坐标转换
    QComboBox* logPolar_type;
    QSpinBox* logPolar_center_x_SpinBox;
    QSlider* logPolar_center_x_slider;
    QSpinBox* logPolar_center_y_SpinBox;
    QSlider* logPolar_center_y_slider;
    QSpinBox* logPolar_Magnitude_SpinBox;
    QSlider* logPolar_Magnitude_slider;
    QLabel* logPolar_show01;
    QLabel* logPolar_show02;
    QLabel* logPolar_show03;
    //基本二值化
    QComboBox* jiben_thresh_func;
    QLabel* show_input_thresh_value;
    QSlider* jibenthresh_slider;
    QSpinBox* jibenthresh_SpinBox;
    QLabel* show_max_input_thresh_value;
    QSlider* jibenthresh_max_slider;
    QSpinBox* jibenthresh_max_SpinBox;
    //THRESH_OTSU+基本二值化方法
    QComboBox* jiben_THRESH_OTSU_func;
    QLabel* show_max_input_THRESH_OTSU_value;
    QSlider* THRESH_OTSU_max_slider;
    QSpinBox* THRESH_OTSU_max_SpinBox;
    //THRESH_TRIANGLE + 基本二值化方法
    QComboBox* jiben_THRESH_TRIANGLE_func;
    QLabel* show_input_THRESH_TRIANGLE_value;
    QSlider* THRESH_TRIANGLE_max_slider;
    QSpinBox* THRESH_TRIANGLE_max_SpinBox;
    //自适应二值化
    QLabel* jiben_THRESH_adapt_show01;
    QLabel* jiben_THRESH_adapt_show02;
    QLabel* jiben_THRESH_adapt_show03;
    QLabel* jiben_THRESH_adapt_show04;
    QSlider* adapt_max_slider;
    QSpinBox* adapt_max_SpinBox;
    QSpinBox* adapt_C_SpinBox;
    QSlider* adapt_C_slider;
    QComboBox* jiben_THRESH_adapt_meth;
    QComboBox* jiben_THRESH_adapt_type;
    QComboBox* adapt_blocksize;
    //边缘检测 sobel
    QComboBox* sobel_fangxiang;
    QComboBox* sobel_ksize;
    QLineEdit* sobel_suofangyinzi_LineEdit;
    QSlider* sobel_suofangyinzi_Slider;
    QLabel* sobel_suofangyinzi_Label;
    QSlider* sobel_lianagdu_Slider;
    QSpinBox* sobel_lianagdu_SpinBox;
    QLabel* sobel_lianagdu_Label;

    //边缘检测 canny
    QLabel* show_input_thresh1;
    QLabel* show_input_thresh2;
    QComboBox* canny_ksize;
    QComboBox* L2gradient;
    QSlider* canny_slider_h;
    QSpinBox* canny_SpinBox_h;
    QSlider* canny_slider_l;
    QSpinBox* canny_SpinBox_l;

    //Laplacian算子
    QComboBox* Laplacian_ksize;
    QLabel* Laplacian_suofangyinzi_Label;
    QSlider* Laplacian_lianagdu_Slider;//亮度
    QSpinBox* Laplacian_lianagdu_SpinBox;
    QLabel* Laplacian_lianagdu_Label;
    QLineEdit* Laplacian_LineEdit;
    QSlider* Laplacian_suofangyinzi_Slider;

    //Scharr算子
    QComboBox* Scharr_fangxiang;
    QLineEdit* Scharr_suofangyinzi_LineEdit;
    QLabel* Scharr_suofangyinzi_Label;
    QSlider* Scharr_lianagdu_Slider;//亮度
    QSpinBox* Scharr_lianagdu_SpinBox;
    QLabel* Scharr_lianagdu_Label;
    QSlider* Scharr_suofangyinzi_Slider;

    //滤镜 颜色效果
    QComboBox* color_xiaoguo;
    //色彩空间转换
    QComboBox* color_kongjianzhuanhuan;
    //滤镜 手工画
    QComboBox* select_shougonghua;
    QSpinBox* shougonghua_SpinBox;
    QSlider* shougonghua_slider;
    QLabel* show_shougonghuayuzhi;
    QComboBox* sumiao_gauss_size;
    //艺术效果
    QComboBox* select_art;
    //光线效果
    QComboBox* select_light;
    //雕刻
    QComboBox*select_diaoke_func;
    QSpinBox* diaoke_canshu_SpinBox;
    QSlider* diaoke_canshu_slider;
    QLabel* diaoke_lab;
    //特殊效果
    QComboBox* teshuxiaoguo_select;
    QSpinBox* teshuxiaoguo_canshu_SpinBox;
    QSlider* teshuxiaoguo_canshu_slider;
    QLabel* teshuxiaoguo_lab;
    QLabel* teshuxiaoguo_lab_x;
    QLabel* teshuxiaoguo_lab_y;
    QSpinBox* teshuxiaoguo_center_x_SpinBox;
    QSlider* teshuxiaoguo_center_x_slider;
    QSpinBox* teshuxiaoguo_center_y_SpinBox;
    QSlider* teshuxiaoguo_center_y_slider;

    //轮廓检测
    QLabel* show_input_lunkuojiance_thresh1;
    QLabel* show_input_lunkuojiance_thresh2;
    QLabel* show_info_lunkuojiance;
    QLabel* show_lunkuojiance_mianji_shaixuan;
    QSlider* lunkuojiance_mianji_shaixuan_slider;
    QSpinBox* lunkuojiance_mianji_shaixuan_SpinBox;
    QSlider* lunkuojiance_slider_h;
    QSpinBox* lunkuojiance_SpinBox_h;
    QSlider* lunkuojiance_slider_l;
    QSpinBox* lunkuojiance_SpinBox_l;
    QComboBox* lunkuojiance_color_QComboBox;
    QComboBox* lunkuojiance_RETR_QComboBox;
    QComboBox* lunkuojiance_APPROX_QComboBox;
    QComboBox* lunkuojiance_lunkuocuxi_QComboBox;
    QCheckBox* lunkuojiance_isdraw_ju;
    QComboBox* canny_ksize_lunkuojiance;
    QComboBox* L2gradient_lunkuojiance;
    QCheckBox* lunkuojiance_draw_in_src;
    QCheckBox* lunkuojiance_draw_hull;
    QCheckBox* lunkuojiance_draw_poly;
    QCheckBox* lunkuojiance_draw_rectangle;
    QCheckBox* lunkuojiance_draw_circle;
    QCheckBox* lunkuojiance_draw_AreaRect;
    QCheckBox* lunkuojiance_draw_Ellipse;

    //霍夫线变换
    QLabel* Hough_canny_show1;
    QLabel* Hough_canny_show2;
    QLabel* Hough_canny_show3;
    QLabel* Hough_canny_show4;
    QLabel* Hough_canny_show5;
    QLabel* Hough_canny_show6;
    QSlider* Hough_canny_slider_h;
    QSpinBox* Hough_canny_SpinBox_h;
    QSlider* Hough_canny_slider_l;
    QSpinBox* Hough_canny_SpinBox_l;
    QSlider* Hough_line_slider;
    QSpinBox* Hough_line_SpinBox;
    QSlider* Hough_line_minlength_slider;
    QSpinBox* Hough_line_minlength_SpinBox;
    QSlider* Hough_line_maxLineGap_slider;
    QSpinBox* Hough_line_maxLineGap_SpinBox;
    QCheckBox* Hough_line_draw_in_src;
    QSpinBox* Hough_line_thickness;
    QCheckBox* Hough_line_color;

    //霍夫圆变换
    QLabel* Hough_circles_show02;
    QLabel* Hough_circles_show03;
    QLabel* Hough_circles_show04;
    QLabel* Hough_circles_show05;
    QLabel* Hough_circles_show06;
    QLabel* Hough_circles_show07;
    QSpinBox* Hough_circles_yuanxin_SpinBox;
    QSlider* Hough_circles_yuanxin_slider;
    QSlider* Hough_circles_canny_slider_h;
    QSpinBox* Hough_circles_canny_SpinBox_h;
    QSlider* Hough_circles_point_slider;
    QSpinBox* Hough_circles_point_SpinBox;
    QSlider* Hough_minRadius_slider;
    QSpinBox* Hough_minRadius_SpinBox;
    QSlider* Hough_maxRadius_slider;
    QSpinBox* Hough_maxRadius_SpinBox;
    QSpinBox* Hough_circles_thickness;
    QCheckBox* Hough_Radius_draw_in_src;

    //角点检测-像素级
    QLabel* Features_pixel_show01;
    QLabel* Features_pixel_show02;
    QLabel* Features_pixel_show03;
    QLabel* Features_pixel_show04;
    QLabel* Features_pixel_show05;
    QLabel* Features_pixel_show06;
    QLabel* Features_pixel_show07;
    QSpinBox* Features_pixel_maxCorners_SpinBox;
    QSlider* Features_pixel_maxCorners_slider;
    QSpinBox* Features_pixel_minDistance_SpinBox;
    QSlider* Features_pixel_minDistance_slider;
    QSpinBox* Features_pixel_qualityLevel_SpinBox;
    QSlider* Features_pixel_qualityLevel_slider;
    QSpinBox* Features_pixel_blockSize_SpinBox;
    QSlider* Features_pixel_blockSize_slider;
    QCheckBox* Features_pixel_draw_color;
    QSpinBox* Features_pixel_line_cuxi_SpinBox;
    QSpinBox* Features_pixel_line_banjing_SpinBox;
    QComboBox* Features_type;

    //k近邻
    QSpinBox* kmeans_num_SpinBox;
    QSlider* kmeans_num_slider;
    QSpinBox* kmeans_time_SpinBox;
    QSlider* kmeans_time_slider;
    QLabel* kmeans_show01;
    QLabel* kmeans_show02;
    QLabel* kmeans_show03;
    QPushButton* kmeans_isok;
    //布局
    QGridLayout* widget_layout;
    //函数
    void dstlabel_show(Mat dstImage_temp);
    void dstlabel_indexed8_show(Mat dstImage_temp);

private slots:
    void on_action_openpic_triggered();

    void on_action_savedst_triggered();

    void on_action_clear_triggered();

    void on_action_exit_triggered();

    void on_action_huanyuan_triggered();

    void domorphfun();

    void on_action_select_scope_triggered();

    void on_action_bulr_triggered();

    void blur_process_pic();//均值滤波处理图像

    void on_action_boxFilter_triggered();

    void boxFilter_process_pic();//方框滤波处理函数

    void on_action_GaussFilter_triggered();

    void gussFilter_process_pic();//高斯滤波处理函数

    void on_action_midian_triggered();

    void median_process_pic();//中值滤波处理函数

    void on_action_bilateral_triggered();

    void bilateral_process_pic();//双边滤波处理函数

    void close_other_obj();//关闭所有控件

    void on_action_equalizehist_triggered();//直方图均衡化

    void on_action_tiaojie_liangdu_duibidu_triggered();

    void change_liangdu_duibidu();

    void on_action_base_triggered();//基本二值化

    void jiben_thresh_process();

    void on_actionTHRESH_OTSU_triggered();

    void jiben_THRESH_OTSU_process();

    void on_actionTHRESH_TRIANGLE_triggered();

    void jiben_THRESH_TRIANGLE_process();

    void on_action_adapt_triggered();

    void jiben_THRESH_adapt_process();

    void on_actionSobel_triggered();

    void sobel_process();

    void on_actionCanny_triggered();

    void canny_process();

    void on_actionLaplacian_triggered();

    void Laplacian_process();

    void on_actionScharr_triggered();

    void Scharr_process();

    void on_action_color_lvjing_triggered();

    void color_lvjing_process();

    void on_action_change_secaikongjian_triggered();

    void color_kongjianzhuanhuan_process();

    void change_lvjing_shougonghua();

    void on_action_shougonghua_2_triggered();

    void on_action_art_triggered();

    void art_process();

    void on_action_light_triggered();

    void light_process();

    void on_action_diaoke_triggered();

    void diaoke_process();

    void on_action_help_triggered();

    void on_action_lunkuojiance_triggered();

    void lunkuojiance_process();

    void on_action_teshuxiaoguo_triggered();

    void teshuxiaoguo_process();

    void on_action_huofu_line_triggered();

    void Hough_line_process();

    void on_action_huofu_yuan_triggered();

    void Hough_circle_process();

    void on_action_jiaodianjiance_triggered();

    void Features_pixel();

    void on_action_warpAffine_triggered();

    void process_warpAffine();

    void on_action_logPolar_triggered();

    void process_logPolar();

    void on_actionK_Means_triggered();

    void process_keans();

private:
    Ui::base_process *ui;
protected:
    void dragEnterEvent(QDragEnterEvent* event);//拖动事件
    void dropEvent(QDropEvent* event);//放下事件
};

#endif // BASE_PROCESS_H
