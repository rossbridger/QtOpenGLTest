#ifndef CAMERA_H
#define CAMERA_H


#include <qmatrix4x4.h>
class Camera
{
public:
	Camera();
	void setPosition(QVector3D position);
	void setOrientation(QVector3D front, QVector3D right, QVector3D up);

	QMatrix4x4 getViewMatrix();
	QMatrix4x4 getProjectionMatrix();

	void setFov(float fov) { this->fov = fov; }
	void setAspectRatio(float aspect_ratio) { this->aspect_ratio = aspect_ratio; }
	void setNearPlane(float near) { this->near = near; }
	void setFarPlane(float far) { this->far = far; }
private:
	QVector3D front, right, up;
	QVector3D position;
	float fov;
	float aspect_ratio;
	float near;
	float far;
};

#endif // CAMERA_H
