
#include "stdafx.h"
#include "pathmapview.h"

using namespace framework;
using namespace graphic;


cPathMapView::cPathMapView()
	: framework::cDockWindow("PathMapView")
	, m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_groundPlane2(Vector3(0, -1, 0), 0)
	, m_showAxis(true)
	, m_showGround(true)
	, m_showVertexId(true)
	, m_showEdgeWeight(true)
	, m_showNavigationNode(true)
{
}

cPathMapView::~cPathMapView()
{
}


bool cPathMapView::Init()
{
	cRenderer &renderer = GetRenderer();
	sf::Vector2u size((uint)m_rect.Width() - 15, (uint)m_rect.Height() - 50);

	m_camera.SetCamera(Vector3(-100, 70, -100), Vector3(0, 0, 0), Vector3(0, 1, 0));
	m_camera.SetProjection(MATH_PI / 4.f, (float)size.x / (float)size.y, 1.f, 100000.0f);
	m_camera.SetViewPort((float)size.x, (float)size.y);

	cViewport vp = renderer.m_viewPort;
	vp.m_vp.Width = (float)size.x;
	vp.m_vp.Height = (float)size.y;
	m_renderTarget.Create(renderer, vp);

	m_ground.Create(renderer, 1000, 1000, 10, 10, eVertexType::POSITION | eVertexType::COLOR);
	m_ground.m_lineColor = Vector4(0.7f, 0.7f, 0.7f, 0.7f);

	m_textMgr.Create(1024, 512);
	m_textMgr2.Create(1024, 512);
	m_textMgr3.Create(128, 512);

	return true;
}


void cPathMapView::OnUpdate(const float deltaSeconds)
{
}


void cPathMapView::OnPreRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();
	cAutoCam cam(&m_camera);

	renderer.UnbindTextureAll();

	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);

	if (m_renderTarget.Begin(renderer, Vector4(70.f / 255.f, 70.f / 255.f, 70.f / 255.f, 1.0f)))
	{
		RenderPathEdit(renderer);
		RenderNaviEdit(renderer);

		if (m_showGround)
			m_ground.Render(renderer);
		if (m_showAxis)
			renderer.RenderAxis();
	}
	m_renderTarget.End(renderer);
}


