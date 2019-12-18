#include "PhysVehicle3D.h"
#include "Primitive.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

// ----------------------------------------------------------------------------
VehicleInfo::~VehicleInfo()
{
	//if(wheels != NULL)
		//delete wheels;
}

// ----------------------------------------------------------------------------
PhysVehicle3D::PhysVehicle3D(btRigidBody* body, btRaycastVehicle* vehicle, const VehicleInfo& info) : PhysBody3D(body), vehicle(vehicle), info(info)
{
}

// ----------------------------------------------------------------------------
PhysVehicle3D::~PhysVehicle3D()
{
	delete vehicle;

}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Render()
{
	// Render Wheels -------------------------------
	for (int i = 0; i < vehicle->getNumWheels(); ++i)
	{		
		Cylinder wheel(info.wheels[i].radius, info.wheels[0].width);
		wheel.color = White;
		vehicle->updateWheelTransform(i);
		vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(&wheel.transform);

		wheel.Render();
	}	

	// Render Chassis -------------------------------
	for (int i = 0; i < 5; ++i)
	{
		vec3 chassis_size = { info.chassis[i].chassis_size.x, info.chassis[i].chassis_size.y, info.chassis[i].chassis_size.z };
		Cube chassis(chassis_size);
		chassis.color = info.chassis[i].chassis_color;

		vehicle->getChassisWorldTransform().getOpenGLMatrix(&chassis.transform);
		btQuaternion q = vehicle->getChassisWorldTransform().getRotation();
		btVector3 offset(info.chassis[i].chassis_offset.x, info.chassis[i].chassis_offset.y, info.chassis[i].chassis_offset.z);
		offset = offset.rotate(q.getAxis(), q.getAngle());

		chassis.transform.M[12] += offset.getX();
		chassis.transform.M[13] += offset.getY();
		chassis.transform.M[14] += offset.getZ();

		chassis.Render();
	}
	
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::ApplyEngineForce(float force)
{
	for (int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if (info.wheels[i].drive == true)
		{
			vehicle->applyEngineForce(force, i);
		}
	}
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Brake(float force)
{
	for (int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if (info.wheels[i].brake == true)
		{
			vehicle->setBrake(force, i);
		}
	}
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Turn(float degrees)
{
	for (int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if (info.wheels[i].steering == true)
		{
			vehicle->setSteeringValue(degrees, i);
		}
	}
}

// ----------------------------------------------------------------------------
float PhysVehicle3D::GetKmh() const
{
	return vehicle->getCurrentSpeedKmHour();
}