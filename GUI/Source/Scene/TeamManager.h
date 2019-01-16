#pragma once
#include <vector>

#include "EntityManager.h"
#include "Messenger.h"



namespace gen
{
// extern
extern CMessenger Messenger;

// global
const float teamMemberSpace = 15.0f;
enum class EFormation { line, square, surround };

class CTeamManager
{
private:
	CEntityManager* m_EntityManager;
	int m_NumOfTeams;
	vector<vector<TEntityUID>> m_Teams;
	vector<int> m_TeamSizes;
	vector<int> m_TeamLeaders;
	int m_TotalNumberOfTanks;
	vector<EFormation> m_TeamFormation;

public:
	CTeamManager(CEntityManager* entityManager);
	~CTeamManager();
	int AddTank(TEntityUID tankUID, int team);
	inline int GetNumberOfTeams() noexcept { return m_NumOfTeams; }
	int GetTeamSize(int team);
	TEntityUID GetTankUID(int team, int memberNumber);
	inline int GetTotalNumberOfTanks() noexcept { return m_TotalNumberOfTanks; }
	int GetTankMemberNumber(int team, TEntityUID UID);
	CVector3 GetTankPos(int team, int memberNumber);
	bool UpdateMembership(int team);
};
}


