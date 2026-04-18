#include <QHBoxLayout>
#include <qtextedit.h>
#include "mainwindow.h"
#include "openglwidget.h"

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	QWidget* side_pane = new QWidget();
	side_pane->setFixedWidth(200);
	side_pane->setLayout(new QVBoxLayout(side_pane));
	side_pane->layout()->addWidget(new QTextEdit());
	// TODO: design side pane to show configurable params.

	OpenGLWidget *widget = new OpenGLWidget();
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(side_pane);
	layout->addWidget(widget);
	widget->setMinimumWidth(800);
	widget->setMinimumHeight(600);
	setLayout(layout);
}

MainWindow::~MainWindow() {}
