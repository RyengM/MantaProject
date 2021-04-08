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

namespace Manta
{
    namespace SurfaceTurbulence
    {
    void particleSurfaceTurbulence(FlagGrid& flags, BasicParticleSystem& coarseParts,
                                   ParticleDataImpl<Vec3>& coarsePartsPrevPos,
                                   BasicParticleSystem& surfPoints,
                                   ParticleDataImpl<Vec3>& surfaceNormals,
                                   ParticleDataImpl<Real>& surfaceWaveH,
                                   ParticleDataImpl<Real>& surfaceWaveDtH,
                                   BasicParticleSystem& surfacePointsDisplaced,
                                   ParticleDataImpl<Real>& surfaceWaveSource,
                                   ParticleDataImpl<Real>& surfaceWaveSeed,
                                   ParticleDataImpl<Real>& surfaceWaveSeedAmplitude,
                                   // params with default values
                                   int res,
                                   Real outerRadius = 1.0f,
                                   int surfaceDensity = 20,
                                   int nbSurfaceMaintenanceIterations = 4,
                                   Real dt = 0.005f,
                                   Real waveSpeed = 16.0f,
                                   Real waveDamping = 0.0f,
                                   Real waveSeedFrequency = 4,
                                   Real waveMaxAmplitude = 0.25f,
                                   Real waveMaxFrequency = 800,
                                   Real waveMaxSeedingAmplitude = 0.5, // as multiple of max amplitude
                                   Real waveSeedingCurvatureThresholdRegionCenter = 0.025f, // any curvature higher than this value will seed waves
                                   Real waveSeedingCurvatureThresholdRegionRadius = 0.01f,
                                   Real waveSeedStepSizeRatioOfMax = 0.05f); // higher values will result in faster and more violent wave seeding

    } // namespace SurfaceTurbulence
} // namespace Manta
