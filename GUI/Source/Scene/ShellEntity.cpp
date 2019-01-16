/*******************************************
	ShellEntity.cpp

	Shell entity class
********************************************/

#include "ShellEntity.h"
#include "TankEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "TeamManager.h"

namespace gen
{
const TFloat32 bulletCollisionRadious = 3.0f;

// Reference to entity manager from TankAssignment.cpp, allows look up of entities by name, UID etc.
// Can then access other entity's data. See the CEntityManager.h file for functions. Example:
//    CVector3 targetPos = EntityManager.GetEntity( targetUID )->GetMatrix().Position();
extern CEntityManager EntityManager;

// Messenger class for sending messages to and between entities
extern CMessenger Messenger;

// Helper function made available from TankAssignment.cpp - gets UID of tank A (team 0) or B (team 1).
// Will be needed to implement the required shell behaviour in the Update function below
extern CTeamManager TeamManager;



/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Shell Entity Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// Shell constructor intialises shell-specific data and passes its parameters to the base
// class constructor
CShellEntity::CShellEntity
(
	CEntityTemplate* entityTemplate,
	TEntityUID       UID,
	const string&    name /*=""*/,
	const CVector3&  position /*= CVector3::kOrigin*/, 
	const CVector3&  rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
	const CVector3&  scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
) : CEntity( entityTemplate, UID, name, position, rotation, scale )
{
	// Initialise any shell data you add
	m_Speed = 50.0f;
	m_Life = 3.0f;
}


// Update the shell - controls its behaviour. The shell code is empty, it needs to be written as
// one of the assignment requirements
// Return false if the entity is to be destroyed
bool CShellEntity::Update( TFloat32 updateTime )
{
	// movement
	Matrix().MoveLocalZ(m_Speed * updateTime);

	const int NumOfTeams = TeamManager.GetNumberOfTeams();
	for (int i = 0; i < NumOfTeams; ++i)
	{
		const int teamSize = TeamManager.GetTeamSize(i);
		for (int j = 0; j < teamSize; ++j)
		{
			auto tankUID = TeamManager.GetTankUID(i, j);
			auto tank = EntityManager.GetEntity(tankUID);
			if (tank != nullptr && Distance(Position(), tank->Position()) < bulletCollisionRadious)
			{
				SMessage msg;
				msg.type = EMessageType::Msg_TankHit;
				msg.from = m_UID;
				Messenger.SendMessage(tankUID, msg);
				return false;
			}
		}
	}

	// life
	m_Life -= updateTime;
	if (m_Life <= 0)
		return false;
	return true;
}

void CShellEntity::BulletOwner(TEntityUID tank)
{
	m_UID = tank;
}


} // namespace gen
