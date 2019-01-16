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
	CEntityManager* m_EntityManager;	// pointer to entity manger
	int m_NumOfTeams;					// number of teams made
	vector<vector<TEntityUID>> m_Teams;	// list of team and there members
	vector<int> m_TeamSizes;			// list of team sizes
	vector<int> m_TeamLeaders;			// list of team leaders
	vector<EFormation> m_TeamFormation;	// teams current formation

public:
	// constructor / destructor
	CTeamManager(CEntityManager* entityManager);
	~CTeamManager();

	// Getters
	inline int GetNumberOfTeams() noexcept { return m_NumOfTeams; }
	int GetTeamSize(int team);
	TEntityUID GetTankUID(int team, int memberNumber);
	int GetTankMemberNumber(int team, TEntityUID UID);
	CVector3 GetTankPos(int team, int memberNumber);

	int AddTank(TEntityUID tankUID, int team);	// adds a tank to a team
	bool UpdateMembership(int team);			// checks team leader, re-asign team leader if ther is none and updates position in team
};
}


