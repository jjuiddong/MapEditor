//
// 2017-07-05, jjuiddong
// debug window
//
#pragma once


class cDbgWindow : public framework::cDockWindow
{
public:
	cDbgWindow(const string &name = "");
	virtual ~cDbgWindow();

	void OnRender(const float deltaSeconds) override;
	void OnUpdate(const float deltaSeconds) override;
	void OnEventProc(const sf::Event &evt);


public:
	Vector3 m_cursorPos;
	Vector3 m_frustumPos;
	Vector3 m_cameraPos;
	Vector3 m_lookAtPos;
	bool m_isShowWireFrame;
	bool m_isShowBuilding;
	bool m_isShowTileMap;
	bool m_isShowFrustum;
	bool m_isShowCascadedFrustum;
	bool m_isShowLightFrustum;
	bool m_isShowAxis;
	bool m_isUpdateFrustum;
	bool m_isObserverCamera;
	bool m_isTerrainDebugDisplay;
	bool m_isShadow;
	bool m_showCursor;
	bool m_isShowBoxTexColor;
	bool m_isShowPathMesh;
	bool m_isShowRackDirection;
	bool m_isShowVehiclePath;
	Plane m_groundPlane1;
	Plane m_groundPlane2;
};
