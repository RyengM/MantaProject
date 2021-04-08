#pragma once

#include <queue>
#include <mutex>

class SmokeSolver
{
public:
	static void Run(float* outDensity, std::mutex& mutex, int& bExit);
};