﻿#include "EulerianSimulation.h"

using namespace DirectX;
using namespace std;

EulerianSimulation::EulerianSimulation(float timeStep)
	:FluidSimulation::FluidSimulation(timeStep)
{
}

EulerianSimulation::~EulerianSimulation()
{

}

void EulerianSimulation::initialize()
{
	// 0 is not allowed.
	assert(_gridCount != 0);

	// Set _fluid
	int N = _gridCount - 2;
	for (int i = 0; i < _gridCount; i++)
	{
		for (int j = 0; j < _gridCount; j++)
		{
			if (i == 0 || j == 0
				|| i == N + 1
				|| j == N + 1)
			{
				_gridState.push_back(STATE::BOUNDARY);
			}
			else if ( ((N + 1) / 2 - 9 < i) 
				&& (i < (N + 1) / 2 + 9)
				&& ((N + 1) / 2 - 9 < j) 
				&& (j < (N + 1) / 2 + 9)
				)
			{
				_gridState.push_back(STATE::FLUID);
			}
			else
			{
				_gridState.push_back(STATE::AIR);
			}

			_gridVelocity.push_back(XMFLOAT2(0.0f, 0.01f));
			_gridDivergence.push_back(0.0f);
			_gridPressure.push_back(0.0f);
		}
	}

}


void EulerianSimulation::_setBoundary(std::vector<XMFLOAT2>& vec)
{
	int N = _gridCount - 2;

	// (x, 0) (x, yMax+1)
	for (int i = 1; i <= N; i++)
	{
		vec[_INDEX(i, 0)].x = +vec[_INDEX(i, 1)].x;
		vec[_INDEX(i, 0)].y = -vec[_INDEX(i, 1)].y;

		vec[_INDEX(i, N + 1)].x = +vec[_INDEX(i, N)].x;
		vec[_INDEX(i, N + 1)].y = -vec[_INDEX(i, N)].y;
	}

	// (0, y) (xMax+1, y)
	for (int j = 1; j <= N; j++)
	{
		vec[_INDEX(0, j)].x = -vec[_INDEX(1, j)].x;
		vec[_INDEX(0, j)].y = +vec[_INDEX(1, j)].y;

		vec[_INDEX(N + 1, j)].x = -vec[_INDEX(N, j)].x;
		vec[_INDEX(N + 1, j)].y = +vec[_INDEX(N, j)].y;
	}

	// (0, 0)
	vec[_INDEX(0, 0)].x = vec[_INDEX(0, 1)].x;
	vec[_INDEX(0, 0)].y = vec[_INDEX(1, 0)].y;
	// (0, yCount)
	vec[_INDEX(0, N + 1)].x = vec[_INDEX(0, N)].x;
	vec[_INDEX(0, N + 1)].y = vec[_INDEX(1, N + 1)].y ;
	// (xCount, 0)
	vec[_INDEX(N + 1, 0)].x = vec[_INDEX(N + 1, 1)].x;
	vec[_INDEX(N + 1, 0)].y = vec[_INDEX(N, 0)].y;
	// (xCount, yCount)
	vec[_INDEX(N + 1, N + 1)].x = vec[_INDEX(N + 1, N)].x;
	vec[_INDEX(N + 1, N + 1)].y = vec[_INDEX(N, N + 1)].y;
}

void EulerianSimulation::_setBoundary(std::vector<float>& scalar)
{
	int N = _gridCount - 2;

	// (x, 0) (x, yMax+1)
	for (int i = 1; i <= N; i++)
	{
		scalar[_INDEX(i, 0)] = scalar[_INDEX(i, 1)];
		scalar[_INDEX(i, N + 1)] = scalar[_INDEX(i, N)];
	}

	// (0, y) (xMax+1, y)
	for (int j = 1; j <= N; j++)
	{
		scalar[_INDEX(0, j)] = scalar[_INDEX(1, j)];
		scalar[_INDEX(N + 1, j)] = scalar[_INDEX(N, j)];
	}

	// (0, 0)
	scalar[_INDEX(0, 0)] = (scalar[_INDEX(0, 1)] + scalar[_INDEX(1, 0)]) / 2.0f;
	// (0, yCount)
	scalar[_INDEX(0, N + 1)] = (scalar[_INDEX(0, N)] + scalar[_INDEX(1, N + 1)]) / 2.0f;
	// (xCount, 0)
	scalar[_INDEX(N + 1, 0)] = (scalar[_INDEX(N + 1, 1)] + scalar[_INDEX(N, 0)]) / 2.0f;
	// (xCount, yCount)
	scalar[_INDEX(N + 1, N + 1)] = (scalar[_INDEX(N + 1, N)] + scalar[_INDEX(N, N + 1)]) / 2.0f;
}

