//
// 2017-11-08, jjuiddong
// MapTool CCTV object
//
#pragma once


class cCCtv : public graphic::cCube
{
public:
	cCCtv();
	virtual ~cCCtv();
	
	bool Create(graphic::cRenderer &renderer
			, const char *name="CCTV");

	virtual cNode* Clone(graphic::cRenderer &renderer) const override;


public:
	StrPath m_movieFileName;
};
