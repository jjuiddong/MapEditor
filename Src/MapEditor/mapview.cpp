
#include "stdafx.h"
#include "mapview.h"

using namespace framework;
using namespace graphic;


cMapView::cMapView() 
	: cDockWindow("MapView")
	, m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_groundPlane2(Vector3(0, -1, 0), 0)
	, m_isShadow(true)
	, m_isTexture(true)
	, m_isGrid(true)
	, m_isOcean(false)
	, m_deltaSeconds(0)
	, m_isAreaEdit(false)
	, m_isLineEdit(false)
	, m_copyAndPasteNode(NULL)
{
}

cMapView::~cMapView()
{
}


bool cMapView::Init()
{
	cRenderer &renderer = GetRenderer();

	sf::Vector2u size((u_int)m_rect.Width()-15, (u_int)m_rect.Height() - 50);
	cViewport vp = renderer.m_viewPort;
	vp.m_vp.Width = (float)size.x;
	vp.m_vp.Height = (float)size.y;
	m_renderTarget.Create(renderer, vp);

	m_dbgFrustum.Create(renderer, g_root.m_camWorld.GetViewProjectionMatrix());
	m_ccsm.Create(renderer);

	m_ocean.Create(renderer, vp.m_vp.Width, vp.m_vp.Height);
	m_depthQuad.Create(renderer, 301, 0, 300, 300);

	return true;
}


void cMapView::OnUpdate(const float deltaSeconds)
{
	cAutoCam cam(&g_root.m_camWorld);

	cRenderer &renderer = GetRenderer();
	g_root.m_camWorld.Update(deltaSeconds);
	g_root.m_terrain.Update(renderer, deltaSeconds);
	m_deltaSeconds = deltaSeconds;

	// Edit Geometry
	const bool editGeometry = GetAsyncKeyState(VK_LCONTROL) 
		&& (m_mouseDown[0] || m_mouseDown[1])
		&& g_root.m_terrainEditWindow->m_geoEdit.m_isEdit;
	if (editGeometry)
	{
		eGeoEditType::Enum editType;
		switch (g_root.m_terrainEditWindow->m_geoEdit.m_brushType)
		{
		case 0: editType = m_mouseDown[0] ? eGeoEditType::UP : eGeoEditType::DOWN; break; // normal
		case 1: editType = eGeoEditType::FLAT; break; // flat
		default: assert(0); break;
		}

		const Ray ray = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);
		const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
		g_root.m_tCursor.GeometryEdit(renderer, g_root.m_terrain
			, editType, pos, deltaSeconds, g_root.m_terrainEditWindow->m_geoEdit.m_isHiLightTile );

		g_root.m_tCursor.UpdateCursor(renderer, g_root.m_terrain, pos);
	}

	//g_root.m_tms.Update(renderer, deltaSeconds);

	// Update CursorPos
	{
		const Ray ray = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);
		m_mousePickPos = g_root.m_terrain.GetHeightFromRay(ray);
	}
}


void cMapView::RenderScene(cRenderer &renderer
	, const char *techniqueName
	, const bool isShadowMap
	, const XMMATRIX &tm //=XMIdentity 
)
{
	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);
	g_root.m_terrain.SetTechnique(techniqueName);

	DirectX::CommonStates common(renderer.GetDevice());
	renderer.GetDevContext()->RSSetState(g_root.m_dbgWindow->m_isShowWireFrame ? common.Wireframe() : common.CullCounterClockwise());
	{
		if (!g_root.m_dbgWindow->m_isShowWireFrame)
			g_root.m_skybox.Render(renderer);

		if (isShadowMap)
			g_root.m_terrain.RenderCascadedShadowMap(renderer, m_ccsm);
		else
			g_root.m_terrain.Render(renderer);
	}
	if (m_isOcean)
		m_ocean.Render(renderer, &g_root.m_camWorld, &g_root.m_skybox, m_deltaSeconds);
	renderer.GetDevContext()->RSSetState(common.CullCounterClockwise());

	RenderSelectModel(renderer, tm);

	// Render Geometry Edit Cursor
	if (g_root.m_terrainEditWindow->m_geoEdit.m_isEdit)
		g_root.m_tCursor.Render(renderer, tm);

	RenderLightEdit(renderer, tm);
	RenderGizmo(renderer, tm);

	if (renderer.m_isDbgRender)
	{
		static cTexture shadowTex(g_root.m_depthBuff.m_depthSRV);
		shadowTex.m_texSRV = g_root.m_depthBuff.m_depthSRV;
		m_depthQuad.m_texture = &shadowTex;
		m_depthQuad.Render(renderer);
	}

	RenderPathEdit(renderer, tm);
	RenderAreaEdit(renderer, tm);
	RenderLineEdit(renderer, tm);
	RenderNaviEdit(renderer, tm);
	//g_root.m_tms.Render(renderer);
}