void EulerianSimulation::_paintGrid()
{
	int N = _gridCount - 2;
	// Reset _grid
	for (int i = 1; i <= N; i++)
	{
		for (int j = 1; j <= N; j++)
		{
			_gridState[_INDEX(i, j)] = STATE::AIR;
		}
	}


	for (int i = 0; i < _particlePosition.size(); i++)
	{
		int minXIndex = _computeFaceMinMaxIndex(VALUE::MIN, AXIS::X, _particlePosition[i]);
		int minYIndex = _computeFaceMinMaxIndex(VALUE::MIN, AXIS::Y, _particlePosition[i]);
		int maxXIndex = _computeFaceMinMaxIndex(VALUE::MAX, AXIS::X, _particlePosition[i]);
		int maxYIndex = _computeFaceMinMaxIndex(VALUE::MAX, AXIS::Y, _particlePosition[i]);

		// Painting
		STATE& minMin = _gridState[_INDEX(minXIndex, minYIndex)];
		STATE& minMax = _gridState[_INDEX(minXIndex, maxYIndex)];
		STATE& maxMin = _gridState[_INDEX(maxXIndex, minYIndex)];
		STATE& maxMax = _gridState[_INDEX(maxXIndex, maxYIndex)];

		// Boundary Checking
		if (minMin != STATE::BOUNDARY) minMin = STATE::FLUID;
		if (minMax != STATE::BOUNDARY) minMax = STATE::FLUID;
		if (maxMin != STATE::BOUNDARY) maxMin = STATE::FLUID;
		if (maxMax != STATE::BOUNDARY) maxMax = STATE::FLUID;
	}
}


void EulerianSimulation::_updateParticlePosition()
{
	for (int i = 0; i < _particlePosition.size(); i++)
	{
		// 2. 3.
		_particleVelocity[i] = _velocityInterpolation(_particlePosition[i], _gridVelocity);


		_particlePosition[i].x += _particleVelocity[i].x * _timeStep * 10.0f;
		_particlePosition[i].y += _particleVelocity[i].y * _timeStep * 10.0f;
	}
}


// To calculate the grid index, the calculation result must not depend on the _gridScale.
// Therefore, the intermediate computed variable "should not be multiplied by the _gridScale".
// For example, if the scale is 1.0f, the result is (index * 1.0f).
// But if the scale is 0.5f, the result is (index * 0.5f).
// The index value should of course be immutable.
int EulerianSimulation::_computeFaceMinMaxIndex(VALUE vState, AXIS axis, XMFLOAT2 particlePos)
{
	float pos = (axis == AXIS::X) ? particlePos.x : particlePos.y;
	float value;

	// Compute position by normalizing from (-N, N) to (0, N + 1)
	switch (vState)
	{
	case VALUE::MIN:
		value = pos + 0.5f - 0.5f * _particleScale;
		break;
	case VALUE::MAX:
		value = pos + 0.5f + 0.5f * _particleScale;
		break;
	default:
		value = 0.0f;
		break;
	}

	// Compute Index
	return static_cast<int>(floor(value));
}


// Different from _computeFaceMinMaxIndex().
// 1. Subtract the count of offset by 1.
// 2. Do not subtract particleStride from min, max calculation.
// 3. ceil maxIndex instead of floor.
// ------------------------------------------------------------------
// _PaintGrid() uses the face as the transition point.
// _updateParticlePosition() uses the center as the transition point.
int EulerianSimulation::_computeCenterMinMaxIndex(VALUE vState, AXIS axis, XMFLOAT2 particlePos)
{
	float pos = (axis == AXIS::X) ? particlePos.x : particlePos.y;
	// 2.
	float value = pos;

	switch (vState)
	{
	case VALUE::MIN:
		return static_cast<int>(floor(value));
		break;
	case VALUE::MAX:
		// 3.
		return static_cast<int>(ceil(value));
		break;
	default:
		return -1;
		break;
	}
}

XMFLOAT2 EulerianSimulation::_velocityInterpolation(XMFLOAT2 pos, vector<XMFLOAT2> oldvel)
{
	// 2. 3.
	int minXIndex = _computeCenterMinMaxIndex(VALUE::MIN, AXIS::X, pos);
	int minYIndex = _computeCenterMinMaxIndex(VALUE::MIN, AXIS::Y, pos);
	int maxXIndex = _computeCenterMinMaxIndex(VALUE::MAX, AXIS::X, pos);
	int maxYIndex = _computeCenterMinMaxIndex(VALUE::MAX, AXIS::Y, pos);

	float xRatio = (pos.x - _gridPosition[_INDEX(minXIndex, minYIndex)].x);
	float yRatio = (pos.y - _gridPosition[_INDEX(minXIndex, minYIndex)].y);

	XMFLOAT2 minMinVelocity = oldvel[_INDEX(minXIndex, minYIndex)];
	XMFLOAT2 minMaxVelocity = oldvel[_INDEX(minXIndex, maxYIndex)];
	XMFLOAT2 maxMinVelocity = oldvel[_INDEX(maxXIndex, minYIndex)];
	XMFLOAT2 maxMaxVelocity = oldvel[_INDEX(maxXIndex, maxYIndex)];

	// s0* (t0 * d0[IX(i0, j0)] + t1 * d0[IX(i0, j1)]) +
	//	s1 * (t0 * d0[IX(i1, j0)] + t1 * d0[IX(i1, j1)]);
	// minMinVelocity.x and minMinVelocity.x ​​can be different because they are "velocity", not position.
	return XMFLOAT2(
		_interpolation(_interpolation(minMinVelocity.x, minMaxVelocity.x, yRatio), _interpolation(maxMinVelocity.x, maxMaxVelocity.x, yRatio), xRatio),
		_interpolation(_interpolation(minMinVelocity.y, minMaxVelocity.y, yRatio), _interpolation(maxMinVelocity.y, maxMaxVelocity.y, yRatio), xRatio)
	);
}

float EulerianSimulation::_interpolation(float value1, float value2, float ratio)
{
	return value1 * (1.0f - ratio) + value2 * ratio;
}