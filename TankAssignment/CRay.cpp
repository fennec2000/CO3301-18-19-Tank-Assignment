#include "CRay.h"

gen::CRay::CRay(CEntityManager* entityManager)
{
	m_EntityManager = entityManager;
	m_Building = m_EntityManager->GetEntity("Building");
	m_BuildingBounds[0] = -7.36113;  // -x
	m_BuildingBounds[1] = -0.148627; // -y
	m_BuildingBounds[2] = -4.34613;  // -z
	m_BuildingBounds[3] = 5.11745;   //  x
	m_BuildingBounds[4] = 11.1836;   //  y
	m_BuildingBounds[5] = 5.35663;   //  z
	m_BuildingOuterSphere = sqrt(m_BuildingBounds[0] * m_BuildingBounds[0] +
		m_BuildingBounds[4] * m_BuildingBounds[4] +
		m_BuildingBounds[5] * m_BuildingBounds[5]);
}

bool gen::CRay::HitBuilding(CVector3 origin, CVector3 direction, CVector3 target)
{
	if (Distance(origin, m_Building->Position()) - m_BuildingOuterSphere < Distance(origin, target))
	{
		// check on plane first
		float x1 = m_Building->Position().x + m_BuildingBounds[0], x2 = m_Building->Position().x + m_BuildingBounds[3];
		float y1 = m_Building->Position().y + m_BuildingBounds[1], y2 = m_Building->Position().y + m_BuildingBounds[4];

		float t1x = (x1 - origin.x) / direction.x;
		float t1y = (y1 - origin.y) / direction.y;
		float tmin = (t1x > t1y) ? t1x : t1y;

		float t2x = (x2 - origin.x) / direction.x;
		float t2y = (y2 - origin.y) / direction.y;
		float tmax = (t2x < t2y) ? t2x : t2y;

		if (t1x > t2y || t1y > t2x)	// miss the box
			return false;

		// add z to make cube
		float z1 = m_Building->Position().z + m_BuildingBounds[2], z2 = m_Building->Position().z + m_BuildingBounds[5];

		float t1z = (z1 - origin.z) / direction.z;
		float t2z = (z2 - origin.z) / direction.z;
		
		if (tmin > t2z || t1z > tmax) // miss the cube
			return false;

		return true; // ray passes through building
	}
	return false;
}

gen::CRay::~CRay()
{
}
