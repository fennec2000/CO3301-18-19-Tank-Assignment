/*******************************************
	TankEntity.cpp

	Tank entity template and entity classes
********************************************/

// Additional technical notes for the assignment:
// - Each tank has a team number (0 or 1), HP and other instance data - see the end of TankEntity.h
//   You will need to add other instance data suitable for the assignment requirements
// - A function GetTankUID is defined in TankAssignment.cpp and made available here, which returns
//   the UID of the tank on a given team. This can be used to get the enemy tank UID
// - Tanks have three parts: the root, the body and the turret. Each part has its own matrix, which
//   can be accessed with the Matrix function - root: Matrix(), body: Matrix(1), turret: Matrix(2)
//   However, the body and turret matrix are relative to the root's matrix - so to get the actual 
//   world matrix of the body, for example, we must multiply: Matrix(1) * Matrix()
// - Vector facing work similar to the car tag lab will be needed for the turret->enemy facing 
//   requirements for the Patrol and Aim states
// - The CMatrix4x4 function DecomposeAffineEuler allows you to extract the x,y & z rotations
//   of a matrix. This can be used on the *relative* turret matrix to help in rotating it to face
//   forwards in Evade state
// - The CShellEntity class is simply an outline. To support shell firing, you will need to add
//   member data to it and rewrite its constructor & update function. You will also need to update 
//   the CreateShell function in EntityManager.cpp to pass any additional constructor data required
// - Destroy an entity by returning false from its Update function - the entity manager wil perform
//   the destruction. Don't try to call DestroyEntity from within the Update function.
// - As entities can be destroyed, you must check that entity UIDs refer to existant entities, before
//   using their entity pointers. The return value from EntityManager.GetEntity will be NULL if the
//   entity no longer exists. Use this to avoid trying to target a tank that no longer exists etc.

#include "TankEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "CVector3.h"	// temp for waypoints
#include <random>	// random for random pos

namespace gen
{

// temp waypoints
	const int waypointMax = 2;
	CVector3 waypoints[2][waypointMax] = {	{CVector3(-30.0f, 0.5f, -20.0f),	CVector3(-30.0f, 0.5f, 20.0f)},
									{CVector3(30.0f, 0.5f, 20.0f),		CVector3(30.0f, 0.5f, -20.0f)} };
	float Epsilon = 0.1f;
	float waypointRadious = 1.0f;

	// random
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-40.0f, 40.0f);

// Reference to entity manager from TankAssignment.cpp, allows look up of entities by name, UID etc.
// Can then access other entity's data. See the CEntityManager.h file for functions. Example:
//    CVector3 targetPos = EntityManager.GetEntity( targetUID )->GetMatrix().Position();
extern CEntityManager EntityManager;

// Messenger class for sending messages to and between entities
extern CMessenger Messenger;

// Helper function made available from TankAssignment.cpp - gets UID of tank A (team 0) or B (team 1).
// Will be needed to implement the required tank behaviour in the Update function below
extern TEntityUID GetTankUID( int team );



/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Tank Entity Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// Tank constructor intialises tank-specific data and passes its parameters to the base
// class constructor
CTankEntity::CTankEntity
(
	CTankTemplate*  tankTemplate,
	TEntityUID      UID,
	TUInt32         team,
	const string&   name /*=""*/,
	const CVector3& position /*= CVector3::kOrigin*/, 
	const CVector3& rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
	const CVector3& scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
) : CEntity( tankTemplate, UID, name, position, rotation, scale )
{
	m_TankTemplate = tankTemplate;

	// Tanks are on teams so they know who the enemy is
	m_Team = team;

	// Initialise other tank data and state
	m_Speed = 0.0f;
	m_HP = m_TankTemplate->GetMaxHP();
	m_State = EState::Inactive;
	m_Timer = 0.0f;
	m_Waypoint = 0;
	m_Countdown = 0;
	m_TargetPosition = {0, 0, 0};
}


// Update the tank - controls its behaviour. The shell code just performs some test behaviour, it
// is to be rewritten as one of the assignment requirements
// Return false if the entity is to be destroyed
bool CTankEntity::Update( TFloat32 updateTime )
{
	// Fetch any messages
	SMessage msg;
	while (Messenger.FetchMessage( GetUID(), &msg ))
	{
		// Set state variables based on received messages
		switch (msg.type)
		{
		case EMessageType::Msg_TankStart:
			m_State = EState::Patrol;
			Matrix().FaceTarget(waypoints[m_Team][m_Waypoint]);
			m_Speed = m_TankTemplate->GetMaxSpeed();
			break;
		case EMessageType::Msg_TankStop:
			m_State = EState::Inactive;
			m_Speed = 0;
			break;
		case EMessageType::Msg_TankAim: // temp used to change state
			m_State = EState::Aim;
			m_Speed = 0;
			m_Countdown = 1.0f;
			break;
		case EMessageType::Mgs_TankHit:
			m_HP -= 20;
			if (m_HP <= 0)
				EntityManager.DestroyEntity(GetUID());
			break;
		}
	}

	// Tank behaviour
	// Only move if in Go state
	
	if (m_State == EState::Patrol)
	{
		// check if at waypoint
		if (Distance(waypoints[m_Team][m_Waypoint], Position()) <= waypointRadious)
		{
			// next waypoint and face target
			++m_Waypoint;
			if (m_Waypoint >= waypointMax)
				m_Waypoint = 0;

			Matrix().FaceTarget(waypoints[m_Team][m_Waypoint]);
		}

		// rotate gun
		Matrix(2).RotateLocalY(m_TankTemplate->GetTurretTurnSpeed());

	}
	else if (m_State == EState::Aim)
	{
		if (m_Countdown <= 0)
		{
			m_State = EState::Evade;
			m_TargetPosition = {distribution(generator), 0.5f, distribution(generator)};
			Matrix().FaceTarget(m_TargetPosition);
			m_Speed = m_TankTemplate->GetMaxSpeed();
		}
	}
	else if (m_State == EState::Evade)
	{
		if (Distance(m_TargetPosition, Position()) <= waypointRadious)
		{
			m_State = EState::Patrol;
			Matrix().FaceTarget(waypoints[m_Team][m_Waypoint]);
		}
	}
	else	// EState::Inactive and catches unknown states
	{

	}

	// Perform movement...
	// Move along local Z axis scaled by update time
	Matrix().MoveLocalZ( m_Speed * updateTime );

	// countdown
	if (m_Countdown > 0)
	{
		m_Countdown -= updateTime;
		if (m_Countdown <= 0)
			m_Countdown = 0;
	}

	// mgs to display text on screen
	msg.type = EMessageType::Msg_DisplayEntityInfo;
	msg.from = GetUID();
	Messenger.SendMessage(TextSystemUID, msg);

	return true; // Don't destroy the entity
}


} // namespace gen
