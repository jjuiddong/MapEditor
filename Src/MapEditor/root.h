//
// 2017-07-04, jjuiddong
// global root class
//
#pragma once

#include "mapview.h"
#include "terraineditwindow.h"
#include "patheditwindow.h"
#include "resourcewindow.h"
#include "resourceviewer.h"
#include "dbgwindow.h"
#include "hierarchywindow.h"
#include "componentwindow.h"
#include "pathmapview.h"
#include "tms.h"


struct eMapEditType {
	enum Enum {
		NORMAL,
		AREA,
		LINE,
	};
};


class cRoot
{
public:
	cRoot();
	virtual ~cRoot();

	bool Init();


public:
	cMapView *m_mapView;
	cPathMapView *m_pathMapView;
	cTerrainEditWindow *m_terrainEditWindow;
	cPathEditWindow *m_pathEditWindow;
	cHierarchyWindow *m_hierarchyWindow;
	cResourceWindow *m_resWindow;
	cResourceViewer *m_resViewer;
	cComponentWindow *m_compWindow;
	cDbgWindow *m_dbgWindow;

	cConfig m_cfg;
	//graphic::cSkyBox m_skybox;
	graphic::cSkyBoxCube m_skybox;
	graphic::cTerrain m_terrain;
	graphic::cTerrainCursor m_tCursor;
	tms::cTms m_tms;
	graphic::cCamera3D m_camWorld; // World Camera
	graphic::cCamera3D m_camObserver; // Observer World Camera

	graphic::cDepthBuffer m_depthBuff;
	graphic::cTile *m_selectTile;
	graphic::cNode *m_selectModel;
	graphic::cGizmo *m_gizmo; // create from mapview
	eMapEditType::Enum m_mapEditType;
};
