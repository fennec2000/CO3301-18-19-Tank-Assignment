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
#include "CParseLevel.h"
#include "CRay.h"
#include "TeamManager.h"

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

// Entity manager and level parser
CEntityManager EntityManager;

// Tank UIDs
CTeamManager TeamManager;

// Parse level
CParseLevel LevelParser(&EntityManager, &TeamManager);

// ray
CRay Ray(&EntityManager);

// Tank waypoints - list of lists of CVectro3 variable(team)(wapoint)
vector<vector<CVector3>> TeamWaypoints;

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

	//////////////////////////////////////////////
	// Parse level's XML
	LevelParser.ParseFile("Entities.xml");

	//////////////////////////////////////////////
	// Setups
	Ray.Setup();


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

	// team waypoints
	const vector<CVector3> listAWaypoints = { CVector3(-30.0f, 0.5f, -20.0f),	CVector3(-30.0f, 0.5f, 20.0f),
									CVector3(-50.0f, 0.5f, 0.0f),	CVector3(-70.0f, 0.5f, 20.0f), 
									CVector3(-70.0f, 0.5f, -20.0f),	CVector3(-50.0f, 0.5f, 0.0f) };
	const vector<CVector3> listBWaypoints = { CVector3(30.0f, 0.5f, -20.0f),	CVector3(30.0f, 0.5f, 20.0f),
									CVector3(50.0f, 0.5f, 0.0f),	CVector3(70.0f, 0.5f, 20.0f),
									CVector3(70.0f, 0.5f, -20.0f),	CVector3(50.0f, 0.5f, 0.0f) };
	TeamWaypoints.push_back(listAWaypoints);
	TeamWaypoints.push_back(listBWaypoints);

	return true;
}

// waypoint functions
unsigned int GetMaxTeams()
{
	return TeamWaypoints.size();
}

unsigned int GetMaxWaypoints(unsigned int team)
{
	return TeamWaypoints[team].size();
}

CVector3 GetWaypoint(unsigned int team, unsigned int waypoint)
{
	return TeamWaypoints[team][waypoint];
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
	const int NumOfTeams = TeamManager.GetNumberOfTeams();
	for (int i = 0; i < NumOfTeams; ++i)
	{
		const int teamSize = TeamManager.GetTeamSize(i);
		for (int j = 0; j < teamSize; ++j)
		{
			const auto tankUID = TeamManager.GetTankUID(i, j);
			auto tank = EntityManager.GetEntity(tankUID);
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
	}

	// display tanks info
	for (int i = 0; i < NumOfTeams; ++i)
	{
		const int teamSize = TeamManager.GetTeamSize(i);
		for (int j = 0; j < teamSize; ++j)
		{
			const auto tankUID = TeamManager.GetTankUID(i, j);
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
		const int NumOfTeams = TeamManager.GetNumberOfTeams();
		SMessage msg;
		msg.type = EMessageType::Msg_TankStart;
		msg.from = SystemUID;
		for (int i = 0; i < NumOfTeams; ++i)
		{
			const int teamSize = TeamManager.GetTeamSize(i);
			for (int j = 0; j < teamSize; ++j)
			{
				const auto tankUID = TeamManager.GetTankUID(i, j);
				Messenger.SendMessage(tankUID, msg);
			}
		}
	}

	// Stop
	if (KeyHit(Key_2))
	{
		const int NumOfTeams = TeamManager.GetNumberOfTeams();
		SMessage msg;
		msg.type = EMessageType::Msg_TankStop;
		msg.from = SystemUID;
		for (int i = 0; i < NumOfTeams; ++i)
		{
			const int teamSize = TeamManager.GetTeamSize(i);
			for (int j = 0; j < teamSize; ++j)
			{
				const auto tankUID = TeamManager.GetTankUID(i, j);
				Messenger.SendMessage(tankUID, msg);
			}
		}
	}

	// Aim
	if (KeyHit(Key_3))
	{
		const int NumOfTeams = TeamManager.GetNumberOfTeams();
		SMessage msg;
		msg.type = EMessageType::Msg_TankAim;
		msg.from = SystemUID;
		for (int i = 0; i < NumOfTeams; ++i)
		{
			const int teamSize = TeamManager.GetTeamSize(i);
			for (int j = 0; j < teamSize; ++j)
			{
				const auto tankUID = TeamManager.GetTankUID(i, j);
				Messenger.SendMessage(tankUID, msg);
			}
		}
	}

	// Hit
	if (KeyHit(Key_4))
	{
		const int NumOfTeams = TeamManager.GetNumberOfTeams();
		SMessage msg;
		msg.type = EMessageType::Msg_TankHit;
		msg.from = SystemUID;
		for (int i = 0; i < NumOfTeams; ++i)
		{
			const int teamSize = TeamManager.GetTeamSize(i);
			for (int j = 0; j < teamSize; ++j)
			{
				const auto tankUID = TeamManager.GetTankUID(i, j);
				Messenger.SendMessage(tankUID, msg);
			}
		}
	}

	// last tank on camera
	if (KeyHit(Key_7))
	{
		--currentTankWatched;
		if (currentTankWatched < 0)
			currentTankWatched = TeamManager.GetTotalNumberOfTanks() - 1;
	}

	// next tank on camera
	if (KeyHit(Key_8))
	{
		currentTankWatched++;
		if (currentTankWatched >= TeamManager.GetTotalNumberOfTanks())
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
		int team = 0;
		int tankInTeam = currentTankWatched;
		while (currentTankWatched > TeamManager.GetTeamSize(team))
		{
			tankInTeam -= TeamManager.GetTeamSize(team);
			++team;
		}

		const auto tankUID = TeamManager.GetTankUID(team, tankInTeam);
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
