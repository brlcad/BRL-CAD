/*                 LuminousIntensityUnit.cpp
 * BRL-CAD
 *
 * Copyright (c) 1994-2009 United States Government as represented by
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
/** @file LuminousIntensityUnit.cpp
 *
 * Routines to convert STEP "LuminousIntensityUnit" to BRL-CAD BREP
 * structures.
 *
 */

#include "STEPWrapper.h"
#include "Factory.h"

#include "LuminousIntensityUnit.h"

#define CLASSNAME "LuminousIntensityUnit"
#define ENTITYNAME "Luminous_Intensity_Unit"
string LuminousIntensityUnit::entityname = Factory::RegisterClass(ENTITYNAME,(FactoryMethod)LuminousIntensityUnit::Create);

LuminousIntensityUnit::LuminousIntensityUnit() {
	step = NULL;
	id = 0;
}

LuminousIntensityUnit::LuminousIntensityUnit(STEPWrapper *sw,int STEPid) {
	step = sw;
	id = STEPid;
}

LuminousIntensityUnit::~LuminousIntensityUnit() {
}

bool
LuminousIntensityUnit::Load(STEPWrapper *sw,SCLP23(Application_instance) *sse) {
	step=sw;
	id = sse->STEPfile_id;


	// load base class attributes
	if ( !NamedUnit::Load(step,sse) ) {
		cout << CLASSNAME << ":Error loading base class ::Unit." << endl;
		return false;
	}

	return true;
}

void
LuminousIntensityUnit::Print(int level) {
	TAB(level); cout << CLASSNAME << ":" << "(";
	cout << "ID:" << STEPid() << ")" << endl;

	TAB(level); cout << "Inherited Attributes:" << endl;
	NamedUnit::Print(level+1);
}
STEPEntity *
LuminousIntensityUnit::Create(STEPWrapper *sw, SCLP23(Application_instance) *sse) {
	Factory::OBJECTS::iterator i;
	if ((i = Factory::FindObject(sse->STEPfile_id)) == Factory::objects.end()) {
		LuminousIntensityUnit *object = new LuminousIntensityUnit(sw,sse->STEPfile_id);

		Factory::AddObject(object);

		if (!object->Load(sw, sse)) {
			cerr << CLASSNAME << ":Error loading class in ::Create() method." << endl;
			delete object;
			return NULL;
		}
		return static_cast<STEPEntity *>(object);
	} else {
		return (*i).second;
	}
}

// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
