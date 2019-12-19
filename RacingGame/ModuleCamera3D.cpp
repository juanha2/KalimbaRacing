#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModulePhysics3D.h"
#include "PhysVehicle3D.h"

ModuleCamera3D::ModuleCamera3D(bool start_enabled) : Module(start_enabled)
{
	CalculateViewMatrix();

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 250.0f,50.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;
	debugcamera = false;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");
	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	if (debugcamera)
	{
		// Implement a debug camera with keys and mouse
		// Now we can make this movememnt frame rate independant!

		vec3 newPos(0, 0, 0);
		float speed = 16.0f * dt;
		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
			speed = 32.0f * dt;

		if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT) newPos.y += speed;
		if (App->input->GetKey(SDL_SCANCODE_F) == KEY_REPEAT) newPos.y -= speed;

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) newPos -= Z * speed;
		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) newPos += Z * speed;


		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) newPos -= X * speed;
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) newPos += X * speed;

		Position += newPos;
		Reference += newPos;

		// Mouse motion ----------------

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			int dx = -App->input->GetMouseXMotion();
			int dy = -App->input->GetMouseYMotion();

			float Sensitivity = 0.25f;

			Position -= Reference;

			if (dx != 0)
			{
				float DeltaX = (float)dx * Sensitivity;

				X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
				Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
				Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
			}

			if (dy != 0)
			{
				float DeltaY = (float)dy * Sensitivity;

				Y = rotate(Y, DeltaY, X);
				Z = rotate(Z, DeltaY, X);

				if (Y.y < 0.0f)
				{
					Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
					Y = cross(Z, X);
				}
			}

			Position = Reference + Z * length(Position);
		}
	}
	else
	{
		//actual driving camera
		//the camera has a focus point that is an imaginary point in front of the car
		//it also has a target position that the camera wants to be in that is behind the car
		//the camera moves to the target pos while keeps the focus in the target

		PhysVehicle3D* vehicle = App->physics->GetVehicle();

		btTransform trans_focus_point;
		btTransform trans_newpos_cam;
		trans_focus_point = trans_newpos_cam = vehicle->vehicle->getChassisWorldTransform();

		if (vehicle->GetKmh() >= -10.0f)//if vehicle is going forward
		{
			trans_focus_point.setOrigin(trans_focus_point.getOrigin() + (vehicle->vehicle->getForwardVector() * 5));//gets a position in front of the car //the scalar determines how far away from the car it is
			trans_newpos_cam.setOrigin(trans_newpos_cam.getOrigin() + (vehicle->vehicle->getForwardVector() * -10));//gets a position behind the car //the scalar determines how far away from the car it is

		}
		else
		{
			trans_focus_point.setOrigin(trans_focus_point.getOrigin() + (vehicle->vehicle->getForwardVector() * -5));//gets a position in front of the car //the scalar determines how far away from the car it is
			trans_newpos_cam.setOrigin(trans_newpos_cam.getOrigin() + (vehicle->vehicle->getForwardVector() * 10));//gets a position behind the car //the scalar determines how far away from the car it is
		}

		LookAt({ trans_focus_point.getOrigin().getX(),trans_focus_point.getOrigin().getY(),trans_focus_point.getOrigin().getZ() });

		/*btVector3 direction_to_target;
		direction_to_target.setX(trans_newpos_cam.getOrigin().getX()-Position.x);
		direction_to_target.setY(trans_newpos_cam.getOrigin().getY() - Position.y);
		direction_to_target.setZ(trans_newpos_cam.getOrigin().getZ() - Position.z);
		btScalar distance_to_target= direction_to_target.length();
		direction_to_target.normalize();
		vec3 dir = { direction_to_target.getX(), direction_to_target.getX(), direction_to_target.getX() };
		Position += dir *distance_to_target* dt * 0.5;*/
		vec3 finalpos = { trans_newpos_cam.getOrigin().x(), trans_newpos_cam.getOrigin().y() + 5,trans_newpos_cam.getOrigin().z() };
		btVector3 dist({ finalpos.x - Position.x,finalpos.y - Position.y ,finalpos.z - Position.z });
		btScalar modu = dist.length2();
		if (modu > 150.f)modu = 150.f;//limits the camera speed
		dist.normalize();


		Position += {dist.getX()* dt* modu, dist.getY()* dt* modu, dist.getZ()* dt* modu };//todo change the scalars to customizable variables


	}
	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const vec3& Position, const vec3& Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if (!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}
}

// -----------------------------------------------------------------
void ModuleCamera3D::LookAt(const vec3& Spot)
{
	Reference = Spot;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const vec3& Movement)
{
	Position += Movement;
	Reference += Movement;
}

// -----------------------------------------------------------------
float* ModuleCamera3D::GetRawViewMatrix()
{
	CalculateViewMatrix();
	return &ViewMatrix;
}

mat4x4 ModuleCamera3D::GetViewMatrix()
{
	CalculateViewMatrix();
	return ViewMatrix;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
	ViewMatrixInverse = inverse(ViewMatrix);
}
