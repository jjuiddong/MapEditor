//
// 2017-06-14, jjuiddong
// Vehicle Manager
//
#pragma once

#include "jobmanager.h"


namespace tms
{
	class cWarehouse2;
	//class cForkLift;
	//class cFloor;

	class cVehicleManager
	{
	public:
		cVehicleManager();
		virtual ~cVehicleManager();
		
		bool Create(const graphic::cRenderer &renderer, cFloor *floor);
		bool AddJobs(const sJob &job);
		

	protected:
		//cForkLift* GetFreeForkLift();


	public:
		//cFloor *m_floor; // reference
		//vector<cForkLift*> m_forkLifts;
	};

}
