///////////////////////////////////////////////////////////
//  CParseLevel.cpp
//  A class to cast rays and if the tanks hit each other
//  or the building
//  Created on:      03-01-2019 13:40:00
//  Original author: Stuart Hayes
///////////////////////////////////////////////////////////
using namespace std;
#include "EntityManager.h"
#include "CVector3.h"

namespace gen
{

class CRay
{
private:
	CEntityManager* m_EntityManager;		// pointer to enetity manger
	CEntity* m_Building;					// pointer to the single building in the map
	float m_BuildingBounds[6];				// the hitbox for the build axis aligned xyzxyz
	float m_BuildingOuterSphere;			// a sphere around the building for simple calucations
public:
	CRay(CEntityManager* entityManager);
	bool HitBuilding(CVector3 origin, CVector3 direction, CVector3 target);
	void Setup();	// Setup ray after map has loaded
	~CRay();
};
}


