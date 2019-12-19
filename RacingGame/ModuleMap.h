#ifndef __ModuleMap_H__
#define __ModuleMap_H__

#include "Module.h"
#include "SDL/include/SDL.h"

#define DISTANCE_X_RATIO 200.0f
#define DISTANCE_Z_RATIO 300.0f

class Application;

struct Pillars
{
	vec3 pillars_pos;
	vec3 pillar_size;
};

struct Fan
{
	vec3 fan_pos;
	vec3 fan_size1;
	vec3 fan_size2;
	vec3 joint_size;
	int	mass;	
	btQuaternion rotation;
	btRigidBody* body;
};

struct WreckingBall
{
	btVector3 base_pos;
	btVector3 base_size;
	btVector3 ball_pos;	
	btScalar ball_size; //Radius
	btScalar mass;	
	btRigidBody* body;
};

class ModuleMap : public Module
{
public:

	ModuleMap(bool start_enabled = true);

	// Destructor
	~ModuleMap();
	
	bool Start();
	bool CleanUp();
	void OnCollision(PhysBody3D* body1, PhysBody3D* body2) override;	
	update_status Update(float dt) override;
	update_status PostUpdate(float dt) override; //TODO why override?

	int GetLaps() const;
	void ResetGame();
	PhysBody3D* GetLastWaypoint() const;

	btRigidBody* CreateFloor(const btVector3 size, const btVector3 pos);
	btRigidBody* CreatePillars(const Pillars pillar_info[], const int size, const vec2 dist_origin);
	btRigidBody* CreateRamps(const btVector3 size, const btVector3 pos, const float angle);
	btRigidBody* CreateFan(Fan fan);
	btRigidBody* CreateWreckingBall(WreckingBall wreckingball);
	PhysBody3D*  CreateSensor(vec3 shapePos, vec3 rbPos, btVector3 rbHalfDimensions, float rbRotation);
	
private:

	Pillars pillar[928];	
	Fan fan;
	WreckingBall wrecking_ball;	
	
	PhysBody3D* last_waypoint;
	bool  waypoint_flags[4];
	int laps=0;

	p2List<btDefaultMotionState*>		motions;
	p2List<btCollisionShape*>			shapes;

	p2DynArray<Primitive*>				primitives;
	p2DynArray<PhysBody3D*>				waypoints;

};




#endif // __ModuleMap_H__


