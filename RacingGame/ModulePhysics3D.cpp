#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "PhysBody3D.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "glut/glut.h"

#ifdef _DEBUG
#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif

ModulePhysics3D::ModulePhysics3D(bool start_enabled) : Module(start_enabled), world(nullptr)
{
	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	debug_draw = new DebugDrawer();
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	delete debug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;
}

// Render not available yet----------------------------------
bool ModulePhysics3D::Init()
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

// ---------------------------------------------------------
bool ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);

	// Big rectangle as ground
	{
		btCollisionShape* colShape = new btBoxShape(btVector3(200.0f, 2.0f, 200.0f));

		mat4x4 glMatrix = IdentityMatrix;
		glMatrix.translate(0.f, -2.f, 0.f);
		btTransform startTransform;
		startTransform.setFromOpenGLMatrix(&glMatrix);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		motions.add(myMotionState);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);

		btRigidBody* body = new btRigidBody(rbInfo);
		world->addRigidBody(body);
	}
	//// Big ramp as 2nd ground
	//{
	//	btCollisionShape* colShape2 = new btBoxShape(btVector3(200.0f, 2.0f, 200.0f));

	//	mat4x4 glMatrix2 = IdentityMatrix;
	//	glMatrix2.translate(0.f, -2.f, -200.f);
	//	glMatrix2.rotate(20.0f, { 1,0,0 });
	//	btTransform startTransform2;
	//	startTransform2.setFromOpenGLMatrix(&glMatrix2);

	//	btDefaultMotionState* myMotionState2 = new btDefaultMotionState(startTransform2);
	//	motions.add(myMotionState2);
	//	btRigidBody::btRigidBodyConstructionInfo rbInfo2(0.0f, myMotionState2, colShape2);

	//	btRigidBody* body2 = new btRigidBody(rbInfo2);
	//	world->addRigidBody(body2);
	//}

	//Vehicle inicializations
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);
	return true;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PreUpdate(float dt)
{

	world->stepSimulation(dt, 15);
	

	for (int n = 0; n < world->getDispatcher()->getNumManifolds(); n++)
	{
		btPersistentManifold* manifold = world->getDispatcher()->getManifoldByIndexInternal(n);
		if (manifold->getNumContacts() > 0)
		{
			PhysBody3D* body1 = (PhysBody3D*)manifold->getBody0()->getUserPointer();
			PhysBody3D* body2 = (PhysBody3D*)manifold->getBody1()->getUserPointer();

			if (body1 != nullptr && body2 != nullptr)
			{
				for (uint n = 0; n < body1->collision_listeners.Count(); n++)
				{
					body1->collision_listeners[n]->OnCollision(body1, body2);
				}

				for (uint n = 0; n < body2->collision_listeners.Count(); n++)
				{
					body2->collision_listeners[n]->OnCollision(body2, body1);
				}
			}
			
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::Update(float dt)
{
	if (App->debug == true)
	{
		glDisable(GL_LIGHTING);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		world->debugDrawWorld();
		glEnable(GL_LIGHTING);
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	// Remove from the world all collision bodies
	for (int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	// Remove constraints
	for (int i = world->getNumConstraints() - 1; i >= 0; i--)
	{
		btTypedConstraint* constraint = world->getConstraint(i);
		world->removeConstraint(constraint);
	}

	// Clear motions 
	for (p2List_item<btDefaultMotionState*>* item = motions.getFirst(); item; item = item->next)
		delete item->data;

	motions.clear();

	// Clear shapes 
	for (p2List_item<btCollisionShape*>* item = shapes.getFirst(); item; item = item->next)
		delete item->data;

	shapes.clear();
	//TODO REMOVE CONSTRAINTS

	delete phys_vehicle;
	delete vehicle_raycaster;
	delete world;

	return true;
}

PhysBody3D* ModulePhysics3D::RayCast(const vec3& Origin, const vec3& Direction, vec3& HitPoint)
{

	vec3 Dir = normalize(Direction);

	btVector3 Start = btVector3(Origin.x, Origin.y, Origin.z);
	btVector3 End = btVector3(Origin.x + Dir.x * 1000.f, Origin.y + Dir.y * 1000.f, Origin.z + Dir.z * 1000.f);

	btCollisionWorld::ClosestRayResultCallback RayCallback(Start, End);

	// Perform raycast
	world->rayTest(Start, End, RayCallback);
	if (RayCallback.hasHit()) {

		HitPoint = vec3(RayCallback.m_hitPointWorld.x(), RayCallback.m_hitPointWorld.y(), RayCallback.m_hitPointWorld.z());
		return (PhysBody3D*)RayCallback.m_collisionObject->getUserPointer();
	}
	return nullptr;
}

void ModulePhysics3D::AddBodyToWorld(btRigidBody* body)
{
	world->addRigidBody(body);	
}

void ModulePhysics3D::RemoveBodyFromWorld(btRigidBody* body)
{
	world->removeRigidBody(body);
}

// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	line.origin.Set(from.getX(), from.getY(), from.getZ());
	line.destination.Set(to.getX(), to.getY(), to.getZ());
	line.color.Set(color.getX(), color.getY(), color.getZ());
	line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.transform.translate(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes)debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return mode;
}

// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info)
{	
	//Creation of the chasis 
	btCompoundShape* comShape = new btCompoundShape();
	shapes.add(comShape);

	//Creation of the main Shape ------------------------
	btCollisionShape* mainShape = new btBoxShape(btVector3(info.chassis[0].chassis_size.x * 0.5f, info.chassis[0].chassis_size.y * 0.5f, info.chassis[0].chassis_size.z * 0.5f));
	shapes.add(mainShape);

	btTransform mainTrans;
	mainTrans.setIdentity();
	mainTrans.setOrigin(btVector3(info.chassis[0].chassis_offset.x, info.chassis[0].chassis_offset.y, info.chassis[0].chassis_offset.z));

	comShape->addChildShape(mainTrans, mainShape);

	
	//Creation of the cabin Shape ---------------------
	btCollisionShape* cabinShape = new btBoxShape(btVector3(info.chassis[1].chassis_size.x * 0.5f, info.chassis[1].chassis_size.y * 0.5f, info.chassis[1].chassis_size.z * 0.5f));
	shapes.add(cabinShape);

	btTransform cabinTrans;
	cabinTrans.setIdentity();
	cabinTrans.setOrigin(btVector3(info.chassis[1].chassis_offset.x, info.chassis[1].chassis_offset.y, info.chassis[1].chassis_offset.z));

	comShape->addChildShape(cabinTrans, cabinShape);

	//Creation of the flap Shape -------------------------
	btCollisionShape* flapShape = new btBoxShape(btVector3(info.chassis[2].chassis_size.x * 0.5f, info.chassis[2].chassis_size.y * 0.5f, info.chassis[2].chassis_size.z * 0.5f));
	shapes.add(flapShape);

	btTransform flapTrans;
	flapTrans.setIdentity();
	flapTrans.setOrigin(btVector3(info.chassis[2].chassis_offset.x, info.chassis[2].chassis_offset.y, info.chassis[2].chassis_offset.z));

	comShape->addChildShape(flapTrans, flapShape);

	//Creation of the bar left Shape -------------------------
	btCollisionShape* barleftShape = new btBoxShape(btVector3(info.chassis[3].chassis_size.x * 0.5f, info.chassis[3].chassis_size.y * 0.5f, info.chassis[3].chassis_size.z * 0.5f));
	shapes.add(barleftShape);

	btTransform barleftTrans;
	barleftTrans.setIdentity();
	barleftTrans.setOrigin(btVector3(info.chassis[3].chassis_offset.x, info.chassis[3].chassis_offset.y, info.chassis[3].chassis_offset.z));

	comShape->addChildShape(barleftTrans, barleftShape);

	//Creation of the bar right Shape -------------------------
	btCollisionShape* barrightShape = new btBoxShape(btVector3(info.chassis[4].chassis_size.x * 0.5f, info.chassis[4].chassis_size.y * 0.5f, info.chassis[4].chassis_size.z * 0.5f));
	shapes.add(barrightShape);

	btTransform barrightTrans;
	barrightTrans.setIdentity();
	barrightTrans.setOrigin(btVector3(info.chassis[4].chassis_offset.x, info.chassis[4].chassis_offset.y, info.chassis[4].chassis_offset.z));

	comShape->addChildShape(barrightTrans, barrightShape);
	//------------------------------------------------------

	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	comShape->calculateLocalInertia(info.mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(info.mass, myMotionState, comShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	body->setActivationState(DISABLE_DEACTIVATION);
	world->addRigidBody(body);
	btRaycastVehicle::btVehicleTuning tuning;
	tuning.m_frictionSlip = info.frictionSlip;
	tuning.m_maxSuspensionForce = info.maxSuspensionForce;
	tuning.m_maxSuspensionTravelCm = info.maxSuspensionTravelCm;
	tuning.m_suspensionCompression = info.suspensionCompression;
	tuning.m_suspensionDamping = info.suspensionDamping;
	tuning.m_suspensionStiffness = info.suspensionStiffness;

	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, body, vehicle_raycaster);

	vehicle->setCoordinateSystem(0, 1, 2);

	for (int i = 0; i < info.num_wheels; ++i)
	{
		btVector3 conn(info.wheels[i].connection.x, info.wheels[i].connection.y, info.wheels[i].connection.z);
		btVector3 dir(info.wheels[i].direction.x, info.wheels[i].direction.y, info.wheels[i].direction.z);
		btVector3 axis(info.wheels[i].axis.x, info.wheels[i].axis.y, info.wheels[i].axis.z);

		vehicle->addWheel(conn, dir, axis, info.wheels[i].suspensionRestLength, info.wheels[i].radius, tuning, info.wheels[i].front);

	}
	// ---------------------
	
	phys_vehicle = new PhysVehicle3D(body, vehicle, info);
	phys_vehicle->collision_listeners.PushBack(App->map);
	body->setUserPointer(phys_vehicle);
	world->addVehicle(vehicle);

	return phys_vehicle;
}

PhysVehicle3D* ModulePhysics3D::GetVehicle()const
{
	return phys_vehicle;
}


void ModulePhysics3D::CreateMap(const Pillars pillar_info[], float radius, int size, vec2 dist_origin)
{
	for (int i = 0; i < size/2; i++)
	{
		btCollisionShape* mapShape = new btCylinderShape(btVector3(pillar_info[i].pillar_size.x * 0.5f, pillar_info[i].pillar_size.y * 0.5f, pillar_info[i].pillar_size.z * 0.5f));
		shapes.add(mapShape);

		btTransform mapTrans;
		mapTrans.setIdentity();
		mapTrans.setOrigin(btVector3(pillar_info[i].pillars_pos.x- dist_origin.x, pillar_info[i].pillars_pos.y, pillar_info[i].pillars_pos.z- dist_origin.y));

		btVector3 localInertia(0, 0, 0);
		mapShape->calculateLocalInertia(100.0f, localInertia);

		btDefaultMotionState* mapMotionState = new btDefaultMotionState(mapTrans);
		motions.add(mapMotionState);
		btRigidBody::btRigidBodyConstructionInfo mapInfo(0.0f, mapMotionState, mapShape, localInertia);

		btRigidBody* body = new btRigidBody(mapInfo);
		body->setContactProcessingThreshold(BT_LARGE_FLOAT);
		body->setActivationState(DISABLE_DEACTIVATION);
		world->addRigidBody(body);
	}	

}

void ModulePhysics3D::CreateRamps(const Ramps ramp_info[]) 
{
	// First Ramp
	btCollisionShape* rampShape1 = new btBoxShape(btVector3(ramp_info[0].ramp_size.x * 0.5f, ramp_info[0].ramp_size.y * 0.5f, ramp_info[0].ramp_size.z * 0.5f));
	shapes.add(rampShape1);

	mat4x4 rampMatrix = IdentityMatrix;
	rampMatrix.translate(ramp_info[0].ramp_position.x, ramp_info[0].ramp_position.y, ramp_info[0].ramp_position.z);
	rampMatrix.rotate(85.0f, { 0,0,1 });
	btTransform rampTransform;
	rampTransform.setFromOpenGLMatrix(&rampMatrix);

	btDefaultMotionState* rampMotionState = new btDefaultMotionState(rampTransform);
	motions.add(rampMotionState);
	btRigidBody::btRigidBodyConstructionInfo rampInfo(0.0f, rampMotionState, rampShape1);

	btRigidBody* body = new btRigidBody(rampInfo);
	world->addRigidBody(body);

	// Second Ramp
	btCollisionShape* rampShape2 = new btBoxShape(btVector3(ramp_info[1].ramp_size.x * 0.5f, ramp_info[1].ramp_size.y * 0.5f, ramp_info[1].ramp_size.z * 0.5f));
	shapes.add(rampShape2);

	mat4x4 rampMatrix2 = IdentityMatrix;
	rampMatrix2.translate(ramp_info[1].ramp_position.x, ramp_info[1].ramp_position.y, ramp_info[1].ramp_position.z);
	rampMatrix2.rotate(-85.0f, { 0,0,1 });
	btTransform rampTransform2;
	rampTransform2.setFromOpenGLMatrix(&rampMatrix2);

	btDefaultMotionState* rampMotionState2 = new btDefaultMotionState(rampTransform2);
	motions.add(rampMotionState2);
	btRigidBody::btRigidBodyConstructionInfo rampInfo2(0.0f, rampMotionState2, rampShape2);

	btRigidBody* body2 = new btRigidBody(rampInfo2);
	world->addRigidBody(body2);
}

void ModulePhysics3D::AddConstraintP2P(const Primitive& bodyA, const Primitive& bodyB, const btVector3& pivotInA, const btVector3& pivotInB)
{
	btTypedConstraint* p2p = new btPoint2PointConstraint(*bodyA.body.GetBody(), *bodyB.body.GetBody(), pivotInA, pivotInB);
	world->addConstraint(p2p);
}

btRigidBody* ModulePhysics3D::AddConstraintSlider(const Fan fan)
{
	btRigidBody* bodyB = 0;
	btCollisionShape* shape1 = new btBoxShape(btVector3(fan.fan_size1.x*0.5f, fan.fan_size1.y*0.5f, fan.fan_size1.z*0.5f));
	btCollisionShape* shape3 = new btBoxShape(btVector3(fan.fan_size2.x * 0.5f, fan.fan_size2.y * 0.5f, fan.fan_size2.z * 0.5f));
	btCollisionShape* shape2 = new btCylinderShape(btVector3(fan.joint_size.x, fan.joint_size.y, fan.joint_size.z));
	
	btCompoundShape* fanShape = new btCompoundShape();
	fanShape->addChildShape(btTransform::getIdentity(), shape1);
	fanShape->addChildShape(btTransform::getIdentity(), shape2);
	fanShape->addChildShape(btTransform::getIdentity(), shape3);

	btScalar mass = 150;
	btVector3 localInertia;
	fanShape->calculateLocalInertia(mass, localInertia);
	
	btRigidBody::btRigidBodyConstructionInfo fanInfo(mass, 0, fanShape, localInertia);
	fanInfo.m_startWorldTransform.setOrigin(btVector3(fan.fan_pos.x, fan.fan_pos.y, fan.fan_pos.z));	
	fanInfo.m_startWorldTransform.setRotation(fan.rotation);

	btRigidBody* body = new btRigidBody(fanInfo);
	body->setLinearFactor(btVector3(0, 0, 0));
	btHingeConstraint* hinge = new btHingeConstraint(*body, btVector3(0, 0, 0), btVector3(0, 1, 0), true);
	
	world->addConstraint(hinge);
	bodyB = body;				
	hinge->enableAngularMotor(true, -2.0f, 500);	

	world->addRigidBody(body);

	return body;

}

