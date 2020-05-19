#pragma once

#ifndef QT_H
#define QT_H


#include <QtWidgets/QMainWindow>
#include "ui_QtGuiDemo.h"

#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QObject>
#include <QMetaEnum>
#include <QDebug>
#include <QIcon>

#include <string>
#include <qstring.h>
#include <opencv2/opencv.hpp>
#include "yolo_v2_class.hpp"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

class QtGuiDemo : public QMainWindow
{
	Q_OBJECT

public:
	QtGuiDemo(QWidget *parent = Q_NULLPTR);
	~QtGuiDemo();
	/**opencv mat ͼ��תΪqImage, ��ʾ��qLabel��**/
	void setCVMat2QtLabel(const cv::Mat &image, QLabel *qLabel);
	std::vector<std::string> objects_names_from_file(std::string const filename);
	cv::Scalar obj_id_to_color(int obj_id);
	void init_detector();
// QT �����ʼ���Ͳ�
public slots:
	/* ����*/
	void initView();
	void initMenu();
	void cleanMenu();
	void initLayout();
	void cleanLayout();
	/* �˵��ۺ��� */
	void openVideoSlot();				// �򿪱�����Ƶ�ļ���
	void openPictureSlot();				// ��ͼƬ��
	void openCaptureSlot();
	void paintEvent(QPaintEvent *e);	// ʵʱ��ʾ��

private:
	Ui::QtGuiDemoClass ui;
	/* �˵��� */
	QMenu *openFileMenu;
	
	QAction *openCaptureAction;			// ������ͷ
	QAction *openVideoAction;			// �򿪱�����Ƶ�ļ�
	QAction *openPictureAction;			// ��ͼƬ
	QLabel *imageLabel;					// ��ʾͼƬLabel�ؼ�

	QWidget *centralWidget;

	cv::VideoCapture capture;
	cv::Mat inputImage;
	cv::Mat outputImage;

	std::string  names_file_ = "../data/coco.names";
	std::string  cfg_file_ = "../cfg/yolov4.cfg";
	std::string  weights_file_ = "../model/yolov4.weights";

	std::vector<std::string> obj_names = objects_names_from_file(names_file_);
	Detector *detector_;

	// ��Ƶ����
	bool isOpenVedio = false;
	// ѡ��ģʽ 0. ��ͼƬ�ļ� 2.�򿪱�����Ƶ 3.�򿪱�������ͷ
	int selectMode;
};

#endif // QT_H
