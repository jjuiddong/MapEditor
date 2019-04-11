
#include "stdafx.h"
#include "terraineditwindow.h"
#include "terrainloader2.h"


using namespace graphic;
using namespace framework;

cTerrainEditWindow::cTerrainEditWindow()
	: cDockWindow("Terrain Edit")
	, m_readFileName("gunpo.trn")
	, m_writeFileName("")
	, m_isShowWriteButton(true)
{
}

cTerrainEditWindow::~cTerrainEditWindow()
{
}


void cTerrainEditWindow::OnUpdate(const float deltaSeconds)
{
}


bool cTerrainEditWindow::Init(cRenderer &renderer)
{
	m_pathEdit.Init(renderer);
	m_gridEdit.Init(renderer);
	m_geoEdit.Init(renderer);
	m_lightEdit.Init(renderer);
	m_naviEdit.Init(renderer);

	m_readFileName = g_root.m_cfg.GetString("ReadFile", "gunpo.trn");
	m_writeFileName = g_root.m_cfg.GetString("WriteFile", "");
	return true;
}


void cTerrainEditWindow::OnRender(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();

	ImGui::PushID(0); ImGui::InputText("", m_writeFileName.m_str, ARRAYSIZE(m_writeFileName.m_str)); ImGui::PopID();
	if (m_isShowWriteButton)
	{
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			cTerrainLoader2 loader(&g_root.m_terrain);
			loader.Write(m_writeFileName);
			g_root.m_cfg.SetValue("WriteFile", m_writeFileName.c_str());
		}
	}

	ImGui::PushID(1); ImGui::InputText("", m_readFileName.m_str, ARRAYSIZE(m_readFileName.m_str)); ImGui::PopID();
	ImGui::SameLine();
	if (ImGui::Button("Read"))
	{
		ReadTerrainFile(renderer, m_readFileName);
	}

	ImGui::SameLine();
	if (ImGui::Button("..."))
	{
		StrPath path = OpenFileDialog();
		if (!path.empty())
			if (ReadTerrainFile(renderer, path))
				m_readFileName = path;
	}

	m_gridEdit.Render(renderer);
	m_geoEdit.Render(renderer);
	m_pathEdit.Render(renderer);
	m_lightEdit.Render(renderer);
	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	m_modelEdit.Render();
	m_naviEdit.Render(renderer);
}


void cTerrainEditWindow::OnEventProc(const sf::Event &evt)
{
}


bool cTerrainEditWindow::ReadTerrainFile( graphic::cRenderer &renderer, const StrPath &path )
{
	m_isShowWriteButton = true;
	m_writeFileName = path;
	g_root.m_cfg.SetValue("ReadFile", path.c_str());

	cTerrainLoader2 loader(&g_root.m_terrain);
	loader.Read(renderer, path);

	g_root.m_selectTile = NULL;
	g_root.m_selectModel = NULL;
	g_root.m_gizmo->Cancel();
	g_root.m_terrainEditWindow->m_gridEdit.m_row = g_root.m_terrain.m_tileRows;
	g_root.m_terrainEditWindow->m_gridEdit.m_col = g_root.m_terrain.m_tileCols;
	g_root.m_terrainEditWindow->m_gridEdit.m_gridSelect.resize(g_root.m_terrain.m_rows*g_root.m_terrain.m_cols);
	g_root.m_terrainEditWindow->m_gridEdit.m_selectTileIdx = -1;
	g_root.m_terrainEditWindow->m_gridEdit.m_tiles.clear();
	if (g_root.m_terrain.m_tiles.empty())
	{
		g_root.m_terrainEditWindow->m_gridEdit.m_tileSize = Vector2(100, 100);
	}
	else
	{
		const Vector3 dim = g_root.m_terrain.m_tiles.front()->m_boundingBox.GetDimension();
		g_root.m_terrainEditWindow->m_gridEdit.m_tileSize = Vector2(dim.x, dim.z);
	}

	// setting tile indexing
	for (auto &tile : g_root.m_terrain.m_tiles)
	{
		const int tileIdx = tile->m_location.y + tile->m_location.x * g_root.m_terrain.m_tileCols;
		g_root.m_terrainEditWindow->m_gridEdit.m_tiles[tileIdx] = tile;
	}

	// update title window text
	cDockManager::Get()->GetMainWindow()->m_title.Format(
		"- MapEditor - [ %s ]", path.c_str());

	return true;
}


// 파일열기 다이얼로그를 띄운다.
StrPath cTerrainEditWindow::OpenFileDialog()
{
	IFileOpenDialog *pFileOpen;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC filter[] = {
			{L"Terrain File (*.trn)", L"*.trn"}
			, {L"All File (*.*)", L"*.*"}
		};
		pFileOpen->SetFileTypes(ARRAYSIZE(filter), filter);

		hr = pFileOpen->Show(NULL);
		if (SUCCEEDED(hr))
		{
			IShellItem *pItem;
			hr = pFileOpen->GetResult(&pItem);
			if (SUCCEEDED(hr))
			{
				PWSTR pszFilePath;
				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					WStrPath path = pszFilePath;
					CoTaskMemFree(pszFilePath);
					return path.str();
				}
				pItem->Release();
			}
		}
		pFileOpen->Release();
	}

	return "";
}
