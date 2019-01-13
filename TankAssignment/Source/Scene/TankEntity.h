/*******************************************
	TankEntity.h

	Tank entity template and entity classes
********************************************/

#pragma once

#include <string>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"

enum class ETankTeamMembership { solo, teamMember, teamLeader };

namespace gen
{

/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Tank Template Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// A tank template inherits the type, name and mesh from the base template and adds further
// tank specifications
class CTankTemplate : public CEntityTemplate
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Tank entity template constructor sets up the tank specifications - speed, acceleration and
	// turn speed and passes the other parameters to construct the base class
	CTankTemplate
	(
		const string& type, const string& name, const string& meshFilename,
		TFloat32 maxSpeed, TFloat32 acceleration, TFloat32 turnSpeed,
		TFloat32 turretTurnSpeed, TUInt32 maxHP, TUInt32 shellDamage, 
		TFloat32 deceleration = 0.8f
	) : CEntityTemplate( type, name, meshFilename )
	{
		// Set tank template values
		m_MaxSpeed = maxSpeed;
		m_Acceleration = acceleration;
		m_TurnSpeed = turnSpeed;
		m_TurretTurnSpeed = turretTurnSpeed;
		m_MaxHP = maxHP;
		m_ShellDamage = shellDamage;
		m_Deceleration = deceleration;
	}

	// No destructor needed (base class one will do)


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	//	Getters

	TFloat32 GetMaxSpeed()
	{
		return m_MaxSpeed;
	}

	TFloat32 GetAcceleration()
	{
		return m_Acceleration;
	}

	TFloat32 GetTurnSpeed()
	{
		return m_TurnSpeed;
	}

	TFloat32 GetTurretTurnSpeed()
	{
		return m_TurretTurnSpeed;
	}

	TInt32 GetMaxHP()
	{
		return m_MaxHP;
	}

	TInt32 GetShellDamage()
	{
		return m_ShellDamage;
	}

	TFloat32 GetDeceleration()
	{
		return m_Deceleration;
	}


/////////////////////////////////////
//	Private interface
private:

	// Common statistics for this tank type (template)
	TFloat32 m_MaxSpeed;        // Maximum speed for this kind of tank
	TFloat32 m_Acceleration;    // Acceleration  -"-
	TFloat32 m_TurnSpeed;       // Turn speed    -"-
	TFloat32 m_TurretTurnSpeed; // Turret turn speed    -"-
	TFloat32 m_Deceleration;

	TUInt32  m_MaxHP;           // Maximum (initial) HP for this kind of tank
	TUInt32  m_ShellDamage;     // HP damage caused by shells from this kind of tank
};



/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Tank Entity Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// A tank entity inherits the ID/positioning/rendering support of the base entity class
// and adds instance and state data. It overrides the update function to perform the tank
// entity behaviour
// The shell code performs very limited behaviour to be rewritten as one of the assignment
// requirements. You may wish to alter other parts of the class to suit your game additions
// E.g extra member variables, constructor parameters, getters etc.
class CTankEntity : public CEntity
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Tank constructor intialises tank-specific data and passes its parameters to the base
	// class constructor
	CTankEntity
	(
		CTankTemplate*  tankTemplate,
		TEntityUID      UID,
		TUInt32         team,
		const string&   name = "",
		const CVector3& position = CVector3::kOrigin, 
		const CVector3& rotation = CVector3( 0.0f, 0.0f, 0.0f ),
		const CVector3& scale = CVector3( 1.0f, 1.0f, 1.0f )
	);

	// No destructor needed


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	// Getters

	TFloat32 GetSpeed()
	{
		return m_Speed;
	}

	const TFloat32& GetHp()
	{
		return m_HP;
	}
	
	const TFloat32& GetMaxHp()
	{
		return m_TankTemplate->GetMaxHP();
	}

	const string GetState()
	{
		switch (m_State)
		{
		case EState::Inactive:
			return "Inactive";
			break;
		case EState::Patrol:
			return "Patrol";
			break;
		case EState::Aim:
			return "Aim";
			break;
		case EState::Evade:
			return "Evade";
			break;
		default:
			return "";
			break;
		}
	}

	const string GetMembership()
	{
		switch (m_MemberState)
		{
		case ETankTeamMembership::teamLeader:
			return "Team Leader";
			break;
		case ETankTeamMembership::teamMember:
			return "Team Member";
			break;
		case ETankTeamMembership::solo:
			return "Solo";
			break;
		default:
			return "";
			break;
		}
	}

	/////////////////////////////////////
	// Update

	// Update the tank - performs tank message processing and behaviour
	// Return false if the entity is to be destroyed
	// Keep as a virtual function in case of further derivation
	virtual bool Update( TFloat32 updateTime );

	pair<TFloat32, TFloat32> CTankEntity::AccAndTurn(CVector3 targetPos, TFloat32 updateTime);	// function off turning and acceleration to tanks
	

/////////////////////////////////////
//	Private interface
private:

	/////////////////////////////////////
	// Types

	// States available for a tank - placeholders for shell code
	enum class EState
	{
		Inactive,
		Patrol,
		Aim,
		Evade
	};


	/////////////////////////////////////
	// Data

	// The template holding common data for all tank entities
	CTankTemplate* m_TankTemplate;

	// Tank data
	TUInt32  m_Team;  // Team number for tank (to know who the enemy is)
	TFloat32 m_Speed; // Current speed (in facing direction)
	TInt32   m_HP;    // Current hit points for the tank
	TFloat32 m_TurretSpeed;		// turn rate of the turret per frame
	TFloat32 m_Countdown;		// delay for tank between actions
	TFloat32 m_TurnSpeed;		// tank turn speed
	TEntityUID m_UID;			// tank uid

	// Tank state
	EState   m_State; // Current state
	TFloat32 m_Timer; // A timer used in the example update function   
	ETankTeamMembership m_MemberState; // current team membership
	int m_TeamMemberNumber;	// number in team

	// waypointing
	TInt32 m_Waypoint;				// current target waypoint
	CVector3 m_TargetPosition;	// tanks non waypoint target

	// targeting
	TEntityUID m_Target;		// current target
};


} // namespace gen
