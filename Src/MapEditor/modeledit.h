//
// 2017-07-06, jjuiddong
// model edit
//	- pos, rot, scale
//
#pragma once


class cModelEdit
{
public:
	cModelEdit();
	virtual ~cModelEdit();

	void Render();


public:
	bool m_showModel;
	bool m_showBoundingBox;
	bool m_showAxis;
	Plane m_groundPlane1;
};
