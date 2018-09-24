
#include "stdafx.h"
#include "jobmanager.h"
//#include "forklift.h"

using namespace graphic;
using namespace tms;


cJobManager::cJobManager()
	: m_floor(NULL)
{
}

cJobManager::~cJobManager()
{
}


bool cJobManager::Create(cFloor *floor)
{
	m_floor = floor;

	return true;
}


void cJobManager::Update(graphic::cRenderer &renderer, const float deltaSeconds)
{
	ProcessJobs();
}


void cJobManager::AddJobs(const sJob &job)
{
	m_jobQ.push(job);
}


void cJobManager::ProcessJobs()
{
	while (!m_jobQ.empty())
	{
		sJob job = m_jobQ.front(); m_jobQ.pop();

		switch (job.type)
		{
		case sJob::FORKLIFT_LOAD:
			//if (m_floor)
			//	m_floor->m_vehicleMgr.AddJobs(job);
			break;

		case sJob::FORKLIFT_UNLOAD:
		{
			//if (!m_floor)
			//	break;

			//const int paleteIdx = common::randint(0, m_floor->m_paletes.size() - 1);
			//cPalete *palete = m_floor->m_paletes[paleteIdx];
			//sJob newJob;
			//newJob.type = sJob::FORKLIFT_LOAD;
			//newJob.forkLift.load = palete;
			//newJob.forkLift.dest = Vector3(0, 0, 0);
			//AddJobs(newJob);

			//if (g_root.m_vehicleWindow->m_isShowVehiclePalete)
			//{
			//	if (job.forkLift.vehicle)
			//	{
			//		if (job.forkLift.vehicle->m_loadPalete)
			//		{
			//			job.forkLift.vehicle->m_loadPalete->m_box[0].m_info.color = D3DCOLOR_XRGB(255, 255, 255);
			//			job.forkLift.vehicle->m_loadPalete->m_racker->ChangeColor();
			//		}
			//	}

			//	palete->m_box[0].m_info.color = D3DCOLOR_XRGB(255, 0, 0);
			//	palete->m_racker->ChangeColor();
			//}
		}
		break;

		default: 
			ProcJobEtc(job);
			break;
		}
	}
}


void cJobManager::ProcJobEtc(const sJob &job)
{
	assert(0);
}
