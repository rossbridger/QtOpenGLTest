#include "mainwindow.h"
#include "openglwidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	OpenGLWidget *widget = new OpenGLWidget(this);
	setCentralWidget(widget);
	widget->setMinimumWidth(800);
	widget->setMinimumHeight(600);
}

MainWindow::~MainWindow() {}

