
#include "stdafx.h"
#include "tms.h"
#include "truck.h"

using namespace graphic;
using namespace tms;


cTms::cTms()
	: m_truckAllocTime((float)TRUCK_ALLOC_TIME)
{
}

cTms::~cTms()
{
	Clear();
}


bool cTms::Create(graphic::cRenderer &renderer)
{
	m_pathFinder.Create(256);
	m_lineList.Create(renderer, 1024*2);

	if (m_pathFinder.Read("../Media/vehiclepath3.txt"))
		UpdateLineList(renderer);

	TruckAllocate();

	return true;
}


void cTms::Update(graphic::cRenderer &renderer, const float deltaSeconds)
{
	__super::Update(renderer, deltaSeconds);

	for (auto &p : m_trucks)
		p->Update(renderer, deltaSeconds);

	// Truck allocation
	m_truckAllocTime -= deltaSeconds;
	if (m_truckAllocTime < 0)
	{
		TruckAllocate();
		m_truckAllocTime = (float)TRUCK_ALLOC_TIME;
	}
}


void cTms::Render(graphic::cRenderer &renderer)
{
	for (auto &p : m_trucks)
		p->Render(renderer);

	// Debugging
	// Render Path
	cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
	if (pathEdit.m_isShowWorldPath)
	{
		renderer.m_dbgBox.SetColor(cColor::WHITE);
		renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(1.f, 1.f, 1.f, 1.f);

		for (auto &vtx : m_pathFinder.m_vertices)
		{
			const cBoundingBox bbox(vtx.pos + Vector3(0, 1, 0), Vector3(1,1,1)*0.2f, Quaternion());
			renderer.m_dbgBox.m_color = cColor::BLACK;
			renderer.m_dbgBox.SetBox(bbox);
			renderer.m_dbgBox.Render(renderer);

			if ((1 <= vtx.type) && (vtx.edge[0].to >= 0))
			{
				ai::cPathFinder::sVertex &to = m_pathFinder.m_vertices[vtx.edge[0].to];
				const Vector3 dir = (to.pos - vtx.pos).Normal();

				renderer.m_dbgArrow.SetDirection(vtx.pos + Vector3(0, 1, 0)
					, vtx.pos + dir + Vector3(0, 1, 0)
					, 0.1f
				);
				renderer.m_dbgArrow.Render(renderer);
			}
		}

		m_lineList.Render(renderer);
	}

}


void cTms::ProcJobEtc(const sJob &job)
{
	switch (job.type)
	{
	case sJob::TRUCK_LOAD:
	{
		graphic::cRenderer &renderer = *framework::GetMainRenderer();
		if (cTruck *truck = GetFreeTruck(renderer))
			truck->AssignJob(job);
	}
	break;

	case sJob::TRUCK_LOAD_END:
	{
		vector<ai::cPathFinder::sVertex*> targets;
		for (auto &vtx : m_pathFinder.m_vertices)
			if (1 == vtx.type)
				targets.push_back(&vtx);

		sJob newJob;
		newJob.type = sJob::TRUCK_UNLOAD;
		newJob.truck.dest = targets[common::randint(0, targets.size() - 1)]->pos;
		job.truck.truck->AssignJob(newJob);
	}
	break;

	case sJob::TRUCK_UNLOAD:
	{
		//graphic::cRenderer &renderer = *framework::GetMainRenderer();
		//if (cTruck *truck = GetFreeTruck(renderer))
		//	truck->AssignJob(job);
	}
	break;

	case sJob::TRUCK_UNLOAD_END:
	{
		Vector3 home(541, 0, 466);
		home.y = g_root.m_terrain.GetHeight(home.x, home.z);

		sJob newJob;
		newJob.type = sJob::TRUCK_GOHOME;
		newJob.truck.dest = home;
		job.truck.truck->AssignJob(newJob);
	}
	break;

	case sJob::TRUCK_GOHOME_END:
		TruckAllocate();
		break;

	default: assert(0); break;
	}
}


void cTms::UpdateLineList(cRenderer &renderer)
{
	m_lineList.ClearLines();
	for (auto &vtx : m_pathFinder.m_vertices)
	{
		for (int i = 0; i < ai::cPathFinder::sVertex::MAX_EDGE; ++i)
		{
			if (vtx.edge[i].to < 0)
				break;
			m_lineList.AddLine(renderer, vtx.pos + Vector3(0, 1, 0)
				, m_pathFinder.m_vertices[vtx.edge[i].to].pos + Vector3(0, 1, 0));
		}
	}
}


bool cTms::WritePath()
{
	return m_pathFinder.Write("vehiclepath3.txt");
}


cTruck* cTms::GetFreeTruck(graphic::cRenderer &renderer)
{
	cTruck *truck = NULL;
	for (auto &p : m_trucks)
	{
		if (cTruck::eState::WAIT == p->m_state)
		{
			truck = p;
			break;
		}
	}

	if (!truck && m_trucks.size() < MAX_TRUCK)
	{
		truck = new cTruck();
		truck->Create(renderer, this);
		m_trucks.push_back(truck);
	}
	else
	{
		// All Truck Is Busy
	}

	return truck;
}


void cTms::TruckAllocate()
{
	vector<ai::cPathFinder::sVertex*> targets;
	for (auto &vtx : m_pathFinder.m_vertices)
		if (1 == vtx.type)
			targets.push_back(&vtx);

	if (!targets.empty())
	{
		Vector3 home(541, 0, 466);
		home.y = g_root.m_terrain.GetHeight(home.x, home.z);
		Vector3 dest(349, 0, 448);
		dest.y = g_root.m_terrain.GetHeight(dest.x, dest.z);

		sJob job;
		job.type = sJob::TRUCK_LOAD;
		job.truck.home = home;
		job.truck.dest = dest;
		job.truck.returnHome = home;
		AddJobs(job);
	}
}


void cTms::Clear()
{
	for (auto &p : m_trucks)
		delete p;
	m_trucks.clear();
}
