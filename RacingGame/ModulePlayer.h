#pragma once
#include "Module.h"
#include "Globals.h"
#include "p2Point.h"

struct PhysVehicle3D;

#define MAX_ACCELERATION 1600.0f
#define TURN_DEGREES 8.0f * DEGTORAD
#define BRAKE_POWER 65.0f

class ModulePlayer : public Module
{
public:
	ModulePlayer(bool start_enabled = true);
	virtual ~ModulePlayer();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

public:

	PhysVehicle3D* vehicle;
	float turn;
	float acceleration;
	float brake;
};