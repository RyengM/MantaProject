#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "manta.h"
#include "grid.h"
#include "levelset.h"
#include "particle.h"
#include "mesh.h"
#include "shapes.h"
#include "flip.h"
#include "fastmarch.h"
#include "extforces.h"
#include "pressure.h"

namespace fs = std::filesystem;

int main()
{
    // get grid resolution:
    int res(0);
    do
    {
        std::cout << "Enter grid resolution (between 10 and 500, default 128): ";
        std::cin >> res;
    } while ((res < 10) || (res > 500));

    // check for existing output paths, create if necessary
    if (!fs::exists("simulation data")) fs::create_directory("simulation data");
    if (!fs::exists("simulation data/flip02_surface")) fs::create_directory("simulation data/flip02_surface");

    int  dimension  = 3;
    auto resolution = Manta::Vec3i(res);
    if (dimension == 2) resolution.z = 1;

    auto main_solver = Manta::FluidSolver(resolution, dimension);
    main_solver.setTimeStep(0.5f);
    Real minParticles = pow(2, dimension);

    // size of particles
    Real radiusFactor = 1.0f;

    // solver grids
    auto flags      = Manta::FlagGrid(&main_solver);
    auto phi        = Manta::LevelsetGrid(&main_solver);

    auto vel        = Manta::MACGrid(&main_solver);
    auto vel_old    = Manta::MACGrid(&main_solver);
    auto pressure   = Manta::Grid<Real>(&main_solver);
    auto tmp_vec3   = Manta::Grid<Manta::Vec3>(&main_solver);
    auto tstGrid    = Manta::Grid<Real>(&main_solver);

    // particles
    auto pp         = Manta::BasicParticleSystem(&main_solver);
    auto pVel       = Manta::PdataVec3(&pp);
    auto pTest      = Manta::PdataReal(&pp);    // test real value, not necessary for simulation
    auto mesh       = Manta::Mesh(&main_solver);

    // acceleration data for particle nbs
    auto pindex     = Manta::ParticleIndexSystem(&main_solver);
    auto gpi        = Manta::Grid<int>(&main_solver);

    // scene setup, 0 = breaking dam, 1 = falling drop
    int setup = 0;
    int boundaryWidth = 1;
    flags.initDomain(boundaryWidth);
    auto fluidVel = Manta::Sphere(&main_solver, Manta::Vec3(0.0f), 1.0f);
    Manta::Vec3 fluidSetVel;

    if (setup == 0)
    {
        // breakin dam
        auto fluidbox   = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                                   Manta::Vec3(0.0f, 0.0f, 0.0f),
                                                   Manta::Vec3(resolution.x * 0.4f, resolution.y * 0.6f, resolution.z * 0.1f));
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
    Manta::sampleLevelsetWithParticles(phi, flags, pp, 2, 0.05f);

    if (setup == 1)
    {
        fluidVel.applyToGrid(&vel, fluidSetVel);
        Manta::mapGridToPartsVec3(vel, pp, pVel);
    }

    // testing the real channel while resampling - original particles
    // will have a value of 0.1, new particle will get a value from the tstGrid
    Manta::testInitGridWithPos(tstGrid);
    pTest.setConst(0.1f);

    for (int t = 0; t < 250; t++)
    {
        // FLIP
        pp.advectInGrid(flags, vel, Manta::IntegrationMode::IntRK4, false);

        // make sure we have velocities throught liquid region
        Manta::mapPartsToMAC(flags, vel, vel_old, pp, pVel, &tmp_vec3);
        Manta::extrapolateMACFromWeight(vel, tmp_vec3, 2);
        Manta::markFluidCells(pp, flags);

        // create approximate surface level set, resample particles
        Manta::gridParticleIndex(pp, pindex, flags, gpi);
        Manta::unionParticleLevelset(pp, pindex, flags, gpi, phi, radiusFactor);
        Manta::resetOutflow(flags, nullptr, &pp, nullptr, &gpi,&pindex);

	    // extend levelset somewhat, needed by particle resampling in adjustNumber
	    Manta::extrapolateLsSimple(phi, 4, true);

        // forces and pressure solve
        Manta::addGravity(flags, vel, Manta::Vec3(0.0f, -0.001f, 0.0f));
        Manta::setWallBcs(flags, vel);
        Manta::solvePressure(vel, pressure, flags, 1e-3, &phi);
        Manta::setWallBcs(flags, vel);

        // set source grids for resampling, used in adjustNumber!
        pVel.setSource(&vel, true);
        pTest.setSource(&tstGrid);
        Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor);

        // make sure we have proper velocities
        Manta::extrapolateMACSimple(flags, vel);
        Manta::flipVelocityUpdate(flags, vel, vel_old, pp, pVel, 0.97f);

        if (dimension == 3) phi.createMesh(mesh);

        main_solver.step();

    	// generate data directly and for flip03_gen.exe surface generation scene
        std::stringstream filename1, filename2;
        filename1 << "simulation data\\flip02_surface\\fluidsurface_final_" << std::setw(4) << std::setfill('0') << t << ".bobj.gz";
        filename2 << "simulation data\\flip02_surface\\flipParts_"          << std::setw(4) << std::setfill('0') << t << ".uni";
        mesh.save(filename1.str());
        pp.save(filename2.str());
    }
}
