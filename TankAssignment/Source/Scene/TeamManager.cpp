#include "TeamManager.h"

gen::CTeamManager::CTeamManager(CEntityManager* entityManager)
{
	m_EntityManager = entityManager;
	m_NumOfTeams = 0;
}

gen::CTeamManager::~CTeamManager()
{
}

int gen::CTeamManager::AddTank(TEntityUID tankUID, int team)
{
	// if team doesnt exist yet make empties
	while (m_NumOfTeams <= team)
	{
		vector<TEntityUID> emptyTeam;
		m_Teams.push_back(emptyTeam);
		++m_NumOfTeams;
		m_TeamSizes.push_back(0);
		m_TeamFormation.push_back(EFormation::line);
		m_TeamLeaders.push_back(-1);
	}

	// add tank
	m_Teams.at(team).push_back(tankUID);
	return m_TeamSizes[team]++;
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

int gen::CTeamManager::GetTankMemberNumber(int team, TEntityUID UID)
{
	for (int i = 0; i < m_TeamSizes[team]; ++i)
	{
		if (m_Teams.at(team).at(i) == UID)
			return i;
	}
	return -1;
}

gen::CVector3 gen::CTeamManager::GetTankPos(int team, int memberNumber)
{
	auto leaderUID = m_TeamLeaders.at(team);
	auto leaderEntity = m_EntityManager->GetEntity(leaderUID);
	if (leaderEntity == nullptr)
	{
		UpdateMembership(team);
		leaderEntity = m_EntityManager->GetEntity(leaderUID);
		if (leaderEntity == nullptr)
			return CVector3();
	}
	auto leaderPos = leaderEntity->Position();

	if(m_TeamFormation.at(team) == EFormation::line)
		return leaderPos + CVector3(0, 0, teamMemberSpace) * memberNumber;
	else if (m_TeamFormation.at(team) == EFormation::square)
	{
		int teamSizeSqrt = ceil(sqrt(m_TeamSizes[team]));
		return leaderPos + CVector3(memberNumber / teamSizeSqrt, 0, memberNumber % teamSizeSqrt) * teamMemberSpace;
	}

	return CVector3();
}

bool gen::CTeamManager::UpdateMembership(int team)
{
	int i = 0;
	while (i < m_Teams.at(team).size())
	{
		auto uid = m_Teams.at(team).at(i);

		// if dead
		if (m_EntityManager->GetEntity(uid) == nullptr)
		{
			auto myteam = m_Teams.at(team);
			m_Teams.at(team).erase(m_Teams.at(team).begin() + i);
			--m_TeamSizes[team];
		}
		else // update
		{
			// update tank
			SMessage msg;
			msg.from = TeamManagerUID;
			if (i == 0)
			{
				msg.type = EMessageType::Msg_TankBecomeTeamLeader;
				m_TeamLeaders[team] = uid;
			}
			else
				msg.type = EMessageType::Msg_TankBecomeTeamMember;

			Messenger.SendMessage(uid, msg);
			++i;
		}
	}
	return true;
}
