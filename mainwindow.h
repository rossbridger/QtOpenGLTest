#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "openglwidget.h"
#include <QWidget>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	~MainWindow();
protected:
	virtual void resizeEvent(QResizeEvent *event) override;
private:
	OpenGLWidget *widget;
};
#endif // MAINWINDOW_H
