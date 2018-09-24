//
// 2017-12-04, jjuiddong
// astar, nevigation path render view
//
#pragma once


class cPathMapView : public framework::cDockWindow
{
public:
	cPathMapView();
	virtual ~cPathMapView();

	bool Init();
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnPreRender(const float deltaSeconds) override;
	virtual void OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect) override;
	virtual void OnEventProc(const sf::Event &evt) override;
	virtual void OnResetDevice() override;


protected:
	void RenderPathEdit(graphic::cRenderer &renderer, const XMMATRIX &tm=graphic::XMIdentity);
	void RenderNaviEdit(graphic::cRenderer &renderer, const XMMATRIX &tm = graphic::XMIdentity);

	void UpdateLookAt();
	void OnWheelMove(const float delta, const POINT mousePt);
	void OnMouseMove(const POINT mousePt);
	void OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt);
	void OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt);


public:
	graphic::cGridLine m_ground;
	graphic::cRenderTarget m_renderTarget;
	graphic::cTextManager m_textMgr;
	graphic::cTextManager m_textMgr2;
	graphic::cTextManager m_textMgr3;

	bool m_showAxis;
	bool m_showGround;
	bool m_showVertexId;
	bool m_showEdgeWeight;
	bool m_showNavigationNode;

	// MouseMove Variable
	POINT m_viewPos;
	sRectf m_viewRect; // detect mouse event area
	POINT m_mousePos; // window 2d mouse pos
	Vector3 m_mousePickPos; // mouse cursor pos in ground picking
	bool m_mouseDown[3]; // Left, Right, Middle
	float m_rotateLen;
	Plane m_groundPlane1;
	Plane m_groundPlane2;
};
