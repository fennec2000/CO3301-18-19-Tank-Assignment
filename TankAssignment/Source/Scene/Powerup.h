///////////////////////////////////////////////////////////
//  CPowerup.h
//  A class to cast rays and if the tanks hit each other
//  or the building
//  Created on:      14-01-2019 13:25:00
//  Original author: Stuart Hayes
///////////////////////////////////////////////////////////
#pragma once
#include <string>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"

namespace gen
{
class CPowerupEntity : public CEntity
{
public:
	enum state { active, respawning };
	CPowerupEntity
	(
		CEntityTemplate* entityTemplate,
		TEntityUID       UID,
		const string&    name = "",
		const CVector3&  position = CVector3::kOrigin,
		const CVector3&  rotation = CVector3(0.0f, 0.0f, 0.0f),
		const CVector3&  scale = CVector3(1.0f, 1.0f, 1.0f)
	);

	virtual bool Update(TFloat32 updateTime);

private:
	TEntityUID m_UID;			// my uid
	CVector3 m_DefaultPos;		// default spawning position
	TFloat32 m_RespawnTimer;	// time to respawn
	TFloat32 m_CurrentTimer;	// current time till respawn
	state m_State;				// powerup state
};
}


