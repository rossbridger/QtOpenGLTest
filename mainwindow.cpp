#include "mainwindow.h"
#include "openglwidget.h"
#include <QKeyEvent>

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	widget = new OpenGLWidget(this);
	widget->setMinimumWidth(1024);
	widget->setMinimumHeight(768);
}

MainWindow::~MainWindow() {}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	widget->resize(event->size());
	event->accept();
}
