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
	//App->audio->PlayMusic("Audio/music.ogg", 4.0f);
	App->audio->LoadFx("Audio/lap.wav");
	
	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	
	return true;
}

void ModuleSceneIntro::HandleDebugInput()
{
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		DebugSpawnPrimitive(new Sphere());
	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
		DebugSpawnPrimitive(new Cube());
	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
		DebugSpawnPrimitive(new Cylinder());
	if (App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN)
		for (uint n = 0; n < primitives.Count(); n++)
			primitives[n]->SetPos((float)(std::rand() % 40 - 20), 10.f, (float)(std::rand() % 40 - 20));
	if (App->input->GetKey(SDL_SCANCODE_5) == KEY_DOWN)
		for (uint n = 0; n < primitives.Count(); n++)
			primitives[n]->body.Push(vec3((float)(std::rand() % 500) - 250, 500, (float)(std::rand() % 500) - 250));

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		
		const vec2 mousePos(((float)App->input->GetMouseX() / (float)App->window->Width()) * 2.f - 1.f,
			-((float)App->input->GetMouseY() / (float)App->window->Height()) * 2.f + 1.f);
		const vec4 rayEye = inverse(App->renderer3D->ProjectionMatrix) * vec4(mousePos.x, mousePos.y, -1.f, 1.f);
		const vec4 rayWorld(inverse(App->camera->GetViewMatrix()) * vec4(rayEye.x, rayEye.y, -1.f, 0.f));

		vec3 Dir(rayWorld.x, rayWorld.y, rayWorld.z);
		//Cast a ray from the camera, in the "mouse" direction
		PhysBody3D* body = App->physics->RayCast(App->camera->Position, Dir);
		if (body)
		{
			//Change the color of the clicked primitive
			body->parentPrimitive->color = Color((float)(std::rand() % 255) / 255.f, (float)(std::rand() % 255) / 255.f, (float)(std::rand() % 255) / 255.f);
		}
	}
}

void ModuleSceneIntro::DebugSpawnPrimitive(Primitive * p)
{
	primitives.PushBack(p);
	p->SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
	p->body.collision_listeners.PushBack(this);
	p->body.Push(-App->camera->Z * 1000.f);
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	//TODO move the camera debug mode to the handle inmput function, jsut activate the camera debug property when in debug
	if (App->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
		App->camera->debugcamera = !App->camera->debugcamera;

	if (App->debug == true)
		HandleDebugInput();

	for (uint n = 0; n < primitives.Count(); n++)
		primitives[n]->Update();

	return UPDATE_CONTINUE;
}

update_status ModuleSceneIntro::PostUpdate(float dt)
{
	for (uint n = 0; n < primitives.Count(); n++)
	{
		primitives[n]->Render();
	}

	char title[250];
	if (App->map->GetLaps() >= 3)
	{
		sprintf_s(title, "%.1f Km/h || VICTORY!", App->player->vehicle->GetKmh());
	}
	else
	{
		sprintf_s(title, "%.1f Km/h || Total Laps: %i || Lap Time: %.2f || Best Lap Time: %.2f", App->player->vehicle->GetKmh(), App->map->GetLaps(), lap_time.Read() * 0.001f,best_lap_time);
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