// Render Outline Selected model
void cMapView::RenderSelectModel(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	if (g_root.m_selectModel)
	{
		CommonStates state(renderer.GetDevice());
		renderer.GetDevContext()->OMSetDepthStencilState(state.DepthNone(), 0);
		renderer.GetDevContext()->OMSetBlendState(state.NonPremultiplied(), NULL, 0xffffffff);

		renderer.BindTexture(g_root.m_depthBuff, 7);
		g_root.m_selectModel->SetTechnique("Outline");
		Matrix44 parentTm = g_root.m_selectModel->GetParentWorldMatrix();
		g_root.m_selectModel->Render(renderer, parentTm.GetMatrixXM());

		if (g_root.m_terrainEditWindow->m_modelEdit.m_showAxis)
			renderer.m_dbgAxis.Render(renderer, g_root.m_selectModel->GetWorldMatrix().GetMatrixXM());

		renderer.GetDevContext()->OMSetDepthStencilState(state.DepthDefault(), 0);
		renderer.GetDevContext()->OMSetBlendState(state.Opaque(), NULL, 0xffffffff);
	}
}


void cMapView::RenderLightEdit(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	if (g_root.m_terrainEditWindow->m_lightEdit.m_openLightEdit
		&& g_root.m_terrainEditWindow->m_lightEdit.m_showLightDir)
	{
		Vector3 p0 = GetMainCamera().GetEyePos()
			+ (GetMainCamera().GetUpVector() * 20.f)
			+ GetMainCamera().GetDirection() * 50;
		Vector3 p1 = GetMainLight().m_direction * 3 + p0;
		renderer.m_dbgArrow.SetDirection(p0, p1);
		renderer.m_dbgArrow.Render(renderer);

		Vector3 p2 = GetMainLight().m_direction * 3 + p1;
		renderer.m_dbgLine.SetLine(p2, GetMainLight().m_direction * 1000 + p2, 0.1f);
		renderer.m_dbgLine.Render(renderer);
	}
}


void cMapView::RenderGizmo(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	RET(!g_root.m_gizmo->m_controlNode)

	if (GetFocus() == m_owner->getSystemHandle())
	{
		if (GetAsyncKeyState('R'))
			g_root.m_gizmo->m_type = eGizmoEditType::ROTATE;
		if (GetAsyncKeyState('T'))
			g_root.m_gizmo->m_type = eGizmoEditType::TRANSLATE;
		if (GetAsyncKeyState('S'))
			g_root.m_gizmo->m_type = eGizmoEditType::SCALE;
		if (GetAsyncKeyState('W'))
			g_root.m_gizmo->SetTransformType(eGizmoTransform::WORLD);
		if (GetAsyncKeyState('L'))
			g_root.m_gizmo->SetTransformType(eGizmoTransform::LOCAL);
		if (GetAsyncKeyState('H'))
			g_root.m_gizmo->Cancel();
	}

	if (g_root.m_gizmo->Render(renderer, m_deltaSeconds, m_mousePos, m_mouseDown[0] && GetAsyncKeyState(VK_LCONTROL)))
	{
		if (eGizmoEditType::TRANSLATE == g_root.m_gizmo->m_type)
		{
			// Update Tile Allocation
			if (g_root.m_gizmo->m_controlNode->m_parent)
				if (g_root.m_gizmo->m_controlNode->m_parent->m_type == eNodeType::TERRAIN)
					g_root.m_terrain.UpdateModel(g_root.m_gizmo->m_controlNode);
		}
	}
}


