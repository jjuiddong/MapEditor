
#include "stdafx.h"
#include "root.h"


using namespace graphic;

cRoot::cRoot()
	: m_camWorld("world camera")
	, m_camObserver("observer camera")
	, m_selectTile(NULL)
	, m_gizmo(NULL)
	, m_mapEditType(eMapEditType::NORMAL)
	, m_hWnd(NULL)
{
}

cRoot::~cRoot()
{
	m_cfg.Write("config.txt");
	Clear();	
}


bool cRoot::Init()
{
	using namespace framework;
	cRenderer &renderer = *framework::GetMainRenderer();
	const cViewport vp(0, 0, 1024, 1024, 1.f, 100000.0f);

	const Vector3 origLookAt(117.383698f, 0.000000000f, 100.681229f);
	const Vector3 origEyePos(-128.885040f, 175.372223f, -145.587723f);
	m_camWorld.SetCamera(origEyePos, origLookAt, Vector3(0, 1, 0));
	m_camWorld.SetProjection(MATH_PI / 4.f, (float)vp.m_vp.Width / (float)vp.m_vp.Height, vp.m_vp.MinDepth, vp.m_vp.MaxDepth);
	m_camWorld.SetViewPort(vp.m_vp.Width, vp.m_vp.Height);
	m_camObserver = m_camWorld;

	m_gizmo = new cGizmo();
	m_gizmo->Create(renderer);
	m_gizmo->m_transform.pos = Vector3(10, 10, 10);

	m_modelExts.clear();
	m_modelExts.push_back(".x"); m_modelExts.push_back(".X");
	m_modelExts.push_back(".dae"); m_modelExts.push_back(".DAE");
	m_modelExts.push_back(".fbx"); m_modelExts.push_back(".FBX");
	m_modelExts.push_back(".3ds"); m_modelExts.push_back(".3DS");
	m_modelExts.push_back(".stp"); m_modelExts.push_back(".STP");
	m_modelExts.push_back(".stl"); m_modelExts.push_back(".STL");

	//cMainCamera::Get()->PushCamera(&m_camWorld); // push maincamera
	cAutoCam cam(&m_camWorld);

	m_cfg.Read("config.txt");
	string readFile = m_cfg.GetString("ReadFile");

	//m_skybox.Create(renderer, "skybox");
	m_skybox.Create(renderer, "../media/terraintextures/sky.dds");

	cRenderWindow *main = cDockManager::Get()->GetMainWindow();
	m_hWnd = main->getSystemHandle();
	m_mapView = new cMapView();
	m_mapView->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, main, NULL);
	m_mapView->Init();

	m_terrainEditWindow = new cTerrainEditWindow();
	m_terrainEditWindow->Create(eDockState::DOCKWINDOW, eDockSlot::RIGHT, main, (framework::cDockWindow*)m_mapView, 0.25f
		, eDockSizingOption::PIXEL);
	m_terrainEditWindow->Init(renderer);

	m_pathEditWindow = new cPathEditWindow();
	m_pathEditWindow->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, main, (framework::cDockWindow*)m_terrainEditWindow);
	m_pathEditWindow->Init(renderer);

	m_resWindow = new cResourceWindow();
	m_resWindow->Create(eDockState::DOCKWINDOW, eDockSlot::BOTTOM, main, (framework::cDockWindow*)m_mapView, 0.2f);
	m_resWindow->Init();

	m_compWindow = new cComponentWindow();
	m_compWindow->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, main, (framework::cDockWindow*)m_resWindow);
	m_compWindow->Init();

	m_resViewer = new cResourceViewer();
	m_resViewer->Create(eDockState::DOCKWINDOW
		, eDockSlot::TAB, main, (framework::cDockWindow*)m_mapView);
	m_resViewer->Init();

	m_pathMapView = new cPathMapView();
	m_pathMapView->Create(eDockState::DOCKWINDOW, eDockSlot::TAB, main, (framework::cDockWindow*)m_mapView);
	m_pathMapView->Init();


	m_hierarchyWindow = new cHierarchyWindow();
	m_hierarchyWindow->Create(eDockState::DOCKWINDOW, eDockSlot::RIGHT, main, m_mapView, 0.15f
		, eDockSizingOption::PIXEL);

	m_dbgWindow = new cDbgWindow("Display Window");
	m_dbgWindow->Create(eDockState::DOCKWINDOW
		, eDockSlot::BOTTOM, main, (framework::cDockWindow*)m_terrainEditWindow, 0.4f, eDockSizingOption::PIXEL);

	m_tCursor.Create(renderer);

	cViewport dvp;
	dvp.Create(0, 0, 1024, 1024, 0, 1);
	m_depthBuff.Create(renderer, dvp, false);

	return true;
}


// check model file extends name
// for fast app
bool cRoot::IsModelExtendsName(const char *fileName)
{
	auto it = find(m_modelExts.begin(), m_modelExts.end()
		, common::GetFileExt(fileName));
	return m_modelExts.end() != it;
}


void cRoot::Clear()
{
	m_terrain.Clear();
	SAFE_DELETE(m_gizmo);
}
