
#include "stdafx.h"
#include "vehiclemanager.h"
//#include "forklift.h"


using namespace graphic;
using namespace tms;


cVehicleManager::cVehicleManager()
	: m_floor(NULL)
{
}

cVehicleManager::~cVehicleManager()
{
}


bool cVehicleManager::Create(const cRenderer &renderer, cFloor *floor)
{
	m_floor = floor;

	return true;
}


bool cVehicleManager::AddJobs(const sJob &job)
{
	cForkLift *forkLift = GetFreeForkLift();
	RETV(!forkLift, false);

	switch (job.type)
	{
	case sJob::FORKLIFT_LOAD:
		if (job.forkLift.load)
		{
			forkLift->LoadPalete(*job.forkLift.load, job.forkLift.dest);
		}
		else
		{
			forkLift->NextRandomMove();
		}
		break;

	case sJob::FORKLIFT_UNLOAD:
		break;
	}

	return true;
}


cForkLift* cVehicleManager::GetFreeForkLift()
{
	RETV(!m_floor, NULL);

	if (m_forkLifts.empty())
	{
		cRenderer &renderer = *framework::GetMainRenderer();

		cForkLift *forkLift = new cForkLift();
		forkLift->Create(renderer, m_floor, m_floor->m_pathFinder
			, Vector3(55, -0.5f, 20), m_floor->m_transform.rot);
		m_floor->AddChild(forkLift);

		m_forkLifts.push_back(forkLift);
	}

	return m_forkLifts[0];
}
