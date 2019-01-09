#include "TeamManager.h"

gen::CTeamManager::CTeamManager()
{
	m_NumOfTeams = 0;
	m_TotalNumberOfTanks = 0;
}

gen::CTeamManager::~CTeamManager()
{
}

void gen::CTeamManager::AddTank(TEntityUID tankUID, int team)
{
	// if team doesnt exist yet make empties
	while (m_NumOfTeams <= team)
	{
		vector<TEntityUID> emptyTeam;
		m_Teams.push_back(emptyTeam);
		++m_NumOfTeams;
		m_TeamSizes.push_back(0);
	}

	// add tank
	m_Teams.at(team).push_back(tankUID);
	++m_TotalNumberOfTanks;
	++m_TeamSizes[team];
}

int gen::CTeamManager::GetTeamSize(int team)
{
	if (m_NumOfTeams < team)
		return -1;
	return m_TeamSizes.at(team);
}

gen::TEntityUID gen::CTeamManager::GetTankUID(int team, int memberNumber)
{
	if (m_NumOfTeams < team)
		return TEntityUID();
	return m_Teams.at(team).at(memberNumber);
}
