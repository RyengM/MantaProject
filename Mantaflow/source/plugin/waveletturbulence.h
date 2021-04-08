/******************************************************************************
*
* MantaFlow fluid solver framework
* Copyright 2011 Tobias Pfaff, Nils Thuerey
*
* This program is free software, distributed under the terms of the
* GNU General Public License (GPL)
* http://www.gnu.org/licenses
*
* FLIP (fluid implicit particles)
* for use with particle data fields
*
******************************************************************************/

#pragma once

#include "particle.h"
#include "grid.h"
#include "randomstream.h"
#include "levelset.h"

namespace Manta
{

void interpolateGrid(Grid<Real>& target, Grid<Real>& source, Vec3 scale = Vec3(1.), Vec3 offset = Vec3(0.), Vec3i size = Vec3i(-1, -1, -1), int orderSpace = 1);

} // namespace