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
#include "CRay.h"		// Ray
#include "TeamManager.h"// team manager

#include <random>		// random for random pos

namespace gen
{

// temp const
	const TFloat32 Epsilon = 0.1f;
	const TFloat32 waypointRadious = 2.5f;
	const TFloat32 turretAngularVision = ToRadians(15);
	const TFloat32 turretAimSpeedMultiplyer = 3.0f;
	const TFloat32 minRotation = 0.001f;
	const TFloat32 barrelLenght = 4.0f;
	const TFloat32 tankGravity = 5.0f;
	const TFloat32 deathForce = 10.0f;
	const TInt32 defaultBulletDamage = 20;

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

extern CVector3 MouseTarget3DPos;

// teams
extern CTeamManager TeamManager;

// waypoints
extern unsigned int GetMaxWaypoints(unsigned int team);
extern CVector3 GetWaypoint(unsigned int team, unsigned int waypoint);

// ray
extern CRay Ray;

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
	m_UID = UID;

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
	m_TurretSpeed = 0;
	m_TurnSpeed = 0;
	m_Target = 0;
	m_MemberState = ETankTeamMembership::solo;
	m_TeamMemberNumber = TeamManager.AddTank(UID, m_Team);
	m_Ammo = m_TankTemplate->GetShellAmmo();
	m_DeathVec = { Random(-deathForce, deathForce), deathForce, Random(deathForce, deathForce) };
}

pair<TFloat32, TFloat32> CTankEntity::AccAndTurn(CVector3 targetPos, TFloat32 updateTime)
{
	pair<TFloat32, TFloat32> result(0, 0);

	const CVector3 targetVector = targetPos - Position();
	bool inFrount = false;

	if (Dot(Matrix().ZAxis(), targetVector) > 0)
		inFrount = true;

	bool toRight = false;
	if (Dot(Matrix().XAxis(), targetVector) > 0)
		toRight = true;

	// if behind slow down to min
	if (!inFrount  && m_Speed > -m_TankTemplate->GetMaxSpeed()
		|| Matrix().Position().DistanceTo(targetPos) <= abs(m_Speed * m_Speed / (2 * m_TankTemplate->GetDeceleration())))
	{
		result.first = m_Speed - m_TankTemplate->GetAcceleration() * updateTime;
		if (result.first < -m_TankTemplate->GetMaxSpeed())
			result.first = -m_TankTemplate->GetMaxSpeed();
	}
	else if (m_Speed < m_TankTemplate->GetMaxSpeed())
	{
		result.first = m_Speed + m_TankTemplate->GetAcceleration() * updateTime;
		if (result.first > m_TankTemplate->GetMaxSpeed())
			result.first = m_TankTemplate->GetMaxSpeed();
	}

	// turn
	const TFloat32 rotation = Dot(Normalise(Matrix().ZAxis()), Normalise(targetVector));
	const TFloat32 acosRot = acos(rotation);
	if (acosRot > minRotation)
	{
		if (toRight)
			result.second = Min(acosRot, m_TankTemplate->GetTurnSpeed());
		else
			result.second = Min(acosRot, -m_TankTemplate->GetTurnSpeed());
	}

	return result;
}

void CTankEntity::FindAmmo()
{
	vector<TEntityUID> list = EntityManager.GetListOfUID("Ammo Cube");
	TEntityUID closest;
	TFloat32 closestDistance = INFINITY, distance;
	CEntity* entity;
	for each (TEntityUID uid in list)
	{
		entity = EntityManager.GetEntity(uid);
		distance = Distance(entity->Position(), Position());
		if (distance < closestDistance)
		{
			closest = uid;
			closestDistance = distance;
		}
	}
	m_Target = closest;
	m_TargetPosition = EntityManager.GetEntity(m_Target)->Position();
	m_State = EState::GettingAmmo;
}

