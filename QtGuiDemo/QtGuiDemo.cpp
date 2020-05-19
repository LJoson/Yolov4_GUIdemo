#include "QtGuiDemo.h"
#include"common.h"

#include "QString"
#include "QFileDialog"
#include "QDir"
#include "QDateTime"
#include "QFileInfo"
#include "QProgressDialog"

#include <vector>
#include <queue>
#include <fstream>
#include <thread>
#include <atomic>
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable


QtGuiDemo::QtGuiDemo(QWidget *parent): 
	QMainWindow(parent)
{
	init_detector();
	ui.setupUi(this);
	initView();
}


QtGuiDemo::~QtGuiDemo()
{
	delete [] this->detector_;
	detector_ = nullptr;
	cleanMenu();
	cleanLayout();
}
void QtGuiDemo::init_detector()
{
	this->detector_ = new Detector(this->cfg_file_, this->weights_file_);
}
void QtGuiDemo::initView()
{
	/* �˵� */
	initMenu();
	/* ���� */
	initLayout();
}
/**
* @brief MainWindow::initMenu - ��ʼ���˵���
*/
void QtGuiDemo::initMenu()
{

	/* �ļ��˵�ui���� */
	openFileMenu = new QMenu(SQ("����ģ��"));

	openPictureAction = new QAction(SQ("ͼƬ�ļ�"), this);
	connect(openPictureAction, SIGNAL(triggered()), this, SLOT(openPictureSlot()));
	openFileMenu->addAction(openPictureAction);

	openVideoAction = new QAction(SQ("����Ƶ�ļ�"), this);
	connect(openVideoAction, SIGNAL(triggered()), this, SLOT(openVideoSlot()));
	openFileMenu->addAction(openVideoAction);

	openCaptureAction = new QAction(SQ("������ͷ"), this);
	connect(openCaptureAction, SIGNAL(triggered()), this, SLOT(openCaptureSlot()));
	openFileMenu->addAction(openCaptureAction);

	/* ������ģ����ӵ��˵����� */
	QMenuBar *menuBar = this->menuBar();
	menuBar->addMenu(openFileMenu);

	/* LOGO */
	this->setWindowTitle(SQ("Ŀ����ϵͳ"));
}

/**
* @brief MainWindow::initLayout - ��ʼ���ؼ�����
*/
void QtGuiDemo::initLayout()
{
	/* ͼƬ��ʾ��� */
	centralWidget = new QWidget(this);
	imageLabel = new  QLabel(centralWidget);
	imageLabel->setScaledContents(true);
	QSize labelSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	imageLabel->setFixedSize(labelSize);
	this->setCentralWidget(centralWidget);
	this->resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	this->show();
}

void QtGuiDemo::cleanMenu()
{
	if (openFileMenu)
	{
		delete openFileMenu;
		openFileMenu = NULL;
	}
	if (openCaptureAction)
	{
		delete openCaptureAction;
		openCaptureAction = NULL;
	}
	if (openVideoAction)
	{
		delete openVideoAction;
		openVideoAction = NULL;
	}

	if (openPictureAction)
	{
		delete openPictureAction;
		openPictureAction = NULL;
	}
}

void QtGuiDemo::cleanLayout()
{

	if (centralWidget)
	{
		delete centralWidget;
		centralWidget = NULL;
	}

}

void QtGuiDemo::setCVMat2QtLabel(const cv::Mat &image, QLabel *qLabel)
{
	cv::Mat showImg;
	cv::cvtColor(image, showImg, CV_BGR2RGB); 
	QImage qImg = QImage((const uchar *)(showImg.data),
		showImg.cols, showImg.rows,
		showImg.cols * showImg.channels(),
		QImage::Format_RGB888);
	qLabel->clear();                                //��տؼ�label
	qLabel->setPixmap(QPixmap::fromImage(qImg));    //label��ֵ
	qLabel->show();
	qLabel->update();
}