void cMapView::RenderPathEdit(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
	RET(!pathEdit.m_isShowWorldPath);

	renderer.m_dbgBox.SetColor(cColor::WHITE);
	renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(1.f, 1.f, 1.f, 1.f);

	for (auto &vtx : pathEdit.m_pathFinder.m_vertices)
	{
		const cBoundingBox bbox(vtx.pos + Vector3(0, pathEdit.m_offsetY, 0)
			, Vector3(1, 1, 1) * pathEdit.m_vertexSize
			, Quaternion());

		renderer.m_dbgBox.SetBox(bbox);
		renderer.m_dbgBox.Render(renderer);

		if ((1 <= vtx.type) && (vtx.edge[0] >= 0))
		{
			ai::sVertex &to = pathEdit.m_pathFinder.m_vertices[vtx.edge[0]];
			const Vector3 dir = (to.pos - vtx.pos).Normal();

			renderer.m_dbgArrow.SetDirection(vtx.pos + Vector3(0, 1, 0)
				, vtx.pos + dir + Vector3(0, 1, 0)
				, 0.1f
			);
			renderer.m_dbgArrow.Render(renderer);
		}
	}

	pathEdit.m_lineList.Render(renderer);
}


// Area 편집 출력
void cMapView::RenderAreaEdit(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	// Render Area
	RET(eMapEditType::AREA != g_root.m_mapEditType);

	CommonStates state(renderer.GetDevice());
	renderer.GetDevContext()->OMSetDepthStencilState(state.DepthNone(), 0);
	renderer.GetDevContext()->OMSetBlendState(state.NonPremultiplied(), NULL, 0xffffffff);

	if (m_isAreaEdit)
	{
		m_editPos[1] = m_mousePickPos;
		g_root.m_terrain.GetRect3D(renderer, m_editPos[0], m_editPos[1], renderer.m_rect3D);
		renderer.m_rect3D.Render(renderer, tm);

		// Finish Area Edit
		if (!GetInput().m_mouseDown[0])
		{
			m_isAreaEdit = false;
			g_root.m_mapEditType = eMapEditType::NORMAL;

			cRect3D *newRect = new cRect3D();
			newRect->SetRect(renderer, renderer.m_rect3D);
			g_root.m_terrain.AddChild(newRect);
		}
	}
	else
	{
		if (GetInput().m_mouseDown[0] && GetAsyncKeyState(VK_LCONTROL))
		{
			m_isAreaEdit = true;
			m_editPos[0] = m_mousePickPos;
		}
	}

	Transform tfm;
	tfm.pos = m_mousePickPos;
	tfm.scale = Vector3(1, 1, 1)*0.2f;
	renderer.m_dbgBox.SetColor(cColor::WHITE);
	renderer.m_dbgBox.SetBox(tfm);
	renderer.m_dbgBox.Render(renderer, tm);

	renderer.GetDevContext()->OMSetDepthStencilState(state.DepthDefault(), 0);
	renderer.GetDevContext()->OMSetBlendState(state.Opaque(), NULL, 0xffffffff);
}


void cMapView::RenderLineEdit(graphic::cRenderer &renderer, const XMMATRIX &tm)
{
	RET(eMapEditType::LINE != g_root.m_mapEditType);

	if (m_isLineEdit)
	{
		m_editPos[1] = m_mousePickPos;
		renderer.m_line.SetLine(m_editPos[0], m_editPos[1], 1);
		renderer.m_line.Render(renderer, tm);

		// Finish Line Edit
		if (GetInput().IsClick())
		{
			cLine *newLine = new cLine();
			newLine->Create(renderer, m_editPos[0], m_editPos[1]);
			g_root.m_terrain.AddModel(newLine);

			m_isLineEdit = true;
			m_editPos[0] = m_editPos[1];
		}
		else if (GetInput().m_mouseDown[1])
		{
			m_isLineEdit = false;
			g_root.m_mapEditType = eMapEditType::NORMAL;
		}
	}
	else
	{
		if (GetInput().IsClick() && GetAsyncKeyState(VK_LCONTROL))
		{
			m_isLineEdit = true;
			m_editPos[0] = m_mousePickPos;
		}
	}

	Transform tfm;
	tfm.pos = m_mousePickPos;
	tfm.scale = Vector3(1, 1, 1)*0.2f;
	renderer.m_dbgBox.SetColor(cColor::WHITE);
	renderer.m_dbgBox.SetBox(tfm);
	renderer.m_dbgBox.Render(renderer, tm);
}


void cMapView::RenderNaviEdit(cRenderer &renderer, const XMMATRIX &tm)
{
	cNaviEdit &naviEdit = g_root.m_terrainEditWindow->m_naviEdit;

	if (!naviEdit.m_showNaviMesh)
		return;

	naviEdit.RenderNavigationMesh(renderer);
}


