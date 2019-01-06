#pragma once
#include <vector>

#include "EntityManager.h"

namespace gen
{
class CTeamManager
{
private:
	int m_NumOfTeams;
	vector<vector<TEntityUID>> m_Teams;
	vector<int> m_TeamSizes;
	int m_TotalNumberOfTanks;

public:
	CTeamManager();
	~CTeamManager();
	void AddTank(TEntityUID tankUID, int team);
	inline int GetNumberOfTeams() noexcept { return m_NumOfTeams; }
	int GetTeamSize(int team);
	TEntityUID GetTankUID(int team, int memberNumber);
	inline int GetTotalNumberOfTanks() noexcept { return m_TotalNumberOfTanks; }
};
}