void cPathMapView::RenderPathEdit(graphic::cRenderer &renderer
	, const XMMATRIX &tm//=graphic::XMIdentity
)
{
	cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
	cNaviEdit &naviEdit = g_root.m_terrainEditWindow->m_naviEdit;
	cPathEditWindow *pathEditWnd = g_root.m_pathEditWindow;
	RET(!pathEdit.m_isShowWorldPath);

	cFrustum frustum;
	frustum.SetFrustum(GetMainCamera().GetViewProjectionMatrix());

	renderer.m_dbgBox.SetColor(cColor::WHITE);
	renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(1.f, 1.f, 1.f, 1.f);

	// Render Path Vertex, Edge
	m_textMgr.NewFrame();
	for (uint i=0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
	{
		auto &vtx = pathEdit.m_pathFinder.m_vertices[i];
		const cBoundingBox bbox(vtx.pos + Vector3(0, pathEdit.m_offsetY, 0)
			, Vector3(1, 1, 1) * pathEdit.m_vertexSize
			, Quaternion());

		renderer.m_dbgBox.SetBox(bbox);
		renderer.m_dbgBox.Render(renderer);

		cBoundingSphere bsphere;
		bsphere.SetBoundingSphere(bbox);
		if (m_showVertexId && frustum.IsInSphere(bsphere))
		{
			Transform tfm;
			tfm.pos = vtx.pos;
			tfm.pos.y += 2.f;
			tfm.scale = Vector3(1, 1, 1)*.5f;
			WStrId strId;
			strId.Format(L"%d", i);
			m_textMgr.AddTextRender(renderer, 1000+i, strId.c_str()
				, cColor(1.f, 1.f, 1.f)
				, cColor(0.f, 0.f, 0.f)
				, BILLBOARD_TYPE::ALL_AXIS
				, tfm);
		}

		// special render
		//if ((1 <= vtx.type) && (vtx.edge[0].to >= 0))
		//{
		//	ai::cPathFinder::sVertex &to = pathEdit.m_pathFinder.m_vertices[vtx.edge[0].to];
		//	const Vector3 dir = (to.pos - vtx.pos).Normal();

		//	renderer.m_dbgArrow.SetDirection(vtx.pos + Vector3(0, 1, 0)
		//		, vtx.pos + dir + Vector3(0, 1, 0)
		//		, 0.1f
		//	);
		//	renderer.m_dbgArrow.Render(renderer);
		//}
	}
	m_textMgr.Render(renderer);
	pathEdit.m_lineList.Render(renderer);

	// Render PathEdge Weight
	m_textMgr2.NewFrame();
	if (m_showEdgeWeight)
	{
		set<int> edgeKeys;
		for (uint i = 0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
		{
			auto &vertex = pathEdit.m_pathFinder.m_vertices[i];

			for (int k = 0; i < ai::cPathFinder::sVertex::MAX_EDGE; ++k)
			{
				if (vertex.edge[k].to < 0)
					break;

				const int from = (const int)i;
				const int to = vertex.edge[k].to;
				auto &vtx1 = vertex;
				auto &vtx2 = pathEdit.m_pathFinder.m_vertices[to];
				const Vector3 center = (vtx1.pos + vtx2.pos) * 0.5f;

				const int edgeKey = ai::cPathFinder::MakeEdgeKey(from, to);
				auto it = pathEdit.m_pathFinder.m_lenSet.find(edgeKey);
				if (pathEdit.m_pathFinder.m_lenSet.end() == it)
					continue;

				const float val = it->second;

				Transform tfm;
				tfm.pos = center;
				tfm.pos.y += 2.f;
				tfm.scale = Vector3(1, 1, 1)*0.5f;
				WStrId strId;
				strId.Format(L"%.3f", val);
				m_textMgr2.AddTextRender(renderer, from*ai::cPathFinder::sVertex::MAX_VERTEX + to, strId.c_str()
					, cColor(1.f, 0.f, 0.f)
					, cColor(0.f, 0.f, 0.f)
					, BILLBOARD_TYPE::ALL_AXIS
					, tfm);
			}
		}
	}
	m_textMgr2.Render(renderer);

	// Render NavigationNode Id
	const Ray ray = GetMainCamera().GetRay(m_mousePos.x, m_mousePos.y);
	Vector3 pickPos = m_groundPlane1.Pick(ray.orig, ray.dir);
	Vector3 dir = pickPos - pathEditWnd->m_startPos;
	dir.y = 0;
	dir.Normalize();
	const Vector3 orig = pathEditWnd->m_startPos + Vector3(0,1,0);

	m_textMgr3.NewFrame();
	if (m_showNavigationNode)
	{
		if (cPathEditWindow::eNaviType::NAVIMESH == pathEditWnd->m_naviType)
		{
			for (uint i=0; i < naviEdit.m_naviMesh.m_naviNodes.size(); ++i)
			{
				auto &node = naviEdit.m_naviMesh.m_naviNodes[i];
				const Vector3 center = node.center;

				Transform tfm;
				tfm.pos = center;
				tfm.pos.y += 1.f;
				tfm.scale = Vector3(1, 1, 1)*0.3f;
				WStrId strId;
				strId.Format(L"%d", i);
				m_textMgr3.AddTextRender(renderer, i, strId.c_str()
					, cColor(0.f, 1.f, 0.f)
					, cColor(0.f, 0.f, 0.f)
					, BILLBOARD_TYPE::ALL_AXIS
					, tfm);

			}
		}
	}
	m_textMgr3.Render(renderer);

	

	// Render Starting, Ending Point
	renderer.m_dbgSphere.m_color = cColor::RED;
	renderer.m_dbgSphere.SetPos(pathEditWnd->m_startPos);
	renderer.m_dbgSphere.SetRadius(0.2f);
	renderer.m_dbgSphere.Render(renderer);

	renderer.m_dbgSphere.m_color = cColor::BLUE;
	renderer.m_dbgSphere.SetPos(pathEditWnd->m_endPos);
	renderer.m_dbgSphere.Render(renderer);

	// Render Find Path
	pathEditWnd->m_pathLines1.Render(renderer);
	pathEditWnd->m_pathLines2.Render(renderer);
}


void cPathMapView::RenderNaviEdit(cRenderer &renderer
	, const XMMATRIX &tm //= graphic::XMIdentity
)
{
	cNaviEdit &naviEdit = g_root.m_terrainEditWindow->m_naviEdit;
	if (!naviEdit.m_showNaviMesh)
		return;
	naviEdit.RenderNavigationMesh(renderer);
}


void cPathMapView::OnRender(const float deltaSeconds)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_viewPos = { (int)(pos.x), (int)(pos.y) };
	m_viewRect = { pos.x + 5, pos.y, pos.x + m_rect.Width() - 30, pos.y + m_rect.Height() - 50 };
	ImGui::Image(m_renderTarget.m_resolvedSRV, ImVec2(m_rect.Width() - 15, m_rect.Height() - 50));

	// HUD
	const float windowAlpha = 0.0f;
	bool isOpen = true;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(ImVec2(min(m_viewRect.Width(), 500.f), 120.f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	if (ImGui::Begin("Information", &isOpen, ImVec2(500.f, 120.f), windowAlpha, flags))
	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Checkbox("Show Axis", &m_showAxis);
		ImGui::SameLine();
		ImGui::Checkbox("Show Ground", &m_showGround);
		ImGui::Checkbox("Show VertexId", &m_showVertexId);
		ImGui::SameLine();
		ImGui::Checkbox("Show Edge Weight", &m_showEdgeWeight);
		ImGui::SameLine();
		ImGui::Checkbox("Show Navigation Node", &m_showNavigationNode);

		if (ImGui::Button("Center Camera"))
		{
			m_camera.SetCamera(Vector3(-100, 80, -100), Vector3(0, 0, 0), Vector3(0, 1, 0));
		}

		ImGui::End();
	}
	ImGui::PopStyleColor();
}


void cPathMapView::OnResizeEnd(const framework::eDockResize::Enum type, const sRectf &rect)
{
	if (type == eDockResize::DOCK_WINDOW)
	{
		m_owner->RequestResetDeviceNextFrame();
	}
}


void cPathMapView::UpdateLookAt()
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
void cPathMapView::OnWheelMove(const float delta, const POINT mousePt)
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
void cPathMapView::OnMouseMove(const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	cPathEditWindow *pathEditWnd = g_root.m_pathEditWindow;
	cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;

	// Update Start, End Point 
	const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
	if (cPathEditWindow::eState::SET_STARTPOS == pathEditWnd->m_state)
	{
		const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
		pathEditWnd->m_startPos = pos;
	}
	else if (cPathEditWindow::eState::SET_ENDPOS == pathEditWnd->m_state)
	{
		const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
		pathEditWnd->m_endPos = pos;
	}

	if (m_mouseDown[0])
	{
		if ((pathEdit.m_isEditVehiclePath)
			&& (eEditVehicle::Edge == pathEdit.m_editVehicleType)
			&& GetAsyncKeyState(VK_LCONTROL))
		{
			if (pathEdit.m_selectVertex >= 0)
			{
				const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
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
		m_camera.Yaw2(delta.x * 0.005f, Vector3(0, 1, 0));
		m_camera.Pitch2(delta.y * 0.005f, Vector3(0, 1, 0));

		//GetMainCamera().Yaw2(delta.x * 0.005f, Vector3(0, 1, 0));
		//GetMainCamera().Pitch2(delta.y * 0.005f, Vector3(0, 1, 0));
	}
	else if (m_mouseDown[2])
	{
		const float len = GetMainCamera().GetDistance();
		GetMainCamera().MoveRight(-delta.x * len * 0.001f);
		GetMainCamera().MoveUp(delta.y * len * 0.001f);
	}
}


// Handling Mouse Button Down Event
void cPathMapView::OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt)
{
	m_mousePos = mousePt;
	UpdateLookAt();
	SetCapture();

	switch (button)
	{
	case sf::Mouse::Left:
	{
		m_mouseDown[0] = true;

		// Vehicle Edit Mode
		cPathEditWindow *pathEditWnd = g_root.m_pathEditWindow;
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);

		if (cPathEditWindow::eState::SET_STARTPOS == pathEditWnd->m_state)
		{
			const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
			pathEditWnd->m_startPos = pos;
			pathEditWnd->m_state = cPathEditWindow::eState::NORMAL;
		}
		else if (cPathEditWindow::eState::SET_ENDPOS == pathEditWnd->m_state)
		{
			const Vector3 pos = m_groundPlane1.Pick(ray.orig, ray.dir);
			pathEditWnd->m_endPos = pos;
			pathEditWnd->m_state = cPathEditWindow::eState::NORMAL;
		}
		else if (pathEdit.m_isEditVehiclePath)
		{
			if (eEditVehicle::Vertex == pathEdit.m_editVehicleType)
			{
				ai::cPathFinder::sVertex vtx;
				vtx.pos = m_groundPlane1.Pick(ray.orig, ray.dir);
				vtx.pos.y = g_root.m_terrain.GetHeight(vtx.pos.x, vtx.pos.z);
				pathEdit.m_pathFinder.AddVertex(vtx);
				pathEdit.UpdateLineList(GetRenderer());
			}
			else if (eEditVehicle::Edge == pathEdit.m_editVehicleType)
			{
				int vtxIdx = -1;
				for (uint i = 0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
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
			}
		}
		//

		Vector3 p1 = m_groundPlane1.Pick(ray.orig, ray.dir);
		m_rotateLen = min(500.f, (p1 - ray.orig).Length());
	}
	break;

	case sf::Mouse::Right:
	{
		m_mouseDown[1] = true;

		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);

		// Vehicle Edit Mode
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		if (pathEdit.m_isEditVehiclePath)
		{

			if ((eEditVehicle::Edge == pathEdit.m_editVehicleType)
				&& (pathEdit.m_selectVertex >= 0))
			{
				int addVtxIdx = -1;
				for (uint i = 0; i < pathEdit.m_pathFinder.m_vertices.size(); ++i)
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
		break;
	}
}


void cPathMapView::OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;
	ReleaseCapture();

	switch (button)
	{
	case sf::Mouse::Left:
		m_mouseDown[0] = false;
		break;
	case sf::Mouse::Right:
		m_mouseDown[1] = false;
		break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = false;
		break;
	}
}


void cPathMapView::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.cmd)
		{
		case sf::Keyboard::Return:
			break;

		case sf::Keyboard::Space:
			break;

		case sf::Keyboard::Left: m_camera.MoveRight(-0.5f); break;
		case sf::Keyboard::Right: m_camera.MoveRight(0.5f); break;
		case sf::Keyboard::Up: m_camera.MoveUp(0.5f); break;
		case sf::Keyboard::Down: m_camera.MoveUp(-0.5f); break;
		}
		break;

	case sf::Event::MouseMoved:
	{
		cAutoCam cam(&m_camera);

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
		cAutoCam cam(&m_camera);

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
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnWheelMove(evt.mouseWheelScroll.delta, pos);
	}
	break;
	}
}


void cPathMapView::OnResetDevice()
{
	cRenderer &renderer = GetRenderer();

	// update viewport
	sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
	m_camera.SetViewPort(viewRect.Width(), viewRect.Height());

	cViewport vp = GetRenderer().m_viewPort;
	vp.m_vp.Width = viewRect.Width();
	vp.m_vp.Height = viewRect.Height();
	m_renderTarget.Create(renderer, vp);
}

