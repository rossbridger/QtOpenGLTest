#include "mainwindow.h"
#include "openglwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	OpenGLWidget *widget = new OpenGLWidget(this);
	widget->setMinimumWidth(1024);
	widget->setMinimumHeight(768);
	setCentralWidget(widget);
}

MainWindow::~MainWindow() {}
