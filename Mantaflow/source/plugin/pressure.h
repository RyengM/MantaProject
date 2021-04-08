/******************************************************************************
 *
 * MantaFlow fluid solver framework
 * Copyright 2011 Tobias Pfaff, Nils Thuerey 
 *
 * This program is free software, distributed under the terms of the
 * GNU General Public License (GPL) 
 * http://www.gnu.org/licenses
 *
 * Plugins for pressure correction: solve_pressure, and ghost fluid helpers
 *
 ******************************************************************************/

#pragma once

#include "vectorbase.h"
#include "kernel.h"
#include "conjugategrad.h"
#include "multigrid.h"

namespace Manta
{

//! Preconditioner for CG solver
// - None: Use standard CG
// - MIC: Modified incomplete Cholesky preconditioner
// - MGDynamic: Multigrid preconditioner, rebuilt for each solve
// - MGStatic: Multigrid preconditioner, built only once (faster than
//       MGDynamic, but works only if Poisson equation does not change)
enum Preconditioner { PcNone = 0, PcMIC = 1, PcMGDynamic = 2, PcMGStatic = 3 };

void solvePressure(MACGrid& vel, Grid<Real>& pressure, FlagGrid& flags, Real cgAccuracy = 1e-3,
                    Grid<Real>* phi = 0,
                    Grid<Real>* perCellCorr = 0,
                    MACGrid* fractions = 0,
                    Real gfClamp = 1e-04,
                    Real cgMaxIterFac = 1.5,
                    bool precondition = true, // Deprecated, use preconditioner instead
                    int preconditioner = PcMIC,
                    bool enforceCompatibility = false,
                    bool useL2Norm = false,
                    bool zeroPressureFixing = false,
                    Grid<Real>* retRhs = NULL);

} // namespace
