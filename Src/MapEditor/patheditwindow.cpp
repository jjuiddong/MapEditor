
#include "stdafx.h"
#include "patheditwindow.h"


using namespace graphic;
using namespace framework;

cPathEditWindow::cPathEditWindow()
	: cDockWindow("Path Edit")
	, m_state(eState::NORMAL)
{
}

cPathEditWindow::~cPathEditWindow()
{
}


void cPathEditWindow::OnUpdate(const float deltaSeconds)
{
}


bool cPathEditWindow::Init(cRenderer &renderer)
{
	m_pathLines1.Create(renderer, 256, cColor::YELLOW);
	m_pathLines2.Create(renderer, 256, cColor::RED);
	return true;
}


void cPathEditWindow::OnRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();

	cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
	cNaviEdit &naviEdit = g_root.m_terrainEditWindow->m_naviEdit;

	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
	pathEdit.Render(renderer);
	naviEdit.Render(renderer);

	ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
	if (ImGui::CollapsingHeader("Test PathFind"))
	{
		ImGui::Spacing();

		ImGui::RadioButton("AStar", (int*)&m_naviType, eNaviType::ASTAR);
		ImGui::SameLine();
		ImGui::RadioButton("Navigation Mesh", (int*)&m_naviType, eNaviType::NAVIMESH);

		ImGui::Spacing();

		if (ImGui::Button("Start Point", ImVec2(100,0)))
		{
			m_state = eState::SET_STARTPOS;
		}

		ImGui::SameLine();
		ImGui::DragFloat3("Pos1", (float*)&m_startPos, 0.1f);

		if (ImGui::Button("End Point", ImVec2(100, 0)))
		{
			m_state = eState::SET_ENDPOS;
		}

		ImGui::SameLine();
		ImGui::DragFloat3("Pos2", (float*)&m_endPos, 0.1f);

		if (ImGui::Button("Find Path", ImVec2(100, 0)))
		{
			PathFind(renderer);
		}
	}
}


void cPathEditWindow::PathFind(cRenderer &renderer)
{
	m_path1.clear();
	//m_path2.clear();
	m_nodePath.clear();
	m_pathLines1.ClearLines();
	m_pathLines2.ClearLines();

	if (eNaviType::ASTAR == m_naviType)
	{
		cPathEdit &pathEdit = g_root.m_terrainEditWindow->m_pathEdit;
		pathEdit.m_pathFinder.Find(m_startPos, m_endPos, m_path1);

		if (!m_path1.empty())
		{
			Vector3 offset(0, 1, 0);
			for (u_int i = 1; i < m_path1.size(); ++i)
				m_pathLines1.AddLine(renderer, m_path1[i - 1] + offset, m_path1[i] + offset, false);
			m_pathLines1.UpdateBuffer(renderer);
		}
	}
	else if (eNaviType::NAVIMESH == m_naviType)
	{
		cNaviEdit &naviEdit = g_root.m_terrainEditWindow->m_naviEdit;
		naviEdit.m_naviMesh.Find(m_startPos, m_endPos, m_path1, &m_nodePath);

		if (!m_path1.empty())
		{
			Vector3 offset(0, 1, 0);
			for (u_int i = 1; i < m_path1.size(); ++i)
				m_pathLines1.AddLine(renderer, m_path1[i - 1] + offset, m_path1[i] + offset, false);
			m_pathLines1.UpdateBuffer(renderer);
		}

		//if (!m_path2.empty())
		//{
		//	Vector3 offset(0, 1, 0);
		//	for (u_int i = 1; i < m_path2.size(); ++i)
		//		m_pathLines2.AddLine(renderer, m_path2[i - 1] + offset, m_path2[i] + offset, false);
		//	m_pathLines2.UpdateBuffer(renderer);
		//}
	}
	else
	{
		assert(0);
	}
}
