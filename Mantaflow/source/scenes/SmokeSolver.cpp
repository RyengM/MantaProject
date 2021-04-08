#include "SmokeSolver.h"
#include "manta.h"
#include "grid.h"
#include "noisefield.h"
#include "plugin/extforces.h"
#include "plugin/pressure.h"
#include "plugin/initplugins.h"
#include "plugin/advection.h"

using namespace Manta;

void SmokeSolver::Run(float* outDensity, std::mutex& mutex, int& bExit)
{
	std::cout << "hello world" << std::endl;
	int res = 64;
	Vec3i gs = Vec3i(res, int(1.5 * res), res);
	auto solver = FluidSolver(gs);
	auto flags = FlagGrid(&solver);
	auto vel = MACGrid(&solver);
	auto density = Grid<Real>(&solver);
	auto pressure = Grid<Real>(&solver);

	auto noise = WaveletNoiseField(&solver);
	noise.mPosScale = Vec3(45);
	noise.mClamp = true;
	noise.mClampNeg = 0;
	noise.mClampPos = 1;
	noise.mValOffset = 0.75;
	noise.mTimeAnim = 0.2;

	Vec3 gs_f = Vec3(res, int(1.5 * res), res);
	auto source = Cylinder(&solver, gs_f * Vec3(0.5, 0.1, 0.5), res * 0.14, gs_f * Vec3(0, 0.02, 0));
	flags.initDomain();
	flags.fillGrid();

	while (!bExit)
	{
		std::cout << "frame " << solver.mFrame << std::endl;
		densityInflow(flags, density, noise, &source, 1, 0.5);
		advectSemiLagrange(&flags, &vel, &density, 2);
		advectSemiLagrange(&flags, &vel, &vel, 2, 1);

		setWallBcs(flags, vel);
		addBuoyancy(flags, density, vel, Vec3(0, -6e-4, 0));

		solvePressure(vel, pressure, flags);
		solver.step();

		mutex.lock();
		outDensity = density.getData();
		mutex.unlock();
	}
}