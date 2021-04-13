#include "SmokeSolver.h"
#include "manta.h"
#include "grid.h"
#include "noisefield.h"
#include "plugin/extforces.h"
#include "plugin/pressure.h"
#include "plugin/initplugins.h"
#include "plugin/advection.h"

using namespace Manta;

void SmokeSolver::Run(float*& outDensity, std::mutex& mutex, int& bExit)
{
	std::cout << "hello world" << std::endl;
	int res = 32;
	Vec3i gs = Vec3i(res, 4 * res, res);
	auto solver = FluidSolver(gs);
	auto flags = FlagGrid(&solver);
	auto vel = MACGrid(&solver);
	auto density = Grid<Real>(&solver);
	auto pressure = Grid<Real>(&solver);

	int bWidth = 1;
	flags.initDomain(bWidth);
	flags.fillGrid();

	Vec3 gs_f = Vec3(res, res, res);
	auto source = Cylinder(&solver, gs_f * Vec3(0.5, 0.1, 0.5), res * 0.14, gs_f * Vec3(0, 0.02, 0));

	setOpenBound(flags, bWidth, "xyzXYZ", FlagGrid::TypeEmpty);

	while (!bExit)
	{
		std::cout << "frame " << solver.mFrame << std::endl;
		source.applyToGrid(&density, 1.f);
		advectSemiLagrange(&flags, &vel, &density, 2);
		advectSemiLagrange(&flags, &vel, &vel, 2);

		setWallBcs(flags, vel);
		addBuoyancy(flags, density, vel, Vec3(0, -4e-3, 0));

		solvePressure(vel, pressure, flags);
		solver.step();

		mutex.lock();
		memcpy(outDensity, density.getData(), gs.x * gs.y * gs.z * sizeof(float));
		mutex.unlock();
	}
}