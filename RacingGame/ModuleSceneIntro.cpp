#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "ModulePlayer.h"
#include "PhysVehicle3D.h"
#include "ModuleAudio.h"

ModuleSceneIntro::ModuleSceneIntro(bool start_enabled) : Module(start_enabled)
{	
	best_lap_time = 0.0f;
	victory_played = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(vec3(0, 0, 0));	

	//Loading Audio
	App->audio->PlayMusic("Audio/music.ogg", 4.0f);
	App->audio->LoadFx("Audio/lap.wav");			//1
	App->audio->LoadFx("Audio/checkpoint.wav");		//2
	App->audio->LoadFx("Audio/win.wav");			//3
	
	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	
	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
		App->camera->debugcamera = !App->camera->debugcamera;

	return UPDATE_CONTINUE;
}

update_status ModuleSceneIntro::PostUpdate(float dt)
{
	//Sets the title information 
	char title[250];
	if (App->map->GetLaps() >= 3)
	{		
		if (!victory_played) {
			App->audio->PlayFx(3);
			victory_played = true;
		}		
		sprintf_s(title, "%.1f Km/h || Best Lap Time: %.2f || VICTORY!", App->player->vehicle->GetKmh(), best_lap_time);
	}
	else
	{
		victory_played = false;
		sprintf_s(title, "%.1f Km/h || Total Laps: %i || Lap Time: %.2f || Best Lap Time: %.2f", App->player->vehicle->GetKmh(), App->map->GetLaps(), lap_time.Read() * 0.001f, best_lap_time);
	}
	App->window->SetTitle(title);

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D * body1, PhysBody3D * body2)
{
	Color color = Color((float)(std::rand() % 255) / 255.f, (float)(std::rand() % 255) / 255.f, (float)(std::rand() % 255) / 255.f);

	body1->parentPrimitive->color = color;
	body2->parentPrimitive->color = color;

}

void ModuleSceneIntro::CalculateBestLap(Timer* lap_time)
{
	float newtime = lap_time->Read() * 0.001f;
	if (newtime < best_lap_time||best_lap_time==0.00f)best_lap_time = newtime;

}