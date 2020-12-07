
#include "stdafx.h"
#include "vehiclepath.h"

using namespace graphic;
using namespace tms;


cVehiclePath::cVehiclePath()
	: m_idx(0)
	, m_velocity(10)
	, m_incTime(0)
	, m_initialDirection(0,0,1)
	, m_rotTime(0.333f)
{
	m_path.reserve(32);
}

cVehiclePath::~cVehiclePath()
{
}


bool cVehiclePath::SetPath(const vector<Vector3> &path, const Vector3 &direction)
{
	m_initialDirection = direction;
	m_path.resize(path.size());
	for (uint i=0; i < path.size(); ++i)
		m_path[i] = path[i];

	if (!m_path.empty())
	{
		m_idx = -1;
		NextMove();
	}

	return true;
}


// If Finish Path, return true
bool cVehiclePath::Update(const float deltaSeconds)
{
	if (m_idx >= (int)(m_path.size() - 1))
	{
		m_idx = -1;
		//NextMove();
		return true;
	}

	m_incTime += deltaSeconds;

	if (m_elapsedTime < m_incTime)
	{
		NextMove();
	}
	else
	{
		common::lerp(m_pos, m_path[m_idx], m_path[m_idx + 1], m_incTime / m_elapsedTime);

		if (m_incTime < m_rotTime)
			m_rot = m_rot.Interpolate(m_nextRot, m_incTime / m_rotTime);
		else
			m_rot = m_nextRot;
	
		if (m_pos.LengthRoughly(m_path[m_idx + 1]) < 0.01f)
			NextMove();
	}

	return false;
}


bool cVehiclePath::IsEmpty()
{
	return m_path.empty();
}


void cVehiclePath::NextMove()
{
	++m_idx;
	m_incTime = 0;
	m_pos = m_path[m_idx];

	if ((int)m_path.size() > (m_idx+1))
	{
		Vector3 dir = m_path[m_idx+1] - m_path[m_idx];
		m_elapsedTime = dir.Length() / m_velocity;
		dir.Normalize();
		if (m_elapsedTime > 0)
		{
			// initial direction is Vector3(1,0,0)
			// but Camera initial direction is Vector3(0,0,1)
			// so rotation y 
			Matrix44 rot;
			rot.SetRotationY(-MATH_PI / 2.f);
			Vector3 newDir = dir * rot;

			Matrix44 mDir;
			mDir.SetView(Vector3(0, 0, 0), newDir, Vector3(0, 1, 0));
			mDir.Inverse2();
			m_nextRot = mDir.GetQuaternion();
			//m_nextRot.SetRotationArc(m_initialDirection, dir, Vector3(0, 1, 0));
		}
	}
}
