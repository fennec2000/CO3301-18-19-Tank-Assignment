/*******************************************
	TankAssignment.cpp

	Shell scene and game functions
********************************************/

#include <sstream>
#include <string>
using namespace std;

#include <d3d10.h>
#include <d3dx10.h>

#include "Defines.h"
#include "CVector3.h"
#include "Camera.h"
#include "Light.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "TankAssignment.h"

namespace gen
{

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// Control speed
const float CameraRotSpeed = 2.0f;
float CameraMoveSpeed = 80.0f;

// Amount of time to pass before calculating new average update time
const float UpdateTimePeriod = 1.0f;


//-----------------------------------------------------------------------------
// Global system variables
//-----------------------------------------------------------------------------

// Get reference to global DirectX variables from another source file
extern ID3D10Device*           g_pd3dDevice;
extern IDXGISwapChain*         SwapChain;
extern ID3D10DepthStencilView* DepthStencilView;
extern ID3D10RenderTargetView* BackBufferRenderTarget;
extern ID3DX10Font*            OSDFont;

// Actual viewport dimensions (fullscreen or windowed)
extern TUInt32 ViewportWidth;
extern TUInt32 ViewportHeight;

// Current mouse position
extern CVector2 MousePixel;

// Messenger class for sending messages to and between entities
extern CMessenger Messenger;


//-----------------------------------------------------------------------------
// Global game/scene variables
//-----------------------------------------------------------------------------

// Entity manager
CEntityManager EntityManager;

// Tank UIDs
TEntityUID TankA;
TEntityUID TankB;
int totalNumberOfTanks = 2;

// Other scene elements
const int NumLights = 2;
CLight*  Lights[NumLights];
SColourRGBA AmbientLight;
CCamera* MainCamera;
bool chaseCam = false;
int currentTankWatched = 0;

// Sum of recent update times and number of times in the sum - used to calculate
// average over a given time period
float SumUpdateTimes = 0.0f;
int NumUpdateTimes = 0;
float AverageUpdateTime = -1.0f; // Invalid value at first

// User Interface
bool ExtendedInfo = true;
TEntityUID nearestTank = -1;
TEntityUID currentlySelectedTank = -1;
CVector3 MouseTarget3DPos;

//-----------------------------------------------------------------------------
// Scene management
//-----------------------------------------------------------------------------

// Creates the scene geometry
bool SceneSetup()
{
	//////////////////////////////////////////////
	// Prepare render methods

	InitialiseMethods();


	//////////////////////////////////////////
	// Create scenery templates and entities

	// Create scenery templates - loads the meshes
	// Template type, template name, mesh name
	EntityManager.CreateTemplate("Scenery", "Skybox", "Skybox.x");
	EntityManager.CreateTemplate("Scenery", "Floor", "Floor.x");
	EntityManager.CreateTemplate("Scenery", "Building", "Building.x");
	EntityManager.CreateTemplate("Scenery", "Tree", "Tree1.x");

	// Creates scenery entities
	// Type (template name), entity name, position, rotation, scale
	EntityManager.CreateEntity("Skybox", "Skybox", CVector3(0.0f, -10000.0f, 0.0f), CVector3::kZero, CVector3(10, 10, 10));
	EntityManager.CreateEntity("Floor", "Floor");
	EntityManager.CreateEntity("Building", "Building", CVector3(0.0f, 0.0f, 40.0f));
	for (int tree = 0; tree < 100; ++tree)
	{
		// Some random trees
		EntityManager.CreateEntity( "Tree", "Tree",
			                        CVector3(Random(-200.0f, 30.0f), 0.0f, Random(40.0f, 150.0f)),
			                        CVector3(0.0f, Random(0.0f, 2.0f * kfPi), 0.0f) );
	}


	/////////////////////////////////
	// Create tank templates

	// Template type, template name, mesh name, top speed, acceleration, tank turn speed, turret
	// turn speed, max HP and shell damage. These latter settings are for advanced requirements only
	EntityManager.CreateTankTemplate("Tank", "Rogue Scout", "HoverTank02.x",
		24.0f, 2.2f, 2.0f, kfPi / 3, 100, 20);
	EntityManager.CreateTankTemplate("Tank", "Oberon MkII", "HoverTank07.x",
		18.0f, 1.6f, 1.3f, kfPi / 4, 120, 35);

	// Template for tank shell
	EntityManager.CreateTemplate("Projectile", "Shell Type 1", "Bullet.x");


	////////////////////////////////
	// Create tank entities

	// Type (template name), team number, tank name, position, rotation
	TankA = EntityManager.CreateTank("Rogue Scout", 0, "A-1", CVector3(-30.0f, 0.5f, -20.0f),
		CVector3(0.0f, ToRadians(0.0f), 0.0f));
	TankB = EntityManager.CreateTank("Oberon MkII", 1, "B-1", CVector3(30.0f, 0.5f, 20.0f),
		CVector3(0.0f, ToRadians(180.0f), 0.0f));


	/////////////////////////////
	// Camera / light setup

	// Set camera position and clip planes
	MainCamera = new CCamera(CVector3(0.0f, 30.0f, -100.0f), CVector3(ToRadians(15.0f), 0, 0));
	MainCamera->SetNearFarClip(1.0f, 20000.0f);

	// Sunlight and light in building
	Lights[0] = new CLight(CVector3(-5000.0f, 4000.0f, -10000.0f), SColourRGBA(1.0f, 0.9f, 0.6f), 15000.0f);
	Lights[1] = new CLight(CVector3(6.0f, 7.5f, 40.0f), SColourRGBA(1.0f, 0.0f, 0.0f), 1.0f);

	// Ambient light level
	AmbientLight = SColourRGBA(0.6f, 0.6f, 0.6f, 1.0f);

	return true;
}


// Release everything in the scene
void SceneShutdown()
{
	// Release render methods
	ReleaseMethods();

	// Release lights
	for (int light = NumLights - 1; light >= 0; --light)
	{
		delete Lights[light];
	}

	// Release camera
	delete MainCamera;

	// Destroy all entities
	EntityManager.DestroyAllEntities();
	EntityManager.DestroyAllTemplates();
}


//-----------------------------------------------------------------------------
// Game Helper functions
//-----------------------------------------------------------------------------

// Get UID of tank A (team 0) or B (team 1)
TEntityUID GetTankUID(int team)
{
	return (team == 0) ? TankA : TankB;
}


//-----------------------------------------------------------------------------
// Game loop functions
//-----------------------------------------------------------------------------

// Draw one frame of the scene
void RenderScene( float updateTime )
{
	// Setup the viewport - defines which part of the back-buffer we will render to (usually all of it)
	D3D10_VIEWPORT vp;
	vp.Width  = ViewportWidth;
	vp.Height = ViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports( 1, &vp );

	// Select the back buffer and depth buffer to use for rendering
	g_pd3dDevice->OMSetRenderTargets( 1, &BackBufferRenderTarget, DepthStencilView );
	
	// Clear previous frame from back buffer and depth buffer
	g_pd3dDevice->ClearRenderTargetView( BackBufferRenderTarget, &AmbientLight.r );
	g_pd3dDevice->ClearDepthStencilView( DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );

	// Update camera aspect ratio based on viewport size - for better results when changing window size
	MainCamera->SetAspect( static_cast<TFloat32>(ViewportWidth) / ViewportHeight );

	// Set camera and light data in shaders
	MainCamera->CalculateMatrices();
	SetCamera(MainCamera);
	SetAmbientLight(AmbientLight);
	SetLights(&Lights[0]);

	// Render entities and draw on-screen text
	EntityManager.RenderAllEntities();
	RenderSceneText( updateTime );

    // Present the backbuffer contents to the display
	SwapChain->Present( 0, 0 );
}


// Render a single text string at the given position in the given colour, may optionally centre it
void RenderText( const string& text, int X, int Y, float r, float g, float b, bool centre = false )
{
	RECT rect;
	if (!centre)
	{
		SetRect( &rect, X, Y, 0, 0 );
		OSDFont->DrawText( NULL, text.c_str(), -1, &rect, DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
	}
	else
	{
		SetRect( &rect, X - 100, Y, X + 100, 0 );
		OSDFont->DrawText( NULL, text.c_str(), -1, &rect, DT_CENTER | DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
	}
}

// Render on-screen text each frame
void RenderSceneText( float updateTime )
{
	stringstream outText;
	float mouseDistance = INFINITY;

	// Nearest tank
	for (int i = 0; i < 2; ++i)
	{
		const auto tankUID = GetTankUID(i);
		auto tank = dynamic_cast<CTankEntity*>(EntityManager.GetEntity(tankUID));
		CVector2 entityScreenPos;
		if (tank != nullptr && MainCamera->PixelFromWorldPt(&entityScreenPos, tank->Position(), ViewportWidth, ViewportHeight))
		{
			const float entMouseDistance = Distance(MousePixel, entityScreenPos);
			if (mouseDistance > entMouseDistance)
			{
				mouseDistance = entMouseDistance;
				nearestTank = tankUID;
			}
		}
	}

	// display tanks info
	for (int i = 0; i < 2; ++i)
	{
		const auto tankUID = GetTankUID(i);
		auto tank = dynamic_cast<CTankEntity*>(EntityManager.GetEntity(tankUID));
		if (tank == nullptr)
			continue;
		outText << tank->Template()->GetName() << '\n';
		if (ExtendedInfo)
		{
			outText << tank->GetHp() << '/' << tank->GetMaxHp() << '\n';
			outText << "State: " << tank->GetState() << '\n';
		}
		if (currentlySelectedTank == tankUID)
		{
			outText << "*Selected*\n";
		}
		CVector2 entityScreenPos;
		if (MainCamera->PixelFromWorldPt(&entityScreenPos, tank->Position(), ViewportWidth, ViewportHeight))
		{
			if (nearestTank == tankUID)
				RenderText(outText.str(), entityScreenPos.x, entityScreenPos.y, 1.0f, 0.0f, 0.0f, true);
			else
				RenderText(outText.str(), entityScreenPos.x, entityScreenPos.y, 1.0f, 1.0f, 0.0f, true);
		}
		outText.str("");
	}

	// Accumulate update times to calculate the average over a given period
	SumUpdateTimes += updateTime;
	++NumUpdateTimes;
	if (SumUpdateTimes >= UpdateTimePeriod)
	{
		AverageUpdateTime = SumUpdateTimes / NumUpdateTimes;
		SumUpdateTimes = 0.0f;
		NumUpdateTimes = 0;
	}

	// Write FPS text string
	if (AverageUpdateTime >= 0.0f)
	{
		outText << "Frame Time: " << AverageUpdateTime * 1000.0f << "ms" << endl << "FPS:" << 1.0f / AverageUpdateTime;
		RenderText( outText.str(), 2, 2, 0.0f, 0.0f, 0.0f );
		RenderText( outText.str(), 0, 0, 1.0f, 1.0f, 0.0f );
		outText.str("");
	}
}


// Update the scene between rendering
void UpdateScene( float updateTime )
{
	// Call all entity update functions
	EntityManager.UpdateAllEntities( updateTime );

	// Picker
	if (KeyHit(EKeyCode::Mouse_LButton))
	{
		currentlySelectedTank = nearestTank;
	}

	// Move to
	if (KeyHit(EKeyCode::Mouse_RButton) && currentlySelectedTank != -1)
	{
		MouseTarget3DPos = MainCamera->WorldPtFromPixel(MousePixel.x, MousePixel.y, ViewportWidth, ViewportHeight);
		SMessage msg;
		msg.type = EMessageType::Msg_TankGoto;
		msg.from = SystemUID;
		Messenger.SendMessage(currentlySelectedTank, msg);
	}

	// Set camera speeds
	// Key F1 used for full screen toggle
	if (KeyHit(Key_F2)) CameraMoveSpeed = 5.0f;
	if (KeyHit(Key_F3)) CameraMoveSpeed = 40.0f;
	
	// extra info
	if (KeyHit(Key_0))
		ExtendedInfo = !ExtendedInfo;

	// System messages
	// Go
	if (KeyHit(Key_1))
	{
		SMessage msg;
		msg.type = EMessageType::Msg_TankStart;
		msg.from = SystemUID;
		Messenger.SendMessage(TankA, msg);
		Messenger.SendMessage(TankB, msg);
	}

	// Stop
	if (KeyHit(Key_2))
	{
		SMessage msg;
		msg.type = EMessageType::Msg_TankStop;
		msg.from = SystemUID;
		Messenger.SendMessage(TankA, msg);
		Messenger.SendMessage(TankB, msg);
	}

	// Aim
	if (KeyHit(Key_3))
	{
		SMessage msg;
		msg.type = EMessageType::Msg_TankAim;
		msg.from = SystemUID;
		Messenger.SendMessage(TankA, msg);
		Messenger.SendMessage(TankB, msg);
	}

	// Hit
	if (KeyHit(Key_4))
	{
		SMessage msg;
		msg.type = EMessageType::Msg_TankHit;
		msg.from = SystemUID;
		Messenger.SendMessage(TankA, msg);
		Messenger.SendMessage(TankB, msg);
	}

	// last tank on camera
	if (KeyHit(Key_7))
	{
		--currentTankWatched;
		if (currentTankWatched < 0)
			currentTankWatched = totalNumberOfTanks - 1;
	}

	// next tank on camera
	if (KeyHit(Key_8))
	{
		currentTankWatched++;
		if (currentTankWatched >= totalNumberOfTanks)
			currentTankWatched = 0;
	}

	// Camera mode
	if (KeyHit(Key_9))
	{
		chaseCam = !chaseCam;
	}

	// Update chase cam
	if (chaseCam)
	{
		const auto tankUID = GetTankUID(currentTankWatched);
		auto tank = EntityManager.GetEntity(tankUID);
		if (tank != nullptr)
		{
			MainCamera->Position() = tank->Position() + tank->Matrix().ZAxis() * -15.0f + CVector3(0, 5.0f, 0);
			MainCamera->Matrix().FaceTarget(tank->Position() + CVector3(0, 5.0f, 0));
		}
	}
	else
	{
		// Move the camera
		MainCamera->Control(Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D,
			CameraMoveSpeed * updateTime, CameraRotSpeed * updateTime);
	}
	

	
}


} // namespace gen
