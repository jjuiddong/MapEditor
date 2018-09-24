//
// 2017-09-27, jjuiddong
//
//
#pragma once


class cGeometryEdit
{
public:
	cGeometryEdit();
	virtual ~cGeometryEdit();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer);


public:
	bool m_isEdit; // default : false
	bool m_isHiLightTile; // default : false
	int m_brushType; // normal, flat
};