void cMapView::OnPreRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();

	if (m_isShadow)
	{
		cAutoCam cam(&g_root.m_camWorld);
		GetMainCamera().Bind(renderer);
		GetMainLight().Bind(renderer);
		renderer.m_textMgr.NewFrame();
		g_root.m_terrain.BuildCascadedShadowMap(renderer, m_ccsm);
	}

	cAutoCam cam(g_root.m_dbgWindow->m_isObserverCamera ? &g_root.m_camObserver : &g_root.m_camWorld);

	// Render Outline select model
	if (g_root.m_selectModel)
	{
		GetMainCamera().Bind(renderer);
		if (g_root.m_depthBuff.Begin(renderer))
		{
			const Matrix44 parentTm = g_root.m_selectModel->GetParentWorldMatrix();
			g_root.m_selectModel->SetTechnique("DepthTech");
			g_root.m_selectModel->Render(renderer, parentTm.GetMatrixXM());
		}
		g_root.m_depthBuff.End(renderer);
	}

	// 지형 렌더링
	if (m_renderTarget.Begin(renderer, Vector4(20.f / 255.f, 20.f / 255.f, 20.f / 255.f, 1.0f)))
	{
		RenderScene(renderer, m_isShadow? "ShadowMap" : "Unlit", m_isShadow);
		renderer.RenderAxis();
	}
	m_renderTarget.End(renderer);

	// Gizmo로 모델을 조작하지 않고 있다면, 모델을 픽킹한다.
	if (GetInput().IsClick() 
		&& !g_root.m_gizmo->IsKeepEditMode()
		&& (g_root.m_mapEditType == eMapEditType::NORMAL))
	{
		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		if (m_viewRect.IsIn((float)curPos.x, (float)curPos.y))
		{
			const Ray ray = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);
			if (cNode *node = g_root.m_terrain.Picking(ray, eNodeType::MODEL, false))
			{
				g_root.m_selectModel = node;
				g_root.m_gizmo->SetControlNode(node);
				g_root.m_terrainEditWindow->m_gridEdit.SelectTile(dynamic_cast<cTile*>(node->m_parent));
			}
		}
	}
}


void cMapView::OnRender(const float deltaSeconds)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_viewPos = { (int)(pos.x), (int)(pos.y) };
	m_viewRect = { pos.x + 5, pos.y, pos.x + m_rect.Width() - 30, pos.y + m_rect.Height() - 50 };
	ImGui::Image(m_renderTarget.m_resolvedSRV, ImVec2(m_rect.Width()-15, m_rect.Height()-50));

	// HUD UI
	const float windowAlpha = 0.0f;
	bool isOpen = true;
	const float hudH = 145; // 95
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowPos(ImVec2(pos.x+5, pos.y));
	ImGui::SetNextWindowSize(ImVec2(min(m_viewRect.Width(), 400), min(m_viewRect.Height(), hudH)));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	if (ImGui::Begin("MapView Information", &isOpen, ImVec2(400, hudH), windowAlpha, flags))
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Spacing();
		ImGui::Checkbox("Shadow", &m_isShadow); ImGui::SameLine();
		ImGui::Checkbox("Wireframe", &g_root.m_dbgWindow->m_isShowWireFrame); ImGui::SameLine();
		if (ImGui::Checkbox("Grid", &m_isGrid))
		{
			for (auto &tile : g_root.m_terrain.m_tiles)
				tile->m_isRenderLine = m_isGrid;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Ocean", &m_isOcean);

		if (ImGui::Button("Center Camera"))
		{
			g_root.m_camWorld.SetCamera(Vector3(-100,80,-100), Vector3(0,0,0), Vector3(0, 1, 0));
		}
		//ImGui::Checkbox("Texture", &m_isTexture);
		ImGui::End();
	}
	ImGui::PopStyleColor();

	// Update Gizmo Information
	if (g_root.m_gizmo->IsKeepEditMode())
	{
		const Transform &tfm = g_root.m_gizmo->m_controlNode->m_transform;

		Str64 text;
		ImGui::BeginTooltip();
		switch (g_root.m_gizmo->m_type)
		{
		case eGizmoEditType::TRANSLATE:
			text.Format("pos x = %f, y = %f, z = %f", tfm.pos.x, tfm.pos.y, tfm.pos.z);
			break;
		case eGizmoEditType::SCALE:
			text.Format("scale x = %f, y = %f, z = %f", tfm.scale.x, tfm.scale.y, tfm.scale.z);
			break;
		case eGizmoEditType::ROTATE:
			text.Format("rot x = %f, y = %f, z = %f, w = %f", tfm.rot.x, tfm.rot.y, tfm.rot.z, tfm.rot.w);
			break;
		}
		ImGui::Text(text.c_str());
		ImGui::EndTooltip();
	}

	// Popup Menu From Mouse Right Button
	ContextMenu();
}


// Popup Menu From Mouse Right Button
void cMapView::ContextMenu()
{
	if (GetInput().IsClick(1))
	{
		// 기즈모로 조작하고 있지 않을때 만 체크
		if (!g_root.m_gizmo->IsKeepEditMode()
			&& (g_root.m_mapEditType == eMapEditType::NORMAL))
		{
			cAutoCam cam(&g_root.m_camWorld);
			POINT curPos;
			GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
			ScreenToClient(m_owner->getSystemHandle(), &curPos);
			const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
			if (m_viewRect.IsIn((float)curPos.x, (float)curPos.y))
			{
				const Ray ray = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);
				if (cNode *node = g_root.m_terrain.Picking(ray, eNodeType::MODEL, false))
				{
					m_copyAndPasteNode = node;
					g_root.m_selectModel = node;
					g_root.m_gizmo->SetControlNode(NULL);
					g_root.m_terrainEditWindow->m_gridEdit.SelectTile(dynamic_cast<cTile*>(node->m_parent));
					ImGui::OpenPopup("select");
				}
			}
		}
	}

	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));
	if (ImGui::BeginPopup("select"))
	{
		if (ImGui::Selectable("Copy & Paste       "))
		{
			// Copy & Paste Select Model
			if (m_copyAndPasteNode)
			{
				if (cNode *clone = CopyAndPasteModel(*m_copyAndPasteNode))
				{
					const float oldY = clone->m_transform.pos.y;
					clone->m_transform.pos.x = m_mousePickPos.x;
					clone->m_transform.pos.z = m_mousePickPos.z;
					g_root.m_hierarchyWindow->AddModel(clone);
					clone->m_transform.pos.y = oldY;
				}
				m_copyAndPasteNode = NULL;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
}


// cNode 복사
cNode* cMapView::CopyAndPasteModel(const cNode &src)
{
	return src.Clone(GetRenderer());
}


void cMapView::OnPostRender(const float deltaSeconds)
{
}


void cMapView::OnResizeEnd(const eDockResize::Enum type, const sRectf &rect)
{
	if (type == eDockResize::DOCK_WINDOW)
	{
		m_owner->RequestResetDeviceNextFrame();
	}
}


void cMapView::UpdateLookAt()
{
	GetMainCamera().MoveCancel();

	const float centerX = GetMainCamera().m_width / 2;
	const float centerY = GetMainCamera().m_height / 2;
	const Ray ray = GetMainCamera().GetRay((int)centerX, (int)centerY);
	const float distance = m_groundPlane1.Collision(ray.dir);
	if (distance < -0.2f)
	{
		GetMainCamera().m_lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	}
	else
	{ // horizontal viewing
		const Vector3 lookAt = GetMainCamera().m_eyePos + GetMainCamera().GetDirection() * 50.f;
		GetMainCamera().m_lookAt = lookAt;
	}

	GetMainCamera().UpdateViewMatrix();
}


// 휠을 움직였을 때,
// 카메라 앞에 박스가 있다면, 박스 정면에서 멈춘다.
void cMapView::OnWheelMove(const float delta, const POINT mousePt)
{
	UpdateLookAt();

	float len = 0;
	const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
	Vector3 lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	len = (ray.orig - lookAt).Length();

	// zoom in/out
	float zoomLen = 0;
	if (len > 100)
		zoomLen = 50;
	else if (len > 50)
		zoomLen = max(1.f, (len / 2.f));
	else
		zoomLen = (len / 3.f);

	//Vector3 eyePos = GetMainCamera().m_eyePos + ray.dir * ((delta <= 0) ? -zoomLen : zoomLen);
	GetMainCamera().Zoom(ray.dir, (delta < 0) ? -zoomLen : zoomLen);
}


// Handling Mouse Move Event
void cMapView::OnMouseMove(const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	// Update Terrain Cursor if Geometry Edit Mode
	if (g_root.m_terrainEditWindow->m_geoEdit.m_isEdit)
	{
		cRenderer &renderer = GetRenderer();
		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
		g_root.m_tCursor.UpdateCursor(renderer, g_root.m_terrain, pos);
	}

	// if Edit Geometry, cancel Camera Moving
	const bool editGeometry = GetAsyncKeyState(VK_LCONTROL) 
		&& (m_mouseDown[0] || m_mouseDown[1])
		&& g_root.m_terrainEditWindow->m_geoEdit.m_isEdit;
	if (editGeometry)
		return;

	if (g_root.m_gizmo->IsKeepEditMode())
		return;

	if ((eMapEditType::NORMAL != g_root.m_mapEditType) 
		&& (m_isAreaEdit || m_isLineEdit))
		return;

	if (m_mouseDown[0])
	{
		// Path Edit Mode
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		if ((pathEdit.m_isEditVehiclePath)
			&& (eEditVehicle::Edge == pathEdit.m_editVehicleType)
			&& GetAsyncKeyState(VK_LCONTROL))
		{
			if (pathEdit.m_selectVertex >= 0)
			{
				const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
				Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
				pathEdit.m_pathFinder.m_vertices[pathEdit.m_selectVertex].pos = pos;
			}
		}
		else
		{
			Vector3 dir = GetMainCamera().GetDirection();
			Vector3 right = GetMainCamera().GetRight();
			dir.y = 0;
			dir.Normalize();
			right.y = 0;
			right.Normalize();

			GetMainCamera().MoveRight(-delta.x * m_rotateLen * 0.001f);
			GetMainCamera().MoveFrontHorizontal(delta.y * m_rotateLen * 0.001f);
		}
	}
	else if (m_mouseDown[1])
	{
		g_root.m_camWorld.Yaw2(delta.x * 0.005f, Vector3(0, 1, 0));
		g_root.m_camWorld.Pitch2(delta.y * 0.005f, Vector3(0, 1, 0));
	}
	else if (m_mouseDown[2])
	{
		const float len = GetMainCamera().GetDistance();
		GetMainCamera().MoveRight(-delta.x * len * 0.001f);
		GetMainCamera().MoveUp(delta.y * len * 0.001f);
	}
}


// Handling Mouse Button Down Event
void cMapView::OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt)
{
	m_mousePos = mousePt;
	SetCapture();

	switch (button)
	{
	case sf::Mouse::Left:
	{
		m_mouseDown[0] = true;
		GetMainCamera().MoveCancel();

		// Vehicle Edit Mode
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		if (pathEdit.m_isEditVehiclePath)
		{
			const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);

			if (eEditVehicle::Vertex == pathEdit.m_editVehicleType)
			{
				ai::sVertex vtx;
				vtx.pos = m_groundPlane1.Pick(ray.orig, ray.dir);
				vtx.pos.y = g_root.m_terrain.GetHeight(vtx.pos.x, vtx.pos.z);
				pathEdit.m_pathFinder.AddVertex(vtx);
				pathEdit.UpdateLineList(GetRenderer());
			}
			else if (eEditVehicle::Edge == pathEdit.m_editVehicleType)
			{
				int vtxIdx = -1;
				for (u_int i = 0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
				{
					auto &v = pathEdit.m_pathFinder.m_vertices[i];
					cBoundingBox bbox;
					bbox.SetBoundingBox(v.pos
						, Vector3(1, 1, 1) * pathEdit.m_vertexSize
						, Quaternion());

					if (bbox.Pick(ray.orig, ray.dir))
					{
						vtxIdx = i;
						break;
					}
				}

				pathEdit.m_selectVertex = vtxIdx;
			}
			else if (eEditVehicle::Dest == pathEdit.m_editVehicleType)
			{
				const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
				pathEdit.m_editVehicleStartPoint = pos;
				//g_root.m_tms.m_trucks.front()->m_transform.pos = pos;
				//g_root.m_tms.m_trucks.front()->Move(g_root.m_dbgWindow->m_editVehicleEndPoint);
			}
		}
		//

		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		Vector3 p1 = m_groundPlane1.Pick(ray.orig, ray.dir);
		m_rotateLen = min(500.f, (p1 - ray.orig).Length());
	}
	break;

	case sf::Mouse::Right:
	{
		m_mouseDown[1] = true;
		UpdateLookAt();

		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);

		// Vehicle Edit Mode
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		if (pathEdit.m_isEditVehiclePath)
		{

			if ((eEditVehicle::Edge == pathEdit.m_editVehicleType)
				&& (pathEdit.m_selectVertex >= 0))
			{
				int addVtxIdx = -1;
				for (u_int i = 0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
				{
					auto &v = pathEdit.m_pathFinder.m_vertices[i];
					cBoundingBox bbox;
					bbox.SetBoundingBox(v.pos
						, Vector3(1, 1, 1) * pathEdit.m_vertexSize
						, Quaternion());

					if (bbox.Pick(ray.orig, ray.dir))
					{
						addVtxIdx = i;
						break;
					}
				}

				// Add Vertex Edge
				pathEdit.m_pathFinder.AddEdge(pathEdit.m_selectVertex, addVtxIdx);
				pathEdit.m_pathFinder.AddEdge(addVtxIdx, pathEdit.m_selectVertex);
				pathEdit.UpdateLineList(GetRenderer());
			}
			else if (eEditVehicle::Dest == pathEdit.m_editVehicleType)
			{
				const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
				pathEdit.m_editVehicleEndPoint = pos;
				//g_root.m_tms.m_trucks.front()->Move(pathEdit.m_editVehicleEndPoint);
			}
		}
		//
		else
		{
			Vector3 target = m_groundPlane1.Pick(ray.orig, ray.dir);
			const float len = (GetMainCamera().GetEyePos() - target).Length();
		}
	}
	break;

	case sf::Mouse::Middle:
		m_mouseDown[2] = true;
		UpdateLookAt();
		break;
	}
}


void cMapView::OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;
	ReleaseCapture();

	switch (button)
	{
	case sf::Mouse::Left:
		ReleaseCapture();
		m_mouseDown[0] = false;
		break;
	case sf::Mouse::Right:
		m_mouseDown[1] = false;
		ReleaseCapture();
		break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = false;
		ReleaseCapture();
		break;
	}
}


