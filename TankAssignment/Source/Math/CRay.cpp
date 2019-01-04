#include "CRay.h"

gen::CRay::CRay(CEntityManager* entityManager)
{
	m_EntityManager = entityManager;
}

bool gen::CRay::HitBuilding(CVector3 origin, CVector3 direction, CVector3 target)
{
	if (Distance(origin, m_Building->Position()) - m_BuildingOuterSphere < Distance(origin, target))
	{
		// check on plane first
		const float x1 = m_Building->Position().x + m_BuildingBounds[0], x2 = m_Building->Position().x + m_BuildingBounds[3];
		const float y1 = m_Building->Position().y + m_BuildingBounds[1], y2 = m_Building->Position().y + m_BuildingBounds[4];

		float tmin = (x1 - origin.x) / direction.x;
		float tmax = (x2 - origin.x) / direction.x;
		if (tmin > tmax)
			swap(tmin, tmax);

		float tymin = (y1 - origin.y) / direction.y;
		float tymax = (y2 - origin.y) / direction.y;
		if (tymin > tymax)
			swap(tymin, tymax);

		if (tmin > tymax || tymin > tmax)	// miss the box
			return false;

		if (tymin > tmin)
			tmin = tymin;

		if (tymax < tmax)
			tmax = tymax;

		// add z to make cube
		float z1 = m_Building->Position().z + m_BuildingBounds[2], z2 = m_Building->Position().z + m_BuildingBounds[5];

		float tzmin = (z1 - origin.z) / direction.z;
		float tzmax = (z2 - origin.z) / direction.z;
		if (tzmin > tzmax)
			swap(tzmin, tzmax);
		
		if (tmin > tzmax || tzmin > tmax)  // miss the cube
			return false;

		return true; // ray passes through building
	}
	return false;
}

void gen::CRay::Setup()
{
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

gen::CRay::~CRay()
{
}
