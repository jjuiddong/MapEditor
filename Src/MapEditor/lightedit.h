//
// 2017-10-01, jjuiddong
// Light Edit UI
//
#pragma once


class cLightEdit
{
public:
	cLightEdit();
	virtual ~cLightEdit();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer);


public:
	bool m_openLightEdit;
	bool m_showLightDir;
	Vector3 m_lightDir;
};
