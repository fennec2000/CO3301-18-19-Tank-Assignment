#include "Powerup.h"
#include "TeamManager.h"

namespace gen
{
// globals
const TFloat32 powerupRotationSpeed = ToRadians( 30.0f );
const CVector3 hiddenPos = {0, -100, 0 };
const TFloat32 powerupCollisionDistance = 5.0f;

// externs
extern CTeamManager TeamManager;
extern CEntityManager EntityManager;
extern CMessenger Messenger;

CPowerupEntity::CPowerupEntity
(
	CEntityTemplate* entityTemplate,
	TEntityUID       UID,
	const string&    name /*=""*/,
	const CVector3&  position /*= CVector3::kOrigin*/,
	const CVector3&  rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
	const CVector3&  scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
) : CEntity(entityTemplate, UID, name, position, rotation, scale)
{
	m_UID = UID;
	m_State = state::active;
}
bool CPowerupEntity::Update(TFloat32 updateTime)
{
	if (m_State == state::active)
	{
		// rotate
		Matrix().RotateLocalY(powerupRotationSpeed * updateTime);

		// picked up
		const int NumOfTeams = TeamManager.GetNumberOfTeams();
		for (int i = 0; i < NumOfTeams; ++i)
		{
			const int teamSize = TeamManager.GetTeamSize(i);
			for (int j = 0; j < teamSize; ++j)
			{
				auto tankUID = TeamManager.GetTankUID(i, j);
				auto tank = EntityManager.GetEntity(tankUID);
				if (tank != nullptr && Distance(Position(), tank->Position()) < powerupCollisionDistance)
				{
					// send ammo msg
					SMessage msg;
					msg.type = EMessageType::Msg_GiveAmmo;
					msg.from = m_UID;
					Messenger.SendMessage(tankUID, msg);

					// go to respawning
					m_State = state::respawning;
					m_CurrentTimer = m_RespawnTimer;
					Matrix().SetPosition(hiddenPos);

					return true; // no need to continue
				}
			}
		}
	}
	else if (m_State == state::respawning)
	{
		m_CurrentTimer -= updateTime;
		if (m_CurrentTimer <= 0)
		{
			// goto active
			m_State = state::active;
			Matrix().SetPosition(m_DefaultPos);

			return true; // no need to continue
		}
	}

	// you must live
	return true;
}
}