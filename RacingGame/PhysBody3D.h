#ifndef __PhysBody3D_H__
#define __PhysBody3D_H__

#include "p2DynArray.h"
#include "glmath.h"

class btRigidBody;
class btCollisionShape;
struct btDefaultMotionState;
class Module;

class Sphere;
class Cube;
class Cylinder;
class Plane;

class Primitive;

class PhysBody3D
{
public:
	PhysBody3D();
	PhysBody3D(btRigidBody*body);
	~PhysBody3D();

	void SetBody(Sphere* primitive, float mass);
	void SetBody(btCollisionShape* shape, Primitive* parent, float mass);

	bool HasBody() const;
	btRigidBody* GetBody() const;

	void GetTransform(float* matrix) const;
	void SetTransform(const float* matrix) const;
	void SetPos(float x, float y, float z);
	void SetAsSensor(bool is_sensor);

	void SetSpeed(vec3 speed);
	void Push(vec3 force);
	void Stop();

private:

	btRigidBody* body;
	btCollisionShape* colShape;
	btDefaultMotionState* motionState;

	bool is_sensor;
public:
	Primitive* parentPrimitive;
	p2DynArray<Module*> collision_listeners;
};

#endif // __PhysBody3D_H__