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
	Color pillar_color;
	vec3 pillar_properties;
};


class ModuleMap : public Module
{
public:

	ModuleMap(bool start_enabled = true);

	// Destructor
	~ModuleMap();
	void OnCollision(PhysBody3D* body1, PhysBody3D* body2) override;
	bool Start();
	bool CleanUp();

	update_status PostUpdate(float dt) override; //TODO why override?

private:

	//Pillars data
	Pillars pillar[928];
	
	float pillar_radius;
	float pillar_height;
	float pillar_mass;

	p2DynArray<Primitive*> primitives;
	p2DynArray<PhysBody3D*> waypoints;

};

#endif // __ModuleMap_H__


