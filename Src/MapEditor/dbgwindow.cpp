
#include "stdafx.h"
#include "dbgwindow.h"

using namespace graphic;


cDbgWindow::cDbgWindow(const string &name //= ""
)
	: framework::cDockWindow(name)
	, m_isShowFrustum(false)
	, m_isShowAxis(false)
	, m_isShowBuilding(true)
	, m_isShowTileMap(true)
	, m_isUpdateFrustum(true)
	, m_isObserverCamera(false)
	, m_isShowCascadedFrustum(false)
	, m_isShowLightFrustum(false)
	, m_isTerrainDebugDisplay(false)
	, m_isShadow(true)
	, m_showCursor(false)
	, m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_groundPlane2(Vector3(0, -1, 0), 0)
	, m_isShowBoxTexColor(false)
	, m_isShowPathMesh(true)
	, m_isShowRackDirection(false)
{
}

cDbgWindow::~cDbgWindow()
{
}


void cDbgWindow::OnRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();

	POINT pt = GetInput().m_mousePt;
	const Ray ray = g_root.m_camWorld.GetRay(pt.x, pt.y);
	m_cursorPos = m_groundPlane1.Pick(ray.orig, ray.dir);
	m_cursorPos.y = g_root.m_terrain.GetHeight(m_cursorPos.x, m_cursorPos.z);

	m_cameraPos = g_root.m_camWorld.GetEyePos();
	m_lookAtPos = g_root.m_camWorld.GetLookAt();

	ImGui::InputFloat3("Cursor Pos", (float*)&m_cursorPos);

	//ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	if (ImGui::CollapsingHeader("Display"))
	{
		ImGui::Checkbox("Show WireFrame", &m_isShowWireFrame);
		ImGui::Checkbox("Show Builing", &m_isShowBuilding);
		ImGui::Checkbox("Show TileMap", &m_isShowTileMap);
		ImGui::Checkbox("Show Axis", &m_isShowAxis);
		if (ImGui::Checkbox("Terrain Debug Display", &m_isTerrainDebugDisplay))
		{
			renderer.m_isDbgRender = m_isTerrainDebugDisplay;
		}
		if (m_isTerrainDebugDisplay)
		{
			ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("Sphere", &renderer.m_dbgRenderStyle, 0);
			ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("Box", &renderer.m_dbgRenderStyle, 1);
			ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("None", &renderer.m_dbgRenderStyle, 2);
		}

		if (ImGui::Checkbox("Show Shadow", &m_isShadow))
			g_root.m_terrain.SetShadowRendering(m_isShadow);
		ImGui::Checkbox("Show Cursor", &m_showCursor);
		ImGui::Checkbox("Show BoxTexColor", &m_isShowBoxTexColor);
		ImGui::Checkbox("Show PathMesh", &m_isShowPathMesh);
		ImGui::Checkbox("Show Rack Direction", &m_isShowRackDirection);
		//ImGui::Checkbox("Show World Path", &m_isShowWorldPath);
		ImGui::Checkbox("Show Vehicle Path", &m_isShowVehiclePath);
	}

	//ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::InputFloat3("Frustom Pos", (float*)&m_frustumPos);
		ImGui::InputFloat3("LookAt Pos", (float*)&m_lookAtPos);
		ImGui::InputFloat3("Camera Pos", (float*)&m_cameraPos);
		ImGui::Checkbox("Observer Camera", &m_isObserverCamera);
		ImGui::Checkbox("Show Frustum", &m_isShowFrustum);
		ImGui::Checkbox("Show Cascaded Frustum", &m_isShowCascadedFrustum);
		ImGui::Checkbox("Show Light Frustum", &m_isShowLightFrustum);
		ImGui::Checkbox("Update Frustum", &m_isUpdateFrustum);
	}

	if (ImGui::CollapsingHeader("Shader "))
	{
		if (ImGui::Button("Update Shader"))
		{
			renderer.m_shaderMgr.ReloadAll(renderer);
		}
	}

	//ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	//if (ImGui::CollapsingHeader("Vehicle Path"))
	//{
	//	ImGui::Checkbox("Edit Path", &m_isEditVehiclePath);
	//	ImGui::Spacing();
	//	ImGui::Spacing();

	//	ImGui::RadioButton("Edit Vertex", (int*)&m_editVehicleType, eEditVehicle::Vertex);
	//	ImGui::RadioButton("Edit Edge", (int*)&m_editVehicleType, eEditVehicle::Edge);
	//	ImGui::RadioButton("Edit Destination", (int*)&m_editVehicleType, eEditVehicle::Dest);
	//	ImGui::Spacing();
	//	ImGui::Spacing();

	//	if (m_selectVertex >= 0)
	//	{
	//		ImGui::Text("Select Vertex = %d", m_selectVertex);
	//		ImGui::SameLine(150);

	//		const bool isRemove = ImGui::Button("Remove");

	//		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
	//		ai::sVertex &vtx = pathEdit.m_pathFinder.m_vertices[m_selectVertex];

	//		ImGui::InputInt("type", &vtx.type);
	//		ImGui::DragFloat3("pos", (float*)&vtx.pos, 0.01f);
	//		ImGui::Spacing();

	//		for (int i = 0; i < ai::sVertex::MAX_EDGE; ++i)
	//		{
	//			if (vtx.edge[i] < 0)
	//				break;

	//			const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	//			ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
	//				"%d", vtx.edge[i]);

	//			ImGui::SameLine(100);
	//			if (ImGui::Button(format("Remove-%d", i).c_str())) // remove edge  (rotation left)
	//			{
	//				pathEdit.m_pathFinder.RemoveEdgeEachOther(m_selectVertex, vtx.edge[i]);
	//				pathEdit.UpdateLineList();
	//			}
	//		}

	//		if (isRemove)
	//		{
	//			pathEdit.m_pathFinder.RemoveVertex(m_selectVertex);
	//			pathEdit.UpdateLineList();
	//			m_selectVertex = -1;
	//		}
	//	}

	//	if (eEditVehicle::Dest == m_editVehicleType)
	//	{
	//		ImGui::DragFloat3("Start Point (Left Click)", (float*)&m_editVehicleStartPoint, 0.01f);
	//		ImGui::DragFloat3("End Point (Right Click)", (float*)&m_editVehicleEndPoint, 0.01f);
	//	}

	//	//if (ImGui::Button("Update Path Line"))
	//	//	g_root.m_tms.UpdateLineList();

	//	ImGui::Spacing();
	//	ImGui::Spacing();

	//	if (ImGui::Button("Save Path"))
	//	{
	//		//g_root.m_tms.WritePath();
	//	}
	//}

	ImGui::Spacing();
	ImGui::Spacing();
}


void cDbgWindow::OnUpdate(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();
}


void cDbgWindow::OnEventProc(const sf::Event &evt)
{
}
