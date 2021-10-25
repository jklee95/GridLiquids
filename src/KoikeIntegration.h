#pragma once
#include "TimeIntegration.h"

class KoikeIntegration : public TimeIntegration
{
public:
	KoikeIntegration(float timeStep, GridData& index);
	~KoikeIntegration() override;

	float computeGridTimeStep(DirectX::XMFLOAT2 vel, int i, int j) override;
	float computeParticleTimeStep(DirectX::XMFLOAT2 vel, int i) override;
	void computeGlobalTimeStep(std::vector<DirectX::XMFLOAT2>& vel, std::vector<STATE>& state) override;

	void computeAdvectTimeStep(std::vector<float>& pCount, int i, int j) override;
	void reInterpTimeStep(DirectX::XMFLOAT2 ratio, DirectX::XMINT2 minIndex, DirectX::XMINT2 maxIndex, int i) override;

private:

};

