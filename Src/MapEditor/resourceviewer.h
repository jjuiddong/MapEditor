//
// 2017-07-05, jjuiddong
// Resource Viewer
//
#pragma once


class cResourceViewer : public framework::cDockWindow
{
public:
	cResourceViewer();
	virtual ~cResourceViewer();

	bool Init();
	bool LoadResource(const StrPath &fileName);

	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnPreRender(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect) override;
	virtual void OnEventProc(const sf::Event &evt) override;
	virtual void OnResetDevice() override;


protected:
	void AdjustTextureSize();
	bool IsMouseHoverOnUI();
	void RenderModelInfo();
	void RenderModelNode(const graphic::sRawNode &node);
	void RenderLocalTransformInfo();
	void RenderMeshInfo();
	void RenderAnimationInfo();
	void RenderShader();
	bool WriteModelInfo();
	void UpdateLookAt();
	void OnWheelMove(const float delta, const POINT mousePt);
	void OnMouseMove(const POINT mousePt);
	void OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt);
	void OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt);


public:
	struct eState { 
		enum Enum {
			NONE, TEXTURE, MODEL
		}; };

	eState::Enum m_state;
	graphic::cCamera3D m_camera;
	Str64 m_fileSize;
	bool m_isRenderWireFrame;
	bool m_isRenderAxis;
	bool m_isOutline;
	bool m_isShadow;
	bool m_checkLoadModel;
	graphic::cQuad2D m_quad;
	graphic::cModel m_model;
	graphic::cGridLine m_ground;
	graphic::cRenderTarget m_renderTarget;
	graphic::cCascadedShadowMap m_ccsm;
	graphic::cQuad2D m_depthQuad;

	// MouseMove Variable
	POINT m_viewPos;
	sRectf m_viewRect;
	POINT m_mousePos;
	bool m_mouseDown[3]; // Left, Right, Middle
	float m_rotateLen;
	Plane m_groundPlane1;
	Plane m_groundPlane2;

	// Etc
	bool m_isAdjustTextureSize;
	bool m_isOpenHud;
	bool m_isEditAnimationTime;
};
