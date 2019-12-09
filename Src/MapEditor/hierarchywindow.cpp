
#include "stdafx.h"
#include "hierarchywindow.h"


using namespace graphic;

cHierarchyWindow::cHierarchyWindow()
	: cDockWindow("Hierarchy Window")
{
}

cHierarchyWindow::~cHierarchyWindow()
{
	Clear();
}


void cHierarchyWindow::OnUpdate(const float deltaSeconds)
{
}


void cHierarchyWindow::OnRender(const float deltaSeconds)
{
	static bool showAllHierarchy = false;
	ImGui::Checkbox("Show All", &showAllHierarchy);
	ImGui::Separator();

	ImGui::Spacing();

	if (ImGui::Button(" + Add Model"))
		AddModel();

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Always);
	if (showAllHierarchy)
		RenderAllHierarchy();
	else
		RenderModelHierarchy();
}


// show all terrain model
void cHierarchyWindow::RenderAllHierarchy()
{
	if (ImGui::TreeNode((void*)0, "Hierarchy"))
	{
		ImGui::SameLine();
		const bool isExpandTree = ImGui::SmallButton("- Expand Tree");

		bool isClickItem = false;
		static int selectId = -1;
		for (auto &tile : g_root.m_terrain.m_children) // parents node
		{
			if (isExpandTree)
			{
				ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
				ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Always);
			}

			if (ImGui::TreeNode((void*)tile->m_id, tile->m_name.utf8().c_str()))
			{
				for (auto *cnode : tile->m_children)
				{
					const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow
						| ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Leaf
						| ImGuiTreeNodeFlags_NoTreePushOnOpen
						| ((cnode->m_id == selectId) ? ImGuiTreeNodeFlags_Selected : 0);

					ImGui::TreeNodeEx((void*)(intptr_t)cnode->m_id, node_flags,
						cnode->m_name.utf8().c_str());

					if (!isClickItem && ImGui::IsItemClicked())
					{
						isClickItem = true;
						selectId = cnode->m_id;
						g_root.m_selectModel = cnode;
						g_root.m_gizmo->SetControlNode(cnode);

						if (cTile *p = dynamic_cast<cTile*>(tile))
							g_root.m_terrainEditWindow->m_gridEdit.SelectTile(p);
					}
				}

				ImGui::TreePop();
			}

			if (!isClickItem && ImGui::IsItemClicked())
			{
				isClickItem = true;
				selectId = tile->m_id;
				g_root.m_selectModel = tile;
				g_root.m_gizmo->SetControlNode(NULL);
				if (cTile *p = dynamic_cast<cTile*>(tile))
					g_root.m_terrainEditWindow->m_gridEdit.SelectTile(p);
			}
		}
		ImGui::TreePop();
	}
}


// show model only
void cHierarchyWindow::RenderModelHierarchy()
{
	if (ImGui::TreeNode((void*)0, "Hierarchy"))
	{
		bool isClickItem = false;
		static int selectId = -1;
		for (auto &tile : g_root.m_terrain.m_children) // parents node
		{
			for (auto *cnode : tile->m_children)
			{
				if (cnode->m_name == "Ground")
					continue; // ignore grid model

				const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_Leaf
					| ImGuiTreeNodeFlags_NoTreePushOnOpen
					| ((cnode->m_id == selectId) ? ImGuiTreeNodeFlags_Selected : 0);

				ImGui::TreeNodeEx((void*)(intptr_t)cnode->m_id, node_flags,
					cnode->m_name.utf8().c_str());

				if (!isClickItem && ImGui::IsItemClicked())
				{
					isClickItem = true;
					selectId = cnode->m_id;
					g_root.m_selectModel = cnode;
					g_root.m_gizmo->SetControlNode(cnode);

					if (cTile *p = dynamic_cast<cTile*>(tile))
						g_root.m_terrainEditWindow->m_gridEdit.SelectTile(p);
				}
			}
		}
		ImGui::TreePop();
	}
}


void cHierarchyWindow::Clear()
{
}


bool cHierarchyWindow::AddModel()
{
	if (!g_root.IsModelExtendsName(g_root.m_resWindow->m_selectPath.c_str())) // check ext
	{
		// error
		::MessageBoxA(g_root.m_hWnd, "No Select Model in Resource Window", "Error", MB_OK);
		return false;
	}
	else
	{
		cRenderer &renderer = GetRenderer();

		cModel *model = new cModel();
		model->Create(renderer, common::GenerateId()
			, g_root.m_resWindow->m_selectPath.c_str(), true);

		AddModel(model);
	}

	return true;
}


bool cHierarchyWindow::AddModel(cNode *node)
{
	g_root.m_selectModel = node;
	g_root.m_gizmo->SetControlNode(node);
	g_root.m_mapView->m_pickMgr.Add(node);
	g_root.m_mapView->m_pickMgr.m_offset = { -10, -40 };

	assert(!g_root.m_terrain.m_tiles.empty());

	const Ray ray = g_root.m_camWorld.GetRay();
	Plane ground(Vector3(0, 1, 0), 0);
	const Vector3 pos = ground.Pick(ray.orig, ray.dir);
	node->m_transform.pos = pos;
	node->m_transform.pos.y = g_root.m_terrain.GetHeight(pos.x, pos.z);
	g_root.m_terrain.AddModel(node);

	// 같은 메쉬파일을 쓰는 모델이 있다면, Scale, Rotation 을 동일하게 한다.
	// 동일한 모델이 많다면, 가장 가까운 대상의 모델을 기준으로 한다.
	// todo: 현재 모델의 이름으로 검색하고 있는데, 메쉬파일을 기준으로 검색해야한다.
	if (cModel *model = dynamic_cast<cModel*>(node))
	{
		vector<cNode*> candidates;
		g_root.m_terrain.FindNodeAll(node->m_name, candidates);

		cModel *mostNearModel = NULL;
		float nearLen = FLT_MAX;
		const Vector3 pos = model->GetWorldTransform().pos;
		for (auto &p : candidates)
		{
			cModel *mod = dynamic_cast<cModel*>(p);
			if (!mod)
				continue;
			if (node == p)
				continue;

			const float len = mod->GetWorldTransform().pos.LengthRoughly(pos);
			if (nearLen > len)
			{
				nearLen = len;
				mostNearModel = mod;
			}
		}

		if (mostNearModel)
		{
			node->m_transform.scale = mostNearModel->m_transform.scale;
			node->m_transform.rot = mostNearModel->m_transform.rot;
		}
	}

	return true;
}
