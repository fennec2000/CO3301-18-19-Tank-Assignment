///////////////////////////////////////////////////////////
//  CParseLevel.cpp
//  A class to parse and setup a level (entity templates
//  and instances) from an XML file
//  Created on:      30-Jul-2005 14:40:00
//  Original author: LN
///////////////////////////////////////////////////////////

#ifndef GEN_C_PARSE_LEVEL_H_INCLUDED
#define GEN_C_PARSE_LEVEL_H_INCLUDED

#include <string>
#include <vector>
#include <list>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "EntityManager.h"
#include "CParseXML.h"
#include "TeamManager.h"

namespace gen
{

/*---------------------------------------------------------------------------------------------
	CParseLevel class
---------------------------------------------------------------------------------------------*/
// A XML parser to read and setup a level - made up of entity templates and entity instances
// Derived from the general CParseXML class, which performs the basic syntax parsing. The base
// class calls functions (overridden) in this class when it encounters the start and end of
// elements in the XML (opening and closing tags). These functions then perform appropriate
// setup. This is an event driven system, requiring this class to store state - the entity /
// template / member variables it is currently building
class CParseLevel : public CParseXML
{

/*---------------------------------------------------------------------------------------------
	Constructors / Destructors
---------------------------------------------------------------------------------------------*/
public:
	// Constructor gets a pointer to the entity manager and initialises state variables
	CParseLevel(CEntityManager* entityManager, CTeamManager* teamManager);
	
/*-----------------------------------------------------------------------------------------
	Private interface
-----------------------------------------------------------------------------------------*/
private:

	/*---------------------------------------------------------------------------------------------
		Types
	---------------------------------------------------------------------------------------------*/

	// File section currently being parsed
	enum EFileSection
	{
		None,
		Templates,
		Entities,
		Waypoints
	};


	/*---------------------------------------------------------------------------------------------
		Callback functions
	---------------------------------------------------------------------------------------------*/

	// Callback function called when the parser meets the start of a new element (the opening tag).
	// The element name is passed as a string. The attributes are passed as a list of (C-style)
	// string pairs: attribute name, attribute value. The last attribute is marked with a null name
	void StartElt( const string& eltName, SAttribute* attrs );

	// Callback function called when the parser meets the end of an element (the closing tag). The
	// element name is passed as a string
	void EndElt( const string& eltName );


	/*---------------------------------------------------------------------------------------------
		Section Parsing
	---------------------------------------------------------------------------------------------*/

	// Called when the parser meets the start of an element (opening tag) in the templates section
	void TemplatesStartElt( const string& eltName, SAttribute* attrs );

	// Called when the parser meets the end of an element (closing tag) in the templates section
	void TemplatesEndElt( const string& eltName );

	// Called when the parser meets the start of an element (opening tag) in the entities section
	void EntitiesStartElt( const string& eltName, SAttribute* attrs );

	// Called when the parser meets the end of an element (closing tag) in the entities section
	void EntitiesEndElt( const string& eltName );

	// Called when the parser meets the start of an element (opening tag) in the entities section
	void WaypointsStartElt(const string& eltName, SAttribute* attrs);

	// Called when the parser meets the end of an element (closing tag) in the entities section
	void WaypointsEndElt(const string& eltName);

	// Called when the parser meets the start of an element (opening tag) of a entity component
	void ComponentsStartElt( const string& typeName, SAttribute* attrs );


	/*---------------------------------------------------------------------------------------------
		Data
	---------------------------------------------------------------------------------------------*/

	// Constructer is passed a pointer to an entity manager used to create templates and
	// entities as they are parsed
	CEntityManager* m_EntityManager;
	CTeamManager* m_TeamManager;

	vector<string> m_TankNames; // list of names of tanks for creating tank entity

	// File state
	EFileSection m_CurrentSection;

	// Current template state (i.e. latest values read during parsing)
	string   m_TemplateType;
	string   m_TemplateName;
	string   m_TemplateMesh;

	// Current entity state (i.e. latest values read during parsing)
	CEntity* m_Entity;
	int      m_TankTeam;
	string   m_EntityType;
	string   m_EntityName;
	TInt32   m_EntityNumber;
	CVector3 m_Pos;
	bool     m_PosRand;      // random flag for postition
	CVector3 m_PosRandValue; // size of random
	CVector3 m_Rot;
	bool     m_RotRand;      // random flag for rotation
	CVector3 m_RotRandValue; // size of random
	CVector3 m_Scale;

	// Current waypoint state (i.e. latest values read during parsing)
	vector<CVector3> m_List;
	CVector3         m_WaypointPos;
};


} // namespace gen

#endif // GEN_C_PARSE_LEVEL_H_INCLUDED
