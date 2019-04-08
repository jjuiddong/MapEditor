
#include "stdafx.h"
#include "resourcewindow.h"

using namespace graphic;
using namespace framework;


cResourceWindow::cResourceWindow()
	: cDockWindow("Resource Window")
{
}

cResourceWindow::~cResourceWindow()
{
}


bool cResourceWindow::Init()
{
	UpdateResourceFile();
	return true;
}


void cResourceWindow::OnUpdate(const float deltaSeconds)
{
}


void cResourceWindow::UpdateResourceFile()
{
	{
		m_textureFilesUTF8.clear();

		vector<WStr32> exts;
		exts.reserve(16);
		exts.push_back(L".bmp"); exts.push_back(L".BMP");
		exts.push_back(L".jpg"); exts.push_back(L".JPG");
		exts.push_back(L".tga"); exts.push_back(L".TGA");
		exts.push_back(L".png"); exts.push_back(L".PNG");
		exts.push_back(L".dds"); exts.push_back(L".DDS");
		vector<WStr64> ignores;
		ignores.push_back(L"VWorld");
		ignores.push_back(L"WorldTerrain");
		vector<WStrPath> out;
		out.reserve(256);
		common::CollectFiles3(exts, L"../media/", ignores, out);

		m_textureFilesUTF8.reserve(256);
		for (auto &str : out)
			m_textureFilesUTF8.push_back(str.utf8());
	}

	{
		m_modelFilesUTF8.clear();

		vector<WStr32> exts;
		exts.reserve(8);
		for (auto &ext : g_root.m_modelExts)
			exts.push_back(str2wstr(ext));

		vector<WStr64> ignores;
		ignores.push_back(L"VWorld");
		ignores.push_back(L"WorldTerrain");
		vector<WStrPath> out;
		out.reserve(256);
		common::CollectFiles3(exts, L"../media/", ignores, out);

		m_modelFilesUTF8.reserve(256);
		for (auto &str : out)
			m_modelFilesUTF8.push_back(str.utf8());
	}
}


void cResourceWindow::OnRender(const float deltaSeconds)
{
	static ImGuiTextFilter filter;
	filter.Draw("Search");

	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	if (ImGui::TreeNode((void*)0, "Resource Tree"))
	{
		ImGui::SameLine(150);
		if (ImGui::SmallButton("Refresh"))
			UpdateResourceFile();

		ImGui::Separator();

		static int selectIdx = -1;
		int i = 0;
		bool isOpenPopup = false;

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
		if (ImGui::TreeNode((void*)1, "Model"))
		{
			ImGui::Columns(5, "modelcolumns5", false);
			for (auto &str : m_modelFilesUTF8)
			{
				const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ((i == selectIdx) ? ImGuiTreeNodeFlags_Selected : 0);

				if (filter.PassFilter(str.c_str()))
				{
					ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
						str.c_str());

					if (ImGui::IsItemClicked() || ImGui::IsItemClicked(1))
					{
						selectIdx = i;
						m_selectPath = str.ansi(); // change UTF8 -> UTF16
						g_root.m_resViewer->LoadResource(m_selectPath.c_str());

						// Popup Menu
						if (ImGui::IsItemClicked(1))
						{
							isOpenPopup = true;
							//ImGui::OpenPopup("select");
						}
					}

					ImGui::NextColumn();
				}

				++i;
			}
			ImGui::TreePop();
		}

		ImGui::Columns(1); // columns initialize
		ImGui::Separator();

		ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
		if (ImGui::TreeNode((void*)0, "Texture"))
		{
			ImGui::Columns(5, "texturecolumns5", false);
			for (auto &str : m_textureFilesUTF8)
			{
				const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ((i == selectIdx) ? ImGuiTreeNodeFlags_Selected : 0);

				if (filter.PassFilter(str.c_str()))
				{
					ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
						, str.c_str());

					if (ImGui::IsItemClicked())
					{
						selectIdx = i;
						m_selectPath = str.ansi(); // change UTF8 -> Ansi
						g_root.m_resViewer->LoadResource(m_selectPath.c_str());
					}

					ImGui::NextColumn();
				}

				++i;
			}
			ImGui::TreePop();
		}

		//---------------------------------------------------------------------------
		// Popup Menu
		if (isOpenPopup)
			ImGui::OpenPopup("select");

		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));
		if (ImGui::BeginPopup("select"))
		{
			if (ImGui::Selectable("Add Model      "))
				g_root.m_hierarchyWindow->AddModel();
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();
		//---------------------------------------------------------------------------

		ImGui::TreePop();
	}

}
