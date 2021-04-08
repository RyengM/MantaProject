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

// This FLIP example combines narrow band flip, 2nd order wall boundary conditions, and adaptive time stepping.
int main()
{
    // get grid resolution:
    int res(0);
    do
    {
        std::cout << "Enter grid resolution (between 10 and 500, default 64): ";
        std::cin >> res;
    } while ((res < 10) || (res > 500));

    // check for existing output paths, create if necessary
    if (!fs::exists("simulation data")) fs::create_directory("simulation data");
    if (!fs::exists("simulation data/flip06_obstacle")) fs::create_directory("simulation data/flip06_obstacle");

    int  dimension  = 3;
    auto resolution = Manta::Vec3i(res);
    if (dimension == 2) resolution.z = 1;
    auto main_solver = Manta::FluidSolver(resolution, dimension);

    int narrowBand(3); // narrow band width in cells(= R in[1])
    Real minParticles = pow(2, dimension);
    Real radiusFactor = 1.0f;
    int frames(200);
    auto gravity = Manta::Vec3(0.0f, -0.001f, 0.0f);

    // adaptive time stepping
    main_solver.mFrameLength    = 0.8f;     // length of one frame (in "world time")
    main_solver.mCflCond        = 3.0f;     // maximal velocity per cell and timestep, 3 is fairly strict
    main_solver.setTimeStep(0.8f);
    main_solver.mDtMin          = 0.2f;     // time step range
    main_solver.mDtMax          = 3.2f;

    // prepare grids and particles
    auto flags      = Manta::FlagGrid(&main_solver);
    auto phi        = Manta::LevelsetGrid(&main_solver);
    auto phiParts   = Manta::LevelsetGrid(&main_solver);
    auto phiObs     = Manta::LevelsetGrid(&main_solver);

    auto vel        = Manta::MACGrid(&main_solver);
    auto velOld     = Manta::MACGrid(&main_solver);
    auto velParts   = Manta::MACGrid(&main_solver);
    //auto mapWeights = Manta::MACGrid(&main_solver);

    auto pressure   = Manta::Grid<Real>(&main_solver);
    auto fractions  = Manta::MACGrid(&main_solver);
    auto tmpVec3    = Manta::Grid<Manta::Vec3>(&main_solver);

    auto pp         = Manta::BasicParticleSystem(&main_solver);
    auto pVel       = Manta::PdataVec3(&pp);
    auto mesh       = Manta::Mesh(&main_solver);

    // acceleration data for particle nbs
    auto pindex     = Manta::ParticleIndexSystem(&main_solver);
    auto gpi        = Manta::Grid<int>(&main_solver);

    // scene setup
    int bWidth(1);
    flags.initDomain(bWidth, "xXyYzZ", "      ", "      ", "      ", &phiObs);
    int fluidVel(0);
    int fluidSetVel(0);
    phi.setConst(999.0f);

    // standing dam
    auto fluidBox1 = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                               Manta::Vec3(0.0f, 0.0f, 0.0f),
                               Manta::Vec3(resolution.x * 1.0f, resolution.y * 0.3f, resolution.z * 1.0f));
    phi.join(fluidBox1.computeLevelset());
    auto fluidBox2 = Manta::Box(&main_solver, Manta::Vec3::Invalid,
                               Manta::Vec3(resolution.x * 0.1f, 0.0f, 0.0f),
                               Manta::Vec3(resolution.x * 0.2f, resolution.y * 0.75f, resolution.z * 1.0f));
    phi.join(fluidBox2.computeLevelset());

    // sphere obstacle
    auto sphere = Manta::Sphere(&main_solver, Manta::Vec3(resolution.x * 0.66f, resolution.y * 0.3f, resolution.z * 0.5f), res * 0.2f);
    phiObs.join(sphere.computeLevelset());

    flags.updateFromLevelset(phi);
    phi.subtract(phiObs);
    Manta::sampleLevelsetWithParticles(phi, flags, pp, 2, 0.1f);

    if (fluidVel != 0)
    {
        // set initial velocity
        // TODO
    }

    // also sets boundary flags for phiObs
    updateFractions(flags, phiObs, fractions, bWidth);
    setObstacleFlags(flags, fractions, phiObs);

    int lastFrame(-1);

    while (main_solver.getFrame() < frames)
    {
        auto maxVel = vel.getMaxValue();
        main_solver.adaptTimestep(maxVel);

        // advect particles and grid phi
        // note: Grid velocities are extrapolated at the end of each step
        pp.advectInGrid(flags, vel, Manta::IntegrationMode::IntRK4, false, false);
        Manta::pushOutofObs(pp, flags, phiObs);

        Manta::advectSemiLagrange(&flags, &vel, &phi, 1);
        Manta::advectSemiLagrange(&flags, &vel, &vel, 2);

        // create level set of particles
        Manta::gridParticleIndex(pp, pindex, flags, gpi);
        Manta::unionParticleLevelset(pp, pindex, flags, gpi, phiParts, radiusFactor);

        // combine level set of particles with grid level set
        phi.addConst(1.0);   // shrink slightly
        phi.join(phiParts);
        extrapolateLsSimple(phi, narrowBand + 2, true);
        extrapolateLsSimple(phi, 3);
        phi.setBoundNeumann(1); // make sure no particles are placed at outer boundary
        flags.updateFromLevelset(phi);

        // combine particles velocities with advected grid velocities
        mapPartsToMAC(flags, velParts, velOld, pp, pVel, &tmpVec3);
        extrapolateMACFromWeight(velParts, tmpVec3, 2);
        combineGridVel(velParts, tmpVec3, vel, &phi, narrowBand - 1);
        velOld.copyFrom(vel);

        // forces and pressure solve
        Manta::addGravity(flags, vel, gravity);
        extrapolateMACSimple(flags, vel, 2, nullptr, true);
        Manta::setWallBcs(flags, vel, &fractions, &phiObs);
        Manta::solvePressure(vel, pressure, flags, 1e-3, &phi, nullptr, &fractions);
        extrapolateMACSimple(flags, vel, 4, nullptr, true);
        Manta::setWallBcs(flags, vel, &fractions, &phiObs);

        if (dimension == 3)
        {
            // mis-use phiParts as temp grid to close the mesh
            phiParts.copyFrom(phi);
            phiParts.setBound(0.5, 0);
            phiParts.createMesh(mesh);
        }

        // set source grids for resampling, used in adjustNumber!
        pVel.setSource(&vel, true);
        Manta::adjustNumber(pp, vel, flags, 1 * minParticles, 2 * minParticles, phi, radiusFactor, narrowBand, &phiObs);
        Manta::flipVelocityUpdate(flags, vel, velOld, pp, pVel, 0.97f);

        main_solver.step();

        // generate data directly and for flip03_gen.exe surface generation scene
        if (lastFrame != main_solver.getFrame())
        {
            std::stringstream filename1, filename2;
            filename1 << "simulation data\\flip06_obstacle\\fluidsurface_final_" << std::setw(4) << std::setfill('0') << main_solver.getFrame() << ".bobj.gz";
            filename2 << "simulation data\\flip06_obstacle\\flipParts_"          << std::setw(4) << std::setfill('0') << main_solver.getFrame() << ".uni";
            mesh.save(filename1.str());
            pp.save(filename2.str());
        }

        lastFrame = main_solver.getFrame();
    }
}
