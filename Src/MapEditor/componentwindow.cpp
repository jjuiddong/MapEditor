
#include "stdafx.h"
#include "componentwindow.h"


using namespace graphic;

cComponentWindow::cComponentWindow()
	: cDockWindow("Component Window")
{
}

cComponentWindow::~cComponentWindow()
{
}


bool cComponentWindow::Init()
{
	return true;
}


void cComponentWindow::OnUpdate(const float deltaSeconds)
{
}


void cComponentWindow::OnRender(const float deltaSeconds)
{
	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	if (ImGui::TreeNode((void*)0, "Component List"))
	{
		cRenderer &renderer = GetRenderer();

		ImGui::Spacing();

		const ImVec2 btnSize(80, 0);
		if (ImGui::Button("Cube", btnSize))
		{
			cCube *cube = new cCube();
			cube->m_name = "Cube";
			cube->Create(renderer, cBoundingBox());
			g_root.m_hierarchyWindow->AddModel(cube);
		}

		ImGui::SameLine();
		if (ImGui::Button("Sphere", btnSize))
		{
			cSphere *sphere = new cSphere();
			sphere->Create(renderer, 1, 10, 10);
			g_root.m_hierarchyWindow->AddModel(sphere);
		}

		ImGui::SameLine();
		if (eMapEditType::LINE == g_root.m_mapEditType)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(g_col_button.x, g_col_button.y, g_col_button.z, 1));
			if (ImGui::Button("Line", btnSize))
			{
				g_root.m_mapEditType = eMapEditType::NORMAL;
			}
			ImGui::PopStyleColor();
		}
		else
		{
			if (ImGui::Button("Line", btnSize))
			{
				g_root.m_mapEditType = eMapEditType::LINE;
			}
		}

		ImGui::SameLine();
		if (ImGui::Button("Arrow", btnSize))
		{
			//cDbgArrow *arrow = new cDbgArrow();
			//arrow->Create(renderer, )
		}

		ImGui::SameLine();
		if (ImGui::Button("Torus", btnSize))
		{
			cTorus *torus = new cTorus();
			torus->Create(renderer, 2, 1, 20, 20, eVertexType::POSITION | eVertexType::NORMAL);
			g_root.m_hierarchyWindow->AddModel(torus);
		}


		ImGui::SameLine();
		if (ImGui::Button("Pyramid", btnSize))
		{
			cPyramid *pyramid = new cPyramid();
			pyramid->Create(renderer);
			pyramid->m_color = cColor::WHITE;
			g_root.m_hierarchyWindow->AddModel(pyramid);
		}


		ImGui::SameLine();
		ImGui::Button("Quad", btnSize);

		ImGui::SameLine();
		ImGui::Button("Circle", btnSize);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("CCTV", btnSize))
		{
			cCCtv *cctv = new cCCtv();
			cctv->Create(renderer);
			if (g_root.m_hierarchyWindow->AddModel(cctv))
				cctv->m_transform.pos.y += 10.f;
		}

		ImGui::SameLine();
		if (eMapEditType::AREA == g_root.m_mapEditType)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(g_col_button.x, g_col_button.y, g_col_button.z, 1));
			if (ImGui::Button("Area", btnSize))
			{
				g_root.m_mapEditType = eMapEditType::NORMAL;
			}
			ImGui::PopStyleColor();
		}
		else
		{
			if (ImGui::Button("Area", btnSize))
			{
				g_root.m_mapEditType = eMapEditType::AREA;
			}
		}

		ImGui::TreePop();
	}
}