void cMapView::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.code)
		{
		case sf::Keyboard::Return: break;
		case sf::Keyboard::Space: 
		{
			for (int i = 0; i < 3; ++i)
			{
				cTexture texture(m_ccsm.m_shadowMaps[i].m_depthSRV);
				texture.m_texture = m_ccsm.m_shadowMaps[i].m_texture;
				StrPath fileName;
				fileName.Format("depth%d.bmp", i+1);
				texture.WritePNGFile(GetRenderer(), fileName);
			}

		}
		break;
		//case sf::Keyboard::Left: g_root.m_camWorld.MoveRight(-0.5f); break;
		//case sf::Keyboard::Right: g_root.m_camWorld.MoveRight(0.5f); break;
		//case sf::Keyboard::Up: g_root.m_camWorld.MoveUp(0.5f); break;
		//case sf::Keyboard::Down: g_root.m_camWorld.MoveUp(-0.5f); break;
		}
		break;

	case sf::Event::MouseMoved:
	{
		cAutoCam cam(g_root.m_dbgWindow->m_isObserverCamera ? &g_root.m_camObserver : &g_root.m_camWorld);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnMouseMove(pos);
	}
	break;

	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	{
		cAutoCam cam(g_root.m_dbgWindow->m_isObserverCamera ? &g_root.m_camObserver : &g_root.m_camWorld);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		if (sf::Event::MouseButtonPressed == evt.type)
		{
			if (m_viewRect.IsIn((float)curPos.x, (float)curPos.y))
				OnMouseDown(evt.mouseButton.button, pos);
		}
		else
		{
			OnMouseUp(evt.mouseButton.button, pos);
		}
	}
	break;

	case sf::Event::MouseWheelScrolled:
	{
		cAutoCam cam(g_root.m_dbgWindow->m_isObserverCamera ? &g_root.m_camObserver : &g_root.m_camWorld);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnWheelMove(evt.mouseWheelScroll.delta, pos);
	}
	break;
	}
}


void cMapView::OnResetDevice()
{
	cRenderer &renderer = GetRenderer();

	// update viewport
	sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
	g_root.m_camWorld.SetViewPort(viewRect.Width(), viewRect.Height());
	g_root.m_camObserver.SetViewPort(viewRect.Width(), viewRect.Height());
	
	cViewport vp = GetRenderer().m_viewPort;
	vp.m_vp.Width = viewRect.Width();
	vp.m_vp.Height = viewRect.Height();
	m_renderTarget.Create(renderer, vp);

	m_ocean.Create(renderer, vp.m_vp.Width, vp.m_vp.Height);
}
