
#include "stdafx.h"
#include "truck.h"
#include "tms.h"


using namespace graphic;
using namespace tms;


cTruck::cTruck()
	: m_state(eState::WAIT)
	, m_delayTime(0)
{
	m_vpath.m_velocity = 60;
}

cTruck::~cTruck()
{
}


bool cTruck::Create(graphic::cRenderer &renderer, cTms *tms
	, const Transform &transform //= Transform::Identity
)
{
	__super::Create(renderer, common::GenerateId(), "../Media/truck.dae", true);

	m_tms = tms;
	m_transform = transform;

	m_transform.scale = Vector3(1, 1, 1) * 0.3f;

	m_lineList.Create(renderer, 128, cColor::RED);
	return true;
}


bool cTruck::Render(cRenderer &renderer
	, const XMMATRIX &parentTm //= XMIdentity
	, const int flags //= 1
)
{
	//if (g_root.m_dbgWindow->m_isShowVehiclePath)
	//	m_lineList.Render(renderer, tm);

	return __super::Render(renderer, parentTm, flags);
}


bool cTruck::Update(graphic::cRenderer &renderer, const float deltaSeconds)
{
	__super::Update(renderer, deltaSeconds);

	switch (m_state)
	{
	case eState::WAIT:
		break;

	case eState::MOVE:
		if (m_vpath.IsEmpty() || m_vpath.Update(deltaSeconds))
		{
			m_state = eState::REACH;
		}
		else
		{
			m_transform.pos = m_vpath.m_pos + Vector3(0, -0.5f, 0);
			m_transform.rot = m_vpath.m_rot;
		}
		break;

	case eState::REACH:
	{
		const Vector3 dir = Vector3(0,0,-1) * m_vpath.m_nextRot.GetMatrix();
		m_transform.rot.SetRotationArc(Vector3(1, 0, 0), -dir);

		m_delayTime = 10.f; // 10 seconds
		m_state = eState::WORK;
	}
	break;

	case eState::WORK:
	{
		m_delayTime -= deltaSeconds;
		if (m_delayTime < 0)
		{
			if (!m_tms)
				break;

			switch (m_job.type)
			{
			case sJob::TRUCK_LOAD:
			{
				sJob job;
				job.type = sJob::TRUCK_LOAD_END;
				job.truck.truck = this;
				m_tms->AddJobs(job);
			}
			break;

			case sJob::TRUCK_UNLOAD:
			{
				sJob job;
				job.type = sJob::TRUCK_UNLOAD_END;
				job.truck.truck = this;
				m_tms->AddJobs(job);

				//Move(m_job.truck.returnHome);
				//m_state = eState::RETURN;
			}
			break;

			case sJob::TRUCK_GOHOME:
			{
				sJob job;
				job.type = sJob::TRUCK_GOHOME_END;
				job.truck.truck = this;
				m_tms->AddJobs(job);

				m_state = eState::WAIT;
			}
			break;

			default: assert(0); break;
			}
		}
	}
	break;

	//case eState::RETURN:
	//	if (m_vpath.IsEmpty() || m_vpath.Update(deltaSeconds))
	//	{
	//		m_state = eState::FINISH;
	//	}
	//	else
	//	{
	//		m_transform.pos = m_vpath.m_pos + Vector3(0, -0.5f, 0);
	//		m_transform.rot = m_vpath.m_rot;
	//	}
	//	break;

	//case eState::FINISH:
	//{
	//	if (!m_tms)
	//		break;

	//	sJob job;
	//	job.type = sJob::TRUCK_GOHOME_END;
	//	job.truck.truck = this;
	//	m_tms->AddJobs(job);
	//	m_state = eState::WAIT;
	//}
	//break;
	}

	return true;
}


void cTruck::NextRandomMove()
{
	if (!m_tms)
		return;

	vector<ai::sVertex*> targets;
	for (auto &vtx : m_tms->m_pathFinder.m_vertices)
		if (1 == vtx.type)
			targets.push_back(&vtx);

	vector<Vector3> path;
	if (m_tms->m_pathFinder.Find(
		Vector3(1060, 0, 1900)
		, targets[common::randint(0, targets.size() - 1)]->pos
		, path))
	{
		//m_vpath.SetPath(path, Vector3(0, 0, -1));
		m_vpath.SetPath(path, Vector3(1, 0, 0));

		if (g_root.m_dbgWindow->m_isShowVehiclePath)
		{
			m_lineList.ClearLines();
			//for (auto &pos : path)
			//	m_lineList.AddNextPoint(pos);
		}
	}
}


void cTruck::Move(const Vector3 &dest)
{
	if (!m_tms)
		return;

	vector<Vector3> path;
	if (m_tms->m_pathFinder.Find(m_transform.pos, dest, path))
	{
		//m_vpath.SetPath(path, Vector3(0, 0, -1));
		m_vpath.SetPath(path, Vector3(1, 0, 0));

		if (g_root.m_dbgWindow->m_isShowVehiclePath)
		{
			m_lineList.ClearLines();
			//for (auto &pos : path)
			//	m_lineList.AddNextPoint(pos);
		}

		m_state = eState::MOVE;
	}
}


void cTruck::AssignJob(const sJob &job)
{
	switch (job.type)
	{
	case sJob::TRUCK_LOAD:
	{
		m_transform.pos = job.truck.home;
		Move(job.truck.dest);
	}
	break;

	case sJob::TRUCK_LOAD_END:
		break;

	case sJob::TRUCK_UNLOAD:
	{
		//m_transform.pos = job.truck.home;
		Move(job.truck.dest);
	}
	break;

	case sJob::TRUCK_UNLOAD_END:
		break;

	case sJob::TRUCK_GOHOME:
	{
		//m_transform.pos = job.truck.home;
		Move(job.truck.dest);
	}
	break;
	
	default: assert(0); break;
	}

	m_job = job;
}


void cTruck::InitModel(cRenderer &renderer)
{
	__super::InitModel(renderer);

}
