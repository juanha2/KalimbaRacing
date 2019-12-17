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

struct Ramps
{
	vec3 ramp_position;
	vec3 ramp_size;
};

struct Fan
{	
	vec3 fan_pos;

	vec3 fan_size;
	vec3 joint_size;	
};
class ModuleMap : public Module
{
public:

	ModuleMap(bool start_enabled = true);

	// Destructor
	~ModuleMap();

	bool Start();
	bool CleanUp();

	update_status Update(float dt) override;
	update_status PostUpdate(float dt) override; //TODO why override?

private:

	//Pillars data
	Pillars pillar[928];
	float pillar_radius;
	float pillar_height;
	float pillar_mass;

	//Ramps data
	Ramps ramp[2];
	Fan fan;

	btRigidBody* Fan_body;
	p2DynArray<Primitive*> primitives;

};

#endif // __ModuleMap_H__


