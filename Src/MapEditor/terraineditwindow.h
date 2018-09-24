//
// 2017-07-04, jjuiddong
// terrain edit window
//
#pragma once


#include "gridedit.h"
#include "modeledit.h"
#include "pathedit.h"
#include "geometryedit.h"
#include "lightedit.h"
#include "naviedit.h"


class cTerrainEditWindow : public framework::cDockWindow
{
public:
	cTerrainEditWindow();
	virtual ~cTerrainEditWindow();

	bool Init(graphic::cRenderer &renderer);
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnEventProc(const sf::Event &evt);


protected:
	StrPath OpenFileDialog();
	bool ReadTerrainFile(graphic::cRenderer &renderer, const StrPath &path);



public:
	StrPath m_readFileName;
	StrPath m_writeFileName;
	bool m_isShowWriteButton;

	cGridEdit m_gridEdit;
	cModelEdit m_modelEdit;
	cPathEdit m_pathEdit;
	cGeometryEdit m_geoEdit;
	cLightEdit m_lightEdit;
	cNaviEdit m_naviEdit;
};
