#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "manta.h"
#include "advection.h"
#include "grid.h"
#include "levelset.h"
#include "surfaceturbulence.h"
#include "particle.h"
#include "mesh.h"
#include "shapes.h"
#include "flip.h"
#include "fastmarch.h"
#include "extforces.h"
#include "pressure.h"
#include "initplugins.h"


namespace fs = std::experimental::filesystem;

int main()
{
    // get grid resolution:
    int res(0);
    do
    {
        std::cout << "Enter grid resolution (between 10 and 500, default 32): ";
        std::cin >> res;
    } while ((res < 10) || (res > 500));

    // check for existing output paths, create if necessary
    if (!fs::exists("simulation data")) fs::create_directory("simulation data");
    if (!fs::exists("simulation data/surfaceturbulence")) fs::create_directory("simulation data/surfaceturbulence");

    int  dimension  = 3;
    auto resolution = Manta::Vec3i(res);
    if (dimension == 2) resolution.z = 1;

    auto main_solver = Manta::FluidSolver(resolution, dimension);
    main_solver.setTimeStep(0.8f);
    Real minParticles = pow(2, dimension);
    Real radiusFactor = 1.0f;

    auto gravity = Manta::Vec3(0.0f, -0.001f, 0.0f);

    // prepare grids and particles
    auto flags      = Manta::FlagGrid(&main_solver);
    auto phi        = Manta::LevelsetGrid(&main_solver);

    auto vel        = Manta::MACGrid(&main_solver);
    auto velOld     = Manta::MACGrid(&main_solver);
    auto pressure   = Manta::Grid<Real>(&main_solver);
    auto tmpVec3    = Manta::Grid<Manta::Vec3>(&main_solver);

    auto pp         = Manta::BasicParticleSystem(&main_solver);
    auto pVel       = Manta::PdataVec3(&pp);
    auto pPrevPos   = Manta::PdataVec3(&pp);
    auto mesh       = Manta::Mesh(&main_solver);

    auto surfacePoints              = Manta::BasicParticleSystem(&main_solver);
    auto surfacePointsDisplaced     = Manta::BasicParticleSystem(&main_solver);
    auto surfaceNormal              = Manta::PdataVec3(&surfacePoints);
    auto surfaceWaveH               = Manta::PdataReal(&surfacePoints);
    auto surfaceWaveDtH             = Manta::PdataReal(&surfacePoints);
    auto surfaceWaveSource          = Manta::PdataReal(&surfacePoints);
    auto surfaceWaveSeedAmplitude   = Manta::PdataReal(&surfacePoints);
    auto surfaceWaveSeed            = Manta::PdataReal(&surfacePoints);
    auto spdDummy                   = Manta::PdataVec3(&surfacePointsDisplaced); // dummy for display

    // acceleration data for particle nbs
    auto pindex     = Manta::ParticleIndexSystem(&main_solver);
    auto gpi        = Manta::Grid<int>(&main_solver);

    // scene setup, 0=breaking dam, 1=drop into pool
    int setup = 0;
    int bWidth = 1;
    auto fluidVel = Manta::Sphere(&main_solver, Manta::Vec3(0.0f), 1.0f);
    Manta::Vec3 fluidSetVel;
    flags.initDomain(1);

    if (setup == 0)
    {
        // breakin dam
        auto fluidbox   = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                     Manta::Vec3(0.0f, 0.0f, 0.0f),
                                     Manta::Vec3(resolution.x * 0.4f, resolution.y * 0.4f, resolution.z * 1.0f));
        phi.copyFrom(fluidbox.computeLevelset());

    } else if (setup == 1)
    {
        // falling drop
        auto fluidbasin = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                     Manta::Vec3(0.0f, 0.0f, 0.0f),
                                     Manta::Vec3(resolution.x * 1.0f, resolution.y * 0.1f, resolution.z * 1.0f));
        auto dropCenter = Manta::Vec3(0.5f, 0.3f, 0.5f);
        Real dropRadius = 0.1f;
        auto fluidDrop  = Manta::Sphere(&main_solver, Manta::Vec3(resolution.x * dropCenter.x, resolution.y * dropCenter.y, resolution.z * dropCenter.z),
                                        resolution.x * dropRadius);
        fluidVel        = Manta::Sphere(&main_solver, Manta::Vec3(resolution.x * dropCenter.x, resolution.y * dropCenter.y, resolution.z * dropCenter.z),
                                        resolution.x * (dropRadius + 0.05f));
        fluidSetVel     = Manta::Vec3(0.0f, -1.0f, 0.0f);
        phi.copyFrom(fluidbasin.computeLevelset());
        phi.join(fluidDrop.computeLevelset());
    }

    flags.updateFromLevelset(phi);
    Manta::sampleLevelsetWithParticles(phi, flags, pp, 2, 0.35f);

    for (int t = 0; t < 500; t++)
    {
        // advect particles and grid phi
        pp.advectInGrid(flags, vel, Manta::IntegrationMode::IntRK4, false, false);

        Manta::mapPartsToMAC(flags, vel, velOld, pp, pVel, &tmpVec3);
        Manta::extrapolateMACFromWeight(vel, tmpVec3, 2);
        Manta::markFluidCells(pp, flags);

        // create level set of particles
        Manta::gridParticleIndex(pp, pindex, flags, gpi);
        Manta::unionParticleLevelset(pp, pindex, flags, gpi, phi, radiusFactor);
        Manta::resetOutflow(flags, nullptr, &pp, nullptr, &gpi, &pindex);
        extrapolateLsSimple(phi, 4, true);

        // forces and pressure solve
        Manta::addGravity(flags, vel, gravity);
        Manta::setWallBcs(flags, vel);
        Manta::solvePressure(vel, pressure, flags, 1e-3, &phi);
        Manta::setWallBcs(flags, vel);

        // set source grids for resampling, used in adjustNumber!
        pVel.setSource(&vel, true);
        Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor);

        // make sure we have proper velocities
        Manta::extrapolateMACSimple(flags, vel);
        Manta::flipVelocityUpdate(flags, vel, velOld, pp, pVel, 0.97f);

        Manta::SurfaceTurbulence::particleSurfaceTurbulence(flags, pp, pPrevPos, surfacePoints, surfaceNormal, surfaceWaveH, surfaceWaveDtH, surfacePointsDisplaced, surfaceWaveSource, surfaceWaveSeed, surfaceWaveSeedAmplitude, res,
                                         1.0 * radiusFactor, 12, 4, 0.005, 32, 0.05, 4.0, 0.5, 128.0, 0.5, 0.025, 0.01, 0.05);
        if (dimension == 3) phi.createMesh(mesh);

        main_solver.step();

    	// generate data directly and for flip03_gen.exe surface generation scene
        std::stringstream filename1, filename2;
        filename1 << "simulation data\\surfaceturbulence\\fluidsurface_final_" << std::setw(4) << std::setfill('0') << t << ".bobj.gz";
        filename2 << "simulation data\\surfaceturbulence\\flipParts_"          << std::setw(4) << std::setfill('0') << t << ".uni";
        mesh.save(filename1.str());
        surfacePointsDisplaced.save(filename2.str());
    }
}
