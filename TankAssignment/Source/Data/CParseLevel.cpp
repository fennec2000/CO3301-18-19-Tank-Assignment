///////////////////////////////////////////////////////////
//  CParseLevel.cpp
//  A class to parse and setup a level (entity templates
//  and instances) from an XML file
//  Created on:      30-Jul-2005 14:40:00
//  Original author: LN
///////////////////////////////////////////////////////////

#include "BaseMath.h"
#include "Entity.h"
#include "CParseLevel.h"

namespace gen
{

extern vector<vector<CVector3>> TeamWaypoints;

/*---------------------------------------------------------------------------------------------
	Constructors / Destructors
---------------------------------------------------------------------------------------------*/

// Constructor initialises state variables
CParseLevel::CParseLevel( CEntityManager* entityManager, CTeamManager* teamManager )
{
	// Take copy of entity manager for creation
	m_EntityManager = entityManager;
	m_TeamManager = teamManager;

	// File state
	m_CurrentSection = None;

	// Template state
	m_TemplateType = "";
	m_TemplateName = "";
	m_TemplateMesh = "";

	// Entity state
	m_EntityType = "";
	m_TankTeam = -1;
	m_EntityName = "";
	m_Pos = CVector3::kOrigin;
	m_PosRand = false;
	m_PosRandValue = CVector3::kOrigin;
	m_Rot = CVector3::kOrigin;
	m_Scale = CVector3(1.0f, 1.0f, 1.0f);

	// Waypoint state
	m_WaypointPos = CVector3::kOrigin;
}


/*---------------------------------------------------------------------------------------------
	Callback Functions
---------------------------------------------------------------------------------------------*/

// Callback function called when the parser meets the start of a new element (the opening tag).
// The element name is passed as a string. The attributes are passed as a list of (C-style)
// string pairs: attribute name, attribute value. The last attribute is marked with a null name
void CParseLevel::StartElt( const string& eltName, SAttribute* attrs )
{
	// Open major file sections
	if (eltName == "Templates")
	{
		m_CurrentSection = Templates;
	}
	else if (eltName == "Entities")
	{
		m_CurrentSection = Entities;
	}
	else if (eltName == "Waypoints")
	{
		m_CurrentSection = Waypoints;
	}

	// Different parsing depending on section currently being read
	switch (m_CurrentSection)
	{
		case Templates:
			TemplatesStartElt( eltName, attrs ); // Parse template start elements
			break;
		case Entities:
			EntitiesStartElt( eltName, attrs ); // Parse entity start elements
			break;
		case Waypoints:
			WaypointsStartElt( eltName, attrs );
			break;
	}
}

// Callback function called when the parser meets the end of an element (the closing tag). The
// element name is passed as a string
void CParseLevel::EndElt( const string& eltName )
{
	// Close major file sections
	if (eltName == "Templates" || eltName == "Entities" || eltName == "Waypoints")
	{
		m_CurrentSection = None;
	}

	// Different parsing depending on section currently being read
	switch (m_CurrentSection)
	{
		case Templates:
			TemplatesEndElt( eltName ); // Parse template end elements
			break;
		case Entities:
			EntitiesEndElt( eltName ); // Parse entity end elements
			break;
		case Waypoints:
			WaypointsEndElt( eltName ); // Parse waypoints end elements
			break;
	}
}


/*---------------------------------------------------------------------------------------------
	Section Parsing
---------------------------------------------------------------------------------------------*/

// Called when the parser meets the start of an element (opening tag) in the templates section
void CParseLevel::TemplatesStartElt( const string& eltName, SAttribute* attrs )
{
	// Started reading a new entity template - get type, name and mesh
	if (eltName == "EntityTemplate")
	{
		// Get attributes held in the tag
		m_TemplateType = GetAttribute( attrs, "Type" );
		m_TemplateName = GetAttribute( attrs, "Name" );
		m_TemplateMesh = GetAttribute( attrs, "Mesh" );
		if (m_TemplateType == "Tank")
		{
			m_EntityManager->CreateTankTemplate(m_TemplateType, m_TemplateName, m_TemplateMesh,
				GetAttributeFloat(attrs, "MaxSpeed"), GetAttributeFloat(attrs, "Acceleration"), GetAttributeFloat(attrs, "TurnSpeed"),
				GetAttributeFloat(attrs, "TurretTurnSpeed"), GetAttributeFloat(attrs, "MaxHP"), GetAttributeFloat(attrs, "ShellDamage"),
				GetAttributeFloat(attrs, "ShellAmmo"));
			m_TankNames.push_back(m_TemplateName);
		}
		else
			m_EntityManager->CreateTemplate( m_TemplateType, m_TemplateName, m_TemplateMesh );
	}
}

// Called when the parser meets the end of an element (closing tag) in the templates section
void CParseLevel::TemplatesEndElt( const string& eltName )
{
	// Nothing to do
}


// Called when the parser meets the start of an element (opening tag) in the entities section
void CParseLevel::EntitiesStartElt( const string& eltName, SAttribute* attrs )
{
	// Started reading a new entity - get type and name
	if (eltName == "Entity")
	{
		m_EntityType = GetAttribute( attrs, "Type" );
		m_EntityName = GetAttribute( attrs, "Name" );
		m_TankTeam = GetAttributeInt(attrs, "Team", -1);
		m_EntityNumber = GetAttributeInt( attrs, "Number", 1 );

		// Set default positions
		m_Pos = CVector3::kOrigin;
		m_Rot = CVector3::kOrigin;
		m_Scale = CVector3(1.0f, 1.0f, 1.0f);
	}

	// Started reading an entity position - get X,Y,Z
	else if (eltName == "Position")
	{
		m_Pos.x = GetAttributeFloat( attrs, "X" );
		m_Pos.y = GetAttributeFloat( attrs, "Y" );
		m_Pos.z = GetAttributeFloat( attrs, "Z" );
	}

	// Started reading an entity rotation - get X,Y,Z
	else if (eltName == "Rotation")
	{
		if (GetAttribute( attrs, "Radians" ) == "true")
		{
			m_Rot.x = GetAttributeFloat( attrs, "X" );
			m_Rot.y = GetAttributeFloat( attrs, "Y" );
			m_Rot.z = GetAttributeFloat( attrs, "Z" );
		}
		else
		{
			m_Rot.x = ToRadians(GetAttributeFloat( attrs, "X" ));
			m_Rot.y = ToRadians(GetAttributeFloat( attrs, "Y" ));
			m_Rot.z = ToRadians(GetAttributeFloat( attrs, "Z" ));
		}
	}

	// Started reading an entity scale - get X,Y,Z
	else if (eltName == "Scale")
	{
		m_Scale.x = GetAttributeFloat( attrs, "X" );
		m_Scale.y = GetAttributeFloat( attrs, "Y" );
		m_Scale.z = GetAttributeFloat( attrs, "Z" );
	}

	// Randomising an entity position - get X,Y,Z amounts and randomise
	else if (eltName == "Randomise")
	{
		m_PosRand = true;
		m_PosRandValue = CVector3(GetAttributeFloat(attrs, "X") * 0.5f,
			GetAttributeFloat(attrs, "Y") * 0.5f,
			GetAttributeFloat(attrs, "Z") * 0.5f);
	}

	// Random Rotation
	else if (eltName == "RandomiseRot")
	{
		m_RotRand = true;
		if (GetAttribute(attrs, "Radians") == "true")
		{
			m_RotRandValue = CVector3(GetAttributeFloat(attrs, "X") * 0.5f,
				GetAttributeFloat(attrs, "Y") * 0.5f,
				GetAttributeFloat(attrs, "Z") * 0.5f);
		}
		else
		{
			m_RotRandValue = CVector3(ToRadians(GetAttributeFloat(attrs, "X")) * 0.5f,
				ToRadians(GetAttributeFloat(attrs, "Y")) * 0.5f,
				ToRadians(GetAttributeFloat(attrs, "Z")) * 0.5f);
		}
	}

	//// Component to add to entity
	//else if (eltName == "Component")
	//{
	//	ComponentsStartElt( GetAttribute( attrs, "Type" ), attrs );
	//}
}

// Called when the parser meets the end of an element (closing tag) in the entities section
void CParseLevel::EntitiesEndElt( const string& eltName )
{
	// Finished reading entity - set its position
	if (eltName == "Entity")
	{
		CVector3 PosRand = CVector3::kOrigin;
		CVector3 RotRand = CVector3::kOrigin;
		for (int i = 0; i < m_EntityNumber; ++i)
		{
			// Create a new entity
			TEntityUID entityUID;
			auto result = find(begin(m_TankNames), end(m_TankNames), m_EntityType);
			if (result != end(m_TankNames))
			{
				entityUID = m_EntityManager->CreateTank(m_EntityType, m_TankTeam, m_EntityName);
			}
			else if (m_EntityType == "Ammo Cube")
			{
				entityUID = m_EntityManager->CreatePowerup(m_EntityType, m_EntityName);
			}
			else
				entityUID = m_EntityManager->CreateEntity(m_EntityType, m_EntityName);

			m_Entity = m_EntityManager->GetEntity(entityUID);

			// random per instance
			if (m_PosRand)
			{
				PosRand.x = Random(-m_PosRandValue.x, m_PosRandValue.x);
				PosRand.y = Random(-m_PosRandValue.y, m_PosRandValue.y);
				PosRand.z = Random(-m_PosRandValue.z, m_PosRandValue.z);
			}
			if (m_RotRand)
			{
				RotRand.x = Random(-m_RotRandValue.x, m_RotRandValue.x);
				RotRand.y = Random(-m_RotRandValue.y, m_RotRandValue.y);
				RotRand.z = Random(-m_RotRandValue.z, m_RotRandValue.z);
			}

			m_Entity->Matrix().MakeAffineEuler(m_Pos + PosRand, m_Rot + RotRand, kZXY, m_Scale);
		}
	}
}

void CParseLevel::WaypointsStartElt(const string& eltName, SAttribute* attrs)
{
	// Started reading a new waypoints
	if (eltName == "Waypoints")
	{
		TeamWaypoints.empty();
	}
	else if (eltName == "List")
	{
		m_List.empty();
	}
	else if (eltName == "Waypoint")
	{
		m_WaypointPos.x = GetAttributeFloat(attrs, "X");
		m_WaypointPos.y = GetAttributeFloat(attrs, "Y");
		m_WaypointPos.z = GetAttributeFloat(attrs, "Z");
		m_List.push_back(m_WaypointPos);
	}
}

// Called when the parser meets the end of an element (closing tag) in the templates section
void CParseLevel::WaypointsEndElt(const string& eltName)
{
	if (eltName == "List")
	{
		TeamWaypoints.push_back(m_List);
	}
}

//****************************************************************************/
//  Component Code
//****************************************************************************/

// Called when the parser meets the start of an element (opening tag) of a entity component
void CParseLevel::ComponentsStartElt( const string& typeName, SAttribute* attrs )
{
	// TODO: 
	//// Create a spin movement component
	//if (typeName == "Spin")
	//{
	//	CVector3 spinSpeed;
	//	spinSpeed.x = GetAttributeFloat( attrs, "X" );
	//	spinSpeed.y = GetAttributeFloat( attrs, "Y" );
	//	spinSpeed.z = GetAttributeFloat( attrs, "Z" );

	//	// Create new component - request new UID from entity manager
	//	CEntityComponent* component = new CSpinComponent( m_Entity, m_EntityManager->GetNewUID(), spinSpeed );
	//	m_Entity->AddComponent( component );
	//}

	//// Create a direct (targeted) movement component
	//if (typeName == "Direct")
	//{
	//	TFloat32 maxSpeed = GetAttributeFloat( attrs, "MaxSpeed" );

	//	// Create new component - request new UID from entity manager
	//	CEntityComponent* component = new CDirectComponent( m_Entity, m_EntityManager->GetNewUID(), maxSpeed );
	//	m_Entity->AddComponent( component );
	//}

	//// Create a (targeted) driving movement component
	//if (typeName == "Drive")
	//{
	//	TFloat32 maxSpeed = GetAttributeFloat( attrs, "MaxSpeed" );
	//	TFloat32 turnSpeed = GetAttributeFloat( attrs, "TurnSpeed" );

	//	// Create new component - request new UID from entity manager
	//	CEntityComponent* component = new CDriveComponent( m_Entity, m_EntityManager->GetNewUID(), maxSpeed, turnSpeed );
	//	m_Entity->AddComponent( component );
	//}

	//// Add random wander component
	//else if (typeName == "Wander")
	//{
	//	TFloat32 minX = GetAttributeFloat( attrs, "MinX" );
	//	TFloat32 maxX = GetAttributeFloat( attrs, "MaxX" );
	//	TFloat32 minY = GetAttributeFloat( attrs, "MinY" );
	//	TFloat32 maxY = GetAttributeFloat( attrs, "MaxY" );
	//	TFloat32 minZ = GetAttributeFloat( attrs, "MinZ" );
	//	TFloat32 maxZ = GetAttributeFloat( attrs, "MaxZ" );
	//	TFloat32 range = GetAttributeFloat( attrs, "Range" );

	//	// Create new component - request new UID from entity manager
	//	CEntityComponent* component = new CWanderComponent( m_Entity, m_EntityManager->GetNewUID(), minX, maxX, minY, maxY, minZ, maxZ, range );
	//	m_Entity->AddComponent( component );
	//}

	//// Add random wander component
	//else if (typeName == "Patrol")
	//{
	//	TFloat32 range = GetAttributeFloat( attrs, "Range" );

	//	// Create new component - request new UID from entity manager
	//	CEntityComponent* component = new CPatrolComponent( m_Entity, m_EntityManager->GetNewUID(), range );
	//	m_Entity->AddComponent( component );
	//}
}
//
//****************************************************************************/


} // namespace gen