void QtGuiDemo::openPictureSlot()
{
	this->selectMode = 0;
	QString imageFilePath;
	imageFilePath = QFileDialog::getOpenFileName(
		this,
		SQ("ѡ��ͼƬ�ļ�"),
		QDir::currentPath(),
		"All files(*.jpg *JPG *.jpeg *.png *.PNG *.gif *.bmp)");																						// ���ݼ��
	if (QS(imageFilePath).empty())
	{
		qDebug()<< "The imageFilePath is not exit!!!\n";
		return;
	}

	inputImage = cv::imread(QS(imageFilePath));
	auto start = std::chrono::steady_clock::now();
	std::vector<bbox_t> result_vec = this->detector_->detect(inputImage);
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> spent = end - start;
	qDebug() << " Time: " << spent.count() << " sec \n";

	// result_vec = detector->tracking_id(result_vec);    // comment it - if track_id is not required
	draw_boxes(inputImage, result_vec, obj_names, spent.count());

	this->setCVMat2QtLabel(this->inputImage, this->imageLabel);
}


void QtGuiDemo::openVideoSlot() 
{
	
	QString vedioFilePath;
	vedioFilePath = QFileDialog::getOpenFileName(
		this,
		SQ("ѡ����Ƶ�ļ�"),
		QDir::currentPath(),
		"All files(*.mp4 *.MP4 *.AVI *.avi)");																						// ���ݼ��
	if (QS(vedioFilePath).empty())
	{
		qDebug() << "The VedioFilePath is not exit!!!\n";
		return;
	}
	
	if (this->capture.isOpened())
	{
		this->capture.release();
	}
	if (!this->capture.open(QS(vedioFilePath)))
	{
			qDebug() << "open video false 0";
	}
	else
	{
		this->selectMode = 1;
		this->isOpenVedio = true;
		qDebug() << "open video file successful 0\n";
	}
}


// ��������ͷ��
void QtGuiDemo:: openCaptureSlot()
{
	if (openCaptureAction->text() == SQ("������ͷ"))
	{
		this->selectMode = 2;
		if (!this->capture.open(0))
		{
			if (!this->capture.open(0)) {
				// capture.open(0);
			}
			qDebug() << "open video false 0";

		}
		qDebug() << "open video successful 0\n";
		this->isOpenVedio = true;
		this->openCaptureAction->setText(SQ("�ر�����ͷ"));

	}
	else
	{
		openCaptureAction->setText(SQ("������ͷ"));
		this->capture.release();
		this->isOpenVedio = false;
	}
}


// ʵʱ��ʾ��Ƶ��
void QtGuiDemo::paintEvent(QPaintEvent *e)
{

	if (this->isOpenVedio && (this->selectMode>0))
	{
		this->capture >> this->inputImage;
		this->setCVMat2QtLabel(inputImage, this->imageLabel);


		auto start = std::chrono::steady_clock::now();
		std::vector<bbox_t> result_vec = this->detector_->detect(this->inputImage);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> spent = end - start;
		qDebug() << " Time: " << spent.count() << " sec \n";

		// result_vec = detector->tracking_id(result_vec);    // comment it - if track_id is not required
		draw_boxes(this->inputImage, result_vec, obj_names, spent.count());

		this->setCVMat2QtLabel(inputImage, this->imageLabel);
	}
	
	else if (!this->isOpenVedio && (this->selectMode>0))
	{
		this->imageLabel->clear();
	}
}

std::vector<std::string> QtGuiDemo::objects_names_from_file(std::string const filename) {
	std::ifstream file(filename);
	std::vector<std::string> file_lines;
	if (!file.is_open()) return file_lines;
	for (std::string line; getline(file, line);) file_lines.push_back(line);
	std::cout << "object names loaded \n";
	return file_lines;
}

cv::Scalar QtGuiDemo::obj_id_to_color(int obj_id) {
	int const colors[6][3] = { { 1,0,1 },{ 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };
	int const offset = obj_id * 123457 % 6;
	int const color_scale = 150 + (obj_id * 123457) % 100;
	cv::Scalar color(colors[offset][0], colors[offset][1], colors[offset][2]);
	color *= color_scale;
	return color;
}


