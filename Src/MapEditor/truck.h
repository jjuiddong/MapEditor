//
// 2017-06-29, jjuiddong
// vehicle truck
//
#pragma once

#include "jobmanager.h"
#include "vehiclepath.h"


namespace tms
{
	class cTms;

	class cTruck : public graphic::cModel
	{
	public:
		cTruck();
		virtual ~cTruck();

		bool Create(graphic::cRenderer &renderer, cTms *tms
			, const Transform &transform=Transform::Identity);

		virtual bool Render(graphic::cRenderer &renderer, const XMMATRIX &parentTm = graphic::XMIdentity, const int flags = 1) override;
		virtual bool Update(graphic::cRenderer &renderer, const float deltaSeconds) override;

		void Move(const Vector3 &dest);
		void AssignJob(const sJob &job);


	protected:
		void NextRandomMove();
		virtual void InitModel(graphic::cRenderer &renderer) override;


	public:
		struct eState { 
			enum Enum { WAIT, MOVE, REACH, WORK
				//, RETURN, FINISH 
			}; 
		};

		eState::Enum m_state;
		cTms *m_tms; // reference
		sJob m_job;
		cVehiclePath m_vpath;
		float m_delayTime;

		// debug
		graphic::cDbgLineList m_lineList; // show path
	};

}
