//
// 2017-07-04, jjuiddong
// map view
//
#pragma once


class cMapView : public framework::cDockWindow
{
public:
	cMapView();
	virtual ~cMapView();

	bool Init();
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnPreRender(const float deltaSeconds) override;
	virtual void OnPostRender(const float deltaSeconds) override;
	virtual void OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect) override;
	virtual void OnEventProc(const sf::Event &evt) override;
	virtual void OnResetDevice() override;


protected:
	void RenderScene(graphic::cRenderer &renderer
		, const char *techniqueName
		, const bool isShadowMap
		, const XMMATRIX &tm = graphic::XMIdentity 
	);
	void RenderSelectModel(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderLightEdit(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderGizmo(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderPathEdit(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderAreaEdit(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderLineEdit(graphic::cRenderer &renderer, const XMMATRIX &tm);
	void RenderNaviEdit(graphic::cRenderer &renderer, const XMMATRIX &tm);

	void UpdateLookAt();
	void OnWheelMove(const float delta, const POINT mousePt);
	void OnMouseMove(const POINT mousePt);
	void OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt);
	void OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt);
	void ContextMenu();
	graphic::cNode* CopyAndPasteModel(const graphic::cNode &src);


public:
	graphic::cRenderTarget m_renderTarget;
	graphic::cCascadedShadowMap m_ccsm;
	graphic::cOcean m_ocean;
	graphic::cQuad2D m_depthQuad;

	bool m_isShadow; // default true
	bool m_isGrid; // default true
	bool m_isTexture; // default true
	bool m_isOcean; // default true
	float m_deltaSeconds;

	// MouseMove Variable
	POINT m_viewPos;
	sRectf m_viewRect; // detect mouse event area
	POINT m_mousePos; // window 2d mouse pos
	Vector3 m_mousePickPos; // mouse cursor pos in ground picking
	bool m_mouseDown[3]; // Left, Right, Middle
	float m_rotateLen;
	Plane m_groundPlane1;
	Plane m_groundPlane2;
	bool m_isAreaEdit;
	bool m_isLineEdit;
	Vector3 m_editPos[2];
	graphic::cNode *m_copyAndPasteNode;

	// Debugging
	graphic::cDbgFrustum m_dbgFrustum;
};