// Update the tank - controls its behaviour. The shell code just performs some test behaviour, it
// is to be rewritten as one of the assignment requirements
// Return false if the entity is to be destroyed
bool CTankEntity::Update( TFloat32 updateTime )
{
	if (m_State == EState::Dying)
	{
		Matrix(2).Move(m_DeathVec * updateTime);
		Matrix(2).RotateLocalX(m_DeathVec.x * updateTime);
		Matrix(2).RotateLocalY(m_DeathVec.y * updateTime);
		Matrix(2).RotateLocalZ(m_DeathVec.z * updateTime);
		m_DeathVec.y -= updateTime * tankGravity;
		if (m_DeathVec.y < -deathForce)
			return false;
	}
	else
	{
		// Fetch any messages
		SMessage msg;
		while (Messenger.FetchMessage(GetUID(), &msg) && m_State != EState::Dying)
		{
			// Set state variables based on received messages
			if (msg.type == EMessageType::Msg_TankStart)
			{
				m_State = EState::Patrol;
				m_TargetPosition = GetWaypoint(m_Team, m_Waypoint);
				m_TurretSpeed = m_TankTemplate->GetTurretTurnSpeed();
			}
			else if (msg.type == EMessageType::Msg_TankStop)
			{
				m_State = EState::Inactive;
				m_Speed = 0;
				m_TurretSpeed = 0;
			}
			else if (msg.type == EMessageType::Msg_TankAim)
			{
				m_State = EState::Aim;
				m_Speed = 0;
				m_Countdown = 1.0f;
				m_TurretSpeed = 0;
			}
			else if (msg.type == EMessageType::Msg_TankHit)
			{
				auto entity = EntityManager.GetEntity(msg.from);
				auto tank = dynamic_cast<CTankEntity*>(entity);
				if (tank != nullptr)
					m_HP -= tank->GetShellDamage();
				else
					m_HP -= defaultBulletDamage;

				if (m_HP <= 0)
				{
					m_HP = 0;
					TeamManager.UpdateMembership(m_Team);
					m_State = EState::Dying;
				}

				// request help
				SMessage msg;
				msg.type = EMessageType::Msg_TankHelp;
				msg.from = m_UID;
				const int teamSize = TeamManager.GetTeamSize(m_Team);
				for (int j = 0; j < teamSize; ++j)
				{
					const auto tankUID = TeamManager.GetTankUID(m_Team, j);
					if (tankUID == m_UID)
						continue;
					Messenger.SendMessage(tankUID, msg);
				}
			}
			else if (msg.type == EMessageType::Msg_TankEvade)
			{
				m_State = EState::Evade;
				m_TargetPosition = CVector3(distribution(generator), 0, distribution(generator)) + Position();
			}
			else if (msg.type == EMessageType::Msg_TankGoto)
			{
				m_State = EState::Evade;
				m_TargetPosition = MouseTarget3DPos + CVector3(0, 0.5f, 0);
			}
			else if (msg.type == EMessageType::Msg_TankBecomeTeamLeader)
			{
				m_MemberState = ETankTeamMembership::teamLeader;
				m_TeamMemberNumber = TeamManager.GetTankMemberNumber(m_Team, m_UID);
			}

			else if (msg.type == EMessageType::Msg_TankBecomeTeamMember)
			{
				m_MemberState = ETankTeamMembership::teamMember;
				m_TeamMemberNumber = TeamManager.GetTankMemberNumber(m_Team, m_UID);
			}

			else if (msg.type == EMessageType::Msg_TankHelp)
			{
				auto hurtTank = EntityManager.GetEntity(msg.from);
				auto normalVectorToTarget = Normalise(hurtTank->Position() - Position());
				m_State = EState::Evade;
				m_TargetPosition = hurtTank->Position() - normalVectorToTarget * teamMemberSpace;
			}

			else if (msg.type == EMessageType::Msg_GiveAmmo)
			{
				// get 10% ammo
				auto maxAmmo = m_TankTemplate->GetShellAmmo();
				m_Ammo += maxAmmo * 0.1f;
				if (m_Ammo > maxAmmo) // cap at max
					m_Ammo = maxAmmo;
			}
		}


		// Tank behaviour
		// Only move if in Go state
		if (m_State == EState::Patrol)
		{
			if (m_MemberState == ETankTeamMembership::teamMember)
				m_TargetPosition = TeamManager.GetTankPos(m_Team, m_TeamMemberNumber);

			// check if at waypoint
			if (Distance(m_TargetPosition, Position()) <= waypointRadious)
			{
				// next waypoint and face target
				++m_Waypoint;
				if (m_Waypoint >= GetMaxWaypoints(m_Team))
					m_Waypoint = 0;
				m_TargetPosition = GetWaypoint(m_Team, m_Waypoint);
			}

			// target in range
			const int NumOfTeams = TeamManager.GetNumberOfTeams();
			for (int i = 0; i < NumOfTeams; ++i)
			{
				if (i == m_Team)	// skip own team
					continue;

				const int teamSize = TeamManager.GetTeamSize(i);
				for (int j = 0; j < teamSize; ++j)
				{
					const auto enemyUID = TeamManager.GetTankUID(i, j);
					auto enemy = EntityManager.GetEntity(enemyUID);
					if (enemy != nullptr)
					{
						const auto targetVector = enemy->Position() - Position();
						auto turret = Matrix(2) * Matrix();
						const auto rotationToTarget = Dot(Normalise(targetVector), Normalise(turret.ZAxis()));

						if (abs(rotationToTarget) < turretAngularVision && !Ray.HitBuilding(Position(), turret.ZAxis(), enemy->Position()))
						{
							m_State = EState::Aim;
							m_TurretSpeed = 0;
							m_Speed = 0;
							m_TurnSpeed = 0;
							m_Countdown = 1.0f;
							m_Target = enemyUID;
						}
					}
				}
			}

			const auto speedAndTurn = AccAndTurn(m_TargetPosition, updateTime);
			m_Speed = speedAndTurn.first;
			m_TurnSpeed = speedAndTurn.second;
		}
		else if (m_State == EState::Aim)
		{
			// aim
			auto enemy = EntityManager.GetEntity(m_Target);
			if (enemy == nullptr) // enemy dies
			{
				if (m_Ammo <= 0)
					FindAmmo();
				else
					m_State = EState::Patrol;
			}
			auto targetVector = enemy->Position() - Position();
			auto turret = Matrix(2) * Matrix();
			auto rotationToTarget = Dot(Normalise(targetVector), Normalise(turret.ZAxis()));
			bool toRight = (Dot(targetVector, turret.XAxis()) > 0) ? true : false;
			TFloat32 acosRot = acos(rotationToTarget);
			if (acosRot > minRotation)
			{
				if (toRight)
					m_TurretSpeed = Min(acosRot, m_TankTemplate->GetTurretTurnSpeed()) * turretAimSpeedMultiplyer;
				else
					m_TurretSpeed = Min(acosRot, -(m_TankTemplate->GetTurretTurnSpeed())) * turretAimSpeedMultiplyer;
			}

			if (m_Countdown <= 0)
			{
				if (!Ray.HitBuilding(Position(), turret.ZAxis(), enemy->Position()))
				{
					// fire
					auto bulletUID = EntityManager.CreateShell("Shell Type 1", "Bullet", turret.Position() + turret.ZAxis() * barrelLenght, CVector3(0, 0, 0));
					auto bullet = EntityManager.GetEntity(bulletUID);
					bullet->Matrix().FaceDirection(turret.ZAxis());
					auto shell = dynamic_cast<CShellEntity*>(bullet);
					shell->BulletOwner(m_UID);
					--m_Ammo;

					// change state
					m_State = EState::Evade;
					m_TargetPosition = CVector3(distribution(generator), 0, distribution(generator)) + Position();
				}
				else
				{
					if (m_Ammo <= 0)
						FindAmmo();
					else
						m_State = EState::Patrol;
				}
			}

			// countdown
			else
			{
				m_Countdown -= updateTime;
				if (m_Countdown <= 0)
					m_Countdown = 0;
			}
		}
		else if (m_State == EState::Evade)
		{
			// reset the turret
			auto turret = Matrix(2) * Matrix();
			const auto rotationToNeutral = Dot(Normalise(Matrix().ZAxis()), Normalise(turret.ZAxis()));
			const bool toRight = (Dot(Matrix().ZAxis(), turret.XAxis()) > 0) ? true : false;
			const TFloat32 acosRot = acos(rotationToNeutral);
			if (acosRot > minRotation)
			{
				if (toRight)
					m_TurretSpeed = Min(acosRot, m_TankTemplate->GetTurretTurnSpeed()) * turretAimSpeedMultiplyer;
				else
					m_TurretSpeed = Min(acosRot, -(m_TankTemplate->GetTurretTurnSpeed())) * turretAimSpeedMultiplyer;
			}

			if (Distance(m_TargetPosition, Position()) <= waypointRadious)
			{
				m_State = EState::Patrol;
				m_TargetPosition = GetWaypoint(m_Team, m_Waypoint);
				m_TurretSpeed = m_TankTemplate->GetTurretTurnSpeed();
			}

			const auto speedAndTurn = AccAndTurn(m_TargetPosition, updateTime);
			m_Speed = speedAndTurn.first;
			m_TurnSpeed = speedAndTurn.second;
		}
		else if (m_State == EState::GettingAmmo)
		{
			//if target is null get another
			if (EntityManager.GetEntity(m_Target) == nullptr)
				FindAmmo();

			// goto target
			const auto speedAndTurn = AccAndTurn(m_TargetPosition, updateTime);
			m_Speed = speedAndTurn.first;
			m_TurnSpeed = speedAndTurn.second;
		}
		else	// EState::Inactive and catches unknown states
		{
		}

		// Perform movement...
		// Move along local Z axis scaled by update time
		Matrix().MoveLocalZ(m_Speed * updateTime);
		Matrix().RotateLocalY(m_TurnSpeed * updateTime);

		// turret
		Matrix(2).RotateLocalY(m_TurretSpeed * updateTime);

		// mgs to display text on screen
		msg.type = EMessageType::Msg_DisplayEntityInfo;
		msg.from = GetUID();
		Messenger.SendMessage(TextSystemUID, msg);
	}
	

	return true; // Don't destroy the entity
}


} // namespace gen
