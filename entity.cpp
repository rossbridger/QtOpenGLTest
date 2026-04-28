#include "entity.h"
#include <qopenglcontext.h>


Entity::Entity(QOpenGLContext* context)
	: QObject(context), QOpenGLExtraFunctions(context)
{
	modelMatrix.setToIdentity();
}

void Entity::setModelMatrix(QMatrix4x4 matrix)
{
	modelMatrix = matrix;
}
