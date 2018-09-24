//
// 2017-06-09, jjuiddong
// Vehicle Path
//
#pragma once

#include "pathmesh.h"


namespace tms
{

	class cVehiclePath
	{
	public:
		cVehiclePath();
		virtual ~cVehiclePath();
		bool SetPath(const vector<Vector3> &path, const Vector3 &direction);
		bool Update(const float deltaSeconds);
		bool IsEmpty();
		

	protected:
		void NextMove();


	public:
		vector<Vector3> m_path;
		int m_idx;
		float m_velocity;
		float m_incTime;
		float m_elapsedTime;
		float m_rotTime; // 0.3 seconds

		Vector3 m_initialDirection;
		Vector3 m_pos;
		Quaternion m_rot;
		Quaternion m_nextRot;
	};

}
