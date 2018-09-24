//
// 2017-06-14, jjuiddong
// job manager
//
#pragma once


namespace tms
{
	class cTms;
	class cTruck;
	//class cForkLift;

	class cFloor {
	public:
		cFloor() {}
		virtual ~cFloor() {}
	};

	struct sJob
	{
		sJob() {}

		enum eType {FORKLIFT_LOAD, FORKLIFT_UNLOAD, 
			TRUCK_LOAD, TRUCK_LOAD_END, TRUCK_UNLOAD, TRUCK_UNLOAD_END, TRUCK_GOHOME, TRUCK_GOHOME_END};
		eType type;

		union
		{
			//struct sForkLift {
			//	cForkLift *vehicle;
			//	cPalete *load;
			//	Vector3 dest;
			//} forkLift;

			struct sTruck {
				cTruck *truck;
				Vector3 home;
				Vector3 dest;
				Vector3 returnHome;
			} truck;

		};
	};


	class cJobManager
	{
	public:
		cJobManager();
		virtual ~cJobManager();
		
		virtual bool Create(cFloor *floor);
		virtual void Update(graphic::cRenderer &renderer, const float deltaSeconds);
		virtual void AddJobs(const sJob &job);


	protected:
		virtual void ProcessJobs();
		virtual void ProcJobEtc(const sJob &job);


	public:
		cFloor *m_floor;
		queue<sJob> m_jobQ;
	};

}
