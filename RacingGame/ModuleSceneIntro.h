#pragma once
#include "Module.h"
#include "p2DynArray.h"
#include "Globals.h"#include "Timer.h"

class Primitive;
class  PhysBody3D;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update(float dt) override;
	update_status PostUpdate(float dt) override;

	void OnCollision(PhysBody3D* body1, PhysBody3D* body2) override;

	bool CleanUp();

	void CalculateBestLap(Timer* lap_time);


public:

	Timer lap_time;
	float best_lap_time;

private:

	bool victory_played;
};
