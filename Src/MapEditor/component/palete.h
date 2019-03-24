//
// 2017-02-22, jjuiddong
// Palete
//
#pragma once

#include "box.h"


namespace graphic
{
	class cBox;
	class cRack;
	class cRacker;

	class cPalete
	{
	public:
		cPalete();
		virtual ~cPalete();
		
		bool Create(cRenderer &renderer
			, const StrId &name, const cRack *rack
			, const int row, const int col, const int index);

		cBox* Pick(const Ray &ray, const Matrix44 &tm = Matrix44::Identity);


	public:
		int m_id;
		int m_rackId;
		StrId m_name;
		cBox m_box[1];  // floor local space coordinate system
		const cRack *m_rack; // reference
		cRacker *m_racker; // reference
		cBoundingBox m_boundingBox;
	};

}
