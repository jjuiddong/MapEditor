//
// 2017-06-29, jjuiddong
// Transportation Management System
//
#pragma once

#include "truck.h"


namespace tms
{

	class cTruck;

	class cTms : public cJobManager
	{
	public:
		cTms();
		virtual ~cTms();

		virtual bool Create(graphic::cRenderer &renderer);
		virtual void Update(graphic::cRenderer &renderer, const float deltaSeconds);
		virtual void Render(graphic::cRenderer &renderer);
		virtual void Clear();

		void UpdateLineList(graphic::cRenderer &renderer);
		bool WritePath();


	protected:
		virtual void ProcJobEtc(const sJob &job);
		cTruck* GetFreeTruck(graphic::cRenderer &renderer);
		void TruckAllocate();


	public:
		enum { MAX_TRUCK=20, TRUCK_ALLOC_TIME = 10};

		vector<cTruck*> m_trucks;
		ai::cPathFinder m_pathFinder;
		float m_truckAllocTime; // TRUCK_ALLOC_TIME seconds

		// debugging
		graphic::cDbgLineList m_lineList;
	};

}