#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "PhysBody3D.h"
#include "ModulePhysics3D.h"

ModulePlayer::ModulePlayer(bool start_enabled) : Module(start_enabled), vehicle(NULL)
{
	turn = acceleration = brake = 0.0f;
}

ModulePlayer::~ModulePlayer()
{}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading player");

	VehicleInfo car;

	// Car properties ----------------------------------------	
	car.mass = 700.0f;
	car.suspensionStiffness = 15.88f;
	car.suspensionCompression = 0.83f;
	car.suspensionDamping = 0.88f;
	car.maxSuspensionTravelCm = 1000.0f;
	car.frictionSlip = 900;
	car.maxSuspensionForce = 4000.0f;

	// Chassis properties ---------------------------------------

	car.num_chassis = 5;
	car.chassis = new Chassis[5];

	// MAIN CHASSIS -------------------------------------------
	car.chassis[0].chassis_size.Set(2.2f, 0.80f, 4);
	car.chassis[0].chassis_offset.Set(0, 0.85f, 0);
	car.chassis[0].chassis_color = Yellow;

	// CABIN CHASSIS -------------------------------------------
	car.chassis[1].chassis_size.Set(1.5f, 0.50f, 1.75f);
	car.chassis[1].chassis_offset.Set(0, 1.4f, -0.5f);
	car.chassis[1].chassis_color = White;

	// FLAP CHASSIS -------------------------------------------
	car.chassis[2].chassis_size.Set(2.3f, 0.075f, 0.3f);
	car.chassis[2].chassis_offset.Set(0, 1.5f, -1.7f);
	car.chassis[2].chassis_color = Green;

	// FLAP LEFT CHASSIS -------------------------------------------
	car.chassis[3].chassis_size.Set(0.1f, 0.3f, 0.15f);
	car.chassis[3].chassis_offset.Set(-0.75f, 1.35f, -1.7f);
	car.chassis[3].chassis_color = Green;

	// FLAP RIGHT CHASSIS -------------------------------------------
	car.chassis[4].chassis_size.Set(0.1f, 0.3f, 0.15f);
	car.chassis[4].chassis_offset.Set(0.75f, 1.35f, -1.7f);
	car.chassis[4].chassis_color = Green;

	// Wheel properties ---------------------------------------
	float connection_height = 1.2f;
	float wheel_radius = 0.6f;
	float wheel_width = 0.5f;
	float suspensionRestLength = 1.2f;

	// Don't change anything below this line ------------------

	float half_width = car.chassis[0].chassis_size.x * 0.5f;
	float half_length = car.chassis[0].chassis_size.z * 0.5f;

	vec3 direction(0, -1, 0);
	vec3 axis(-1, 0, 0);

	car.num_wheels = 4;
	car.wheels = new Wheel[4];

	// FRONT-LEFT ------------------------
	car.wheels[0].connection.Set(half_width - 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[0].direction = direction;
	car.wheels[0].axis = axis;
	car.wheels[0].suspensionRestLength = suspensionRestLength;
	car.wheels[0].radius = wheel_radius;
	car.wheels[0].width = wheel_width;
	car.wheels[0].front = true;
	car.wheels[0].drive = true;
	car.wheels[0].brake = false;
	car.wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car.wheels[1].connection.Set(-half_width + 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[1].direction = direction;
	car.wheels[1].axis = axis;
	car.wheels[1].suspensionRestLength = suspensionRestLength;
	car.wheels[1].radius = wheel_radius;
	car.wheels[1].width = wheel_width;
	car.wheels[1].front = true;
	car.wheels[1].drive = true;
	car.wheels[1].brake = false;
	car.wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car.wheels[2].connection.Set(half_width - 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[2].direction = direction;
	car.wheels[2].axis = axis;
	car.wheels[2].suspensionRestLength = suspensionRestLength;
	car.wheels[2].radius = wheel_radius;
	car.wheels[2].width = wheel_width;
	car.wheels[2].front = false;
	car.wheels[2].drive = false;
	car.wheels[2].brake = true;
	car.wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car.wheels[3].connection.Set(-half_width + 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[3].direction = direction;
	car.wheels[3].axis = axis;
	car.wheels[3].suspensionRestLength = suspensionRestLength;
	car.wheels[3].radius = wheel_radius;
	car.wheels[3].width = wheel_width;
	car.wheels[3].front = false;
	car.wheels[3].drive = false;
	car.wheels[3].brake = true;
	car.wheels[3].steering = false;

	vehicle = App->physics->AddVehicle(car);
	vehicle->SetPos(0, 12, 10);
	

	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");

	return true;
}

// Update: draw background
update_status ModulePlayer::Update(float dt)
{
	acceleration = brake = 0.0f;
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_IDLE && App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_IDLE)//if not pressing turn slowly center the wheels
	{
		if (turn < -0.025f)turn += dt;
		else if (turn > 0.025f)turn -= dt;
		else turn = 0.0f;
	}

	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT&& vehicle->GetKmh() < 150.0f)//only apply acceleration if vehicle is under 100 km/h
	{
		if (vehicle->GetKmh() < 0.0f)//if its going backwards first brake
		{
			brake = BRAKE_POWER;
		}
		acceleration = MAX_ACCELERATION;
	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	{
		if (turn < TURN_DEGREES)
			turn += TURN_DEGREES*dt*2;
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{
		if (turn > -TURN_DEGREES)
			turn -= TURN_DEGREES*dt*2;
	}
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT&&vehicle->GetKmh()>-30.0f)//only apply acceleration if vehicle is above -30 km/h
	{
		if (vehicle->GetKmh() > 0.0f)//if its going forward first brake
		{
			brake = BRAKE_POWER;
		}
		acceleration = -0.5*MAX_ACCELERATION;
	}
	
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT)
	{
		brake = BRAKE_POWER;
	}

	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		//TODO teleport the vehicle

	}
	vehicle->ApplyEngineForce(acceleration);
	vehicle->Turn(turn);
	vehicle->Brake(brake);

	vehicle->Render();

	

	return UPDATE_CONTINUE;
}



