//
// 2017-02-21, jjuiddong
// Warehouse Rack
// Pure Rendering Object
//
//			   ||                  ||    ||                 ||
//			   ||                  ||    ||                 ||
//		 ||    || -------------    ||--- || ----------||--- || beam
//       ||   /                    ||   /             ||   /||
//       || /                      || /  ||           || /  ||
//       || ====================== || =============== ||    ||
//		 ||    ||                  ||    ||           ||    ||
//		 ||	   ||                  ||    ||           ||    ||
//		 ||    / --------------    ||--- / -----------|| -- || beam
//       ||   /                    ||  /              ||   /
//       || /                      || /  ||           || /  ||
//       || ====================== || =============== ||    ||
//       ||    ||                  ||    ||           ||    ||
//       ||    ||                  ||    ||           ||    ||
//       ||    ||                  ||    ||           ||    ||
//		 ||	pillar                 || pillar3         ||   pillar
//		 ||                        ||                 ||
//     pillar1                       pillar4          pillar
//
//         Y axis     
//         |         Z axis (Back)
//         |      /
//         |    /
//         | /
//           -------------------> X axis
//         (Front)               
//
#pragma once


namespace graphic
{

	class cRack
	{
	public:
		enum {MAX_ROW = 6, MAX_COL = 32};
		struct sRackInfo
		{
			StrId name;
			int type;
			Vector3 pos;	// Rack Center Position, Local Position
			Vector3 dim;	// Rack Dimmension
			Vector3 dir;	// Rack Direction (Local Space)
			Quaternion rot;	// Rack Rotation
			int row;
			int col;
			float width[MAX_COL+1]; // row °¹¼ö¸¸Å­, temporary + 1
			float height[MAX_ROW+1]; // row °¹¼ö¸¸Å­, temporary + 1
			float pillarSize; // default : 0.05, 5cm
			float beamSize; // default: 0.08, 8cm
			bool color;
			bool shadow;
		};

		cRack();
		virtual ~cRack();

		bool Create(const sRackInfo &info);
		void Update(const float deltaSeconds);
		void Edit(const sRackInfo &info);
		bool GetPosition(const int row, const int col, const int index
			, OUT cBoundingBox &out) const;
		void MemClear();
		void Clear();


	public:
		int m_id;
		int m_rackerId;
		sRackInfo m_info;
		Matrix44 m_tm; // Rotation + Position

		vector<cBoundingBox> m_pillars;
		vector<cBoundingBox> m_beams;
		vector<cBoundingBox> m_diagBars;
		cBoundingBox m_boundingBox; // Local Space
	};

}
