#include "entity.h"
#include <qopenglcontext.h>


Entity::Entity(QOpenGLContext* context)
	: QObject(context), QOpenGLExtraFunctions(context)
{
}

void Entity::setModelMatrix(QMatrix4x4 matrix)
{
	modelMatrix = matrix;
}
