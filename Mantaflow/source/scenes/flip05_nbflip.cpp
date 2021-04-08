#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

#include "manta.h"
#include "advection.h"
#include "grid.h"
#include "levelset.h"
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
    // toggle between regular FLIP and NB-FLIP
    auto narrowBand = true;

    // nb configuration
    auto narrowBandWidth  = 3;                      // narrow band width in cells(= R in[1])
    auto combineBandWidth = narrowBandWidth - 1;    // combine band width in cells(= r in[1])
    
    // get grid resolution:
    int res(0);
    do
    {
        std::cout << "Enter grid resolution (between 10 and 500, default 64): ";
        std::cin >> res;
    } while ((res < 10) || (res > 500));

    // check for existing output paths, create if necessary
    if (!fs::exists("simulation data")) fs::create_directory("simulation data");
    if (!fs::exists("simulation data/flip05_nbflip")) fs::create_directory("simulation data/flip05_nbflip");

    int  dimension  = 3;
    auto resolution = Manta::Vec3i(res);
    if (dimension == 2) resolution.z = 1;

    auto main_solver = Manta::FluidSolver(resolution, dimension);

    // adaptive time stepping
    main_solver.mFrameLength    = 1.0f;     // length of one frame (in "world time")
    main_solver.setTimeStep(1.0f);
    main_solver.mDtMin          = 0.5f;     // time step range
    main_solver.mDtMax          = 1.0f;
    main_solver.mCflCond        = 5.0f;     // maximal velocity per cell

    auto gravity = Manta::Vec3(0.0f, -0.003f, 0.0f);

    Real minParticles = pow(2, dimension);
    Real radiusFactor = 1.0f;

    // prepare grids and particles
    auto flags      = Manta::FlagGrid(&main_solver);

    auto phiParts   = Manta::LevelsetGrid(&main_solver);
    auto phi        = Manta::LevelsetGrid(&main_solver);
    auto pressure   = Manta::Grid<Real>(&main_solver);

    auto vel        = Manta::MACGrid(&main_solver);
    auto velOld     = Manta::MACGrid(&main_solver);
    auto velParts   = Manta::MACGrid(&main_solver);
    auto mapWeights = Manta::MACGrid(&main_solver);

    auto pp         = Manta::BasicParticleSystem(&main_solver);
    auto pVel       = Manta::PdataVec3(&pp);
    auto mesh       = Manta::Mesh(&main_solver);

    // acceleration data for particle nbs
    auto pindex     = Manta::ParticleIndexSystem(&main_solver);
    auto gpi        = Manta::Grid<int>(&main_solver);

    // geometry in world units (to be converted to grid space upon init)
    flags.initDomain();
    phi.initFromFlags(flags);

    // a simple breaking dam
    auto fluidBasin = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                 Manta::Vec3(0.0f, 0.0f, 0.0f),
                                 Manta::Vec3(resolution.x * 1.0f, resolution.y * 0.15f, resolution.z * 1.0f));
    phi.join(fluidBasin.computeLevelset());

    auto fluidDam  = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                                Manta::Vec3(resolution.x * 0.0f, resolution.y * 0.15f, 0.0f),
                                Manta::Vec3(resolution.x * 0.4f, resolution.y * 0.5f, resolution.z * 0.8f));
    phi.join(fluidDam.computeLevelset());

    flags.updateFromLevelset(phi);
    Manta::sampleLevelsetWithParticles(phi, flags, pp, 2, 0.1f);
    Manta::mapGridToPartsVec3(vel, pp, pVel);

    int step(-1);

    while (main_solver.getFrame() < 200)
    {
        step += 1;

        auto maxVel = vel.getMaxValue();
        main_solver.adaptTimestep(maxVel);

        // advect particles and grid phi
        // note: Grid velocities are extrapolated at the end of each step
        pp.advectInGrid(flags, vel, Manta::IntegrationMode::IntRK4, false, false);
        Manta::advectSemiLagrange(&flags, &vel, &phi);
        flags.updateFromLevelset(phi);

        // advect grid velocity
        if (narrowBand) Manta::advectSemiLagrange(&flags, &vel, &vel, 2);

        // create level set of particles
        Manta::gridParticleIndex(pp, pindex, flags, gpi);
        Manta::unionParticleLevelset(pp, pindex, flags, gpi, phiParts, radiusFactor);

        if (narrowBand)
        {
            // combine level set of particles with grid level set
            phi.addConst(1.0);   // shrink slightly
            phi.join(phiParts);
            extrapolateLsSimple(phi, narrowBandWidth + 2, true);
        } else
        {
            // overwrite grid level set with level set of particles
            phi.copyFrom(phiParts);
            extrapolateLsSimple(phi, 4, true);
        }

        extrapolateLsSimple(phi, 3);
        flags.updateFromLevelset(phi);
            
        // make sure we have velocities throughout liquid region
        if (narrowBand)
        {
            // combine particles velocities with advected grid velocities
            mapPartsToMAC(flags, velParts, velOld, pp, pVel, &mapWeights);
            extrapolateMACFromWeight(velParts, mapWeights, 2);
            combineGridVel(velParts, mapWeights, vel, &phi, combineBandWidth);
            velOld.copyFrom(vel);
        } else
        {
            // map particle velocities to grid
            mapPartsToMAC(flags, vel, velOld, pp, pVel, &mapWeights);
            extrapolateMACFromWeight(vel, mapWeights, 2);
        }

        // forces and pressure solve
        Manta::addGravity(flags, vel, gravity);
        Manta::setWallBcs(flags, vel);
        Manta::solvePressure(vel, pressure, flags, 1e-3, &phi);
        Manta::setWallBcs(flags, vel);

        // extrapolate and update particle velocities
        extrapolateMACSimple(flags, vel, int(maxVel * 1.25 + 2.0));
        Manta::flipVelocityUpdate(flags, vel, velOld, pp, pVel, 0.95f);

        if (dimension == 3) phi.createMesh(mesh);

        // resample particles
        pVel.setSource(&vel, true);
        if (narrowBand)
        {
            Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor, narrowBandWidth);
        } else
        {
            Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor);
        }

        main_solver.step();

    	// generate data directly and for flip03_gen.exe surface generation scene
        std::stringstream filename1, filename2;
        filename1 << "simulation data\\flip05_nbflip\\fluidsurface_final_" << std::setw(4) << std::setfill('0') << step << ".bobj.gz";
        filename2 << "simulation data\\flip05_nbflip\\flipParts_"          << std::setw(4) << std::setfill('0') << step << ".uni";
        mesh.save(filename1.str());
        pp.save(filename2.str());
    }
}
