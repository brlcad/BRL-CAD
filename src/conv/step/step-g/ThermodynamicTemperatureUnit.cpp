/*                 ThermodynamicTemperatureUnit.cpp
 * BRL-CAD
 *
 * Copyright (c) 1994-2021 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file step/ThermodynamicTemperatureUnit.cpp
 *
 * Routines to convert STEP "ThermodynamicTemperatureUnit" to BRL-CAD BREP
 * structures.
 *
 */

#include "STEPWrapper.h"
#include "Factory.h"

#include "ThermodynamicTemperatureUnit.h"

#define CLASSNAME "ThermodynamicTemperatureUnit"
#define ENTITYNAME "Thermodynamic_Temperature_Unit"
string ThermodynamicTemperatureUnit::entityname = Factory::RegisterClass(ENTITYNAME, (FactoryMethod)ThermodynamicTemperatureUnit::Create);

ThermodynamicTemperatureUnit::ThermodynamicTemperatureUnit()
{
    step = NULL;
    id = 0;
}

ThermodynamicTemperatureUnit::ThermodynamicTemperatureUnit(STEPWrapper *sw, int step_id)
{
    step = sw;
    id = step_id;
}

ThermodynamicTemperatureUnit::~ThermodynamicTemperatureUnit()
{
}

bool
ThermodynamicTemperatureUnit::Load(STEPWrapper *sw, SDAI_Application_instance *sse)
{
    step = sw;
    id = sse->STEPfile_id;


    // load base class attributes
    if (!NamedUnit::Load(step, sse)) {
	std::cout << CLASSNAME << ":Error loading base class ::Unit." << std::endl;
	sw->entity_status[id] = STEP_LOAD_ERROR;
	return false;
    }
    sw->entity_status[id] = STEP_LOADED;
    return true;
}

void
ThermodynamicTemperatureUnit::Print(int level)
{
    TAB(level);
    std::cout << CLASSNAME << ":" << "(";
    std::cout << "ID:" << STEPid() << ")" << std::endl;

    TAB(level);
    std::cout << "Inherited Attributes:" << std::endl;
    NamedUnit::Print(level + 1);

}

STEPEntity *
ThermodynamicTemperatureUnit::GetInstance(STEPWrapper *sw, int id)
{
    return new ThermodynamicTemperatureUnit(sw, id);
}

STEPEntity *
ThermodynamicTemperatureUnit::Create(STEPWrapper *sw, SDAI_Application_instance *sse)
{
    return STEPEntity::CreateEntity(sw, sse, GetInstance, CLASSNAME);
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
