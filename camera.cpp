#include "camera.h"

Camera::Camera()
{

}

void Camera::setPosition(QVector3D position)
{
	this->position = position;
}

void Camera::setOrientation(QVector3D front, QVector3D right, QVector3D up)
{
	this->front = front;
	this->right = right;
	this->up = up;
}

QMatrix4x4 Camera::getViewMatrix()
{
	QMatrix4x4 viewMatrix;
	viewMatrix.setToIdentity();
	viewMatrix.lookAt(position, position + front, up);
	return viewMatrix;
}

QMatrix4x4 Camera::getProjectionMatrix()
{
	QMatrix4x4 projectionMatrix;
	projectionMatrix.setToIdentity();
	projectionMatrix.perspective(fov, aspect_ratio, near, far);
	return projectionMatrix;
}
