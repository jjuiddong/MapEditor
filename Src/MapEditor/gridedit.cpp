
#include "stdafx.h"
#include "gridedit.h"

using namespace graphic;
using namespace framework;


cGridEdit::cGridEdit()
	: m_row(10)
	, m_col(10)
	, m_selectTileIdx(-1)
	, m_tileSize(100, 100)
{
	m_gridSelect.reserve(128);

	m_tileFmtOption.order = 0; // col-row
	m_tileFmtOption.rowInc = 0; // decrement
	m_tileFmtOption.colInc = 1; // increment
}

cGridEdit::~cGridEdit()
{
}


bool cGridEdit::Init(graphic::cRenderer &renderer)
{
	m_row = g_root.m_cfg.GetInt("Grid-Row", 10);
	m_col = g_root.m_cfg.GetInt("Grid-Col", 10);
	//m_tileSize = g_root.m_cfg.GetFloat("Grid-Size", 100);
	m_tileSize.x = max(1.f, g_root.m_cfg.GetFloat("Grid-SizeW", 100));
	m_tileSize.y = max(1.f, g_root.m_cfg.GetFloat("Grid-SizeH", 100));
	m_fileNameFomatter = g_root.m_cfg.GetString("Grid-Fomatter", "filename%d-%d.jpg");

	ChangeGrid(renderer, m_row, m_col);

	//test
	if (1)
	{
		cCube *cube1 = new cCube();
		cube1->m_name = "cube parent";
		cBoundingBox bbox;
		bbox.SetBoundingBox(Vector3(10, 10, 10), Vector3(1, 1, 1), Quaternion());
		cube1->Create(renderer, bbox);
		g_root.m_terrain.m_tiles[0]->AddChild(cube1);

		cCube *cube2 = new cCube();
		cube2->m_name = "cube child";
		cBoundingBox bbox2;
		bbox2.SetBoundingBox(Vector3(0,5,0), Vector3(2, 2, 2), Quaternion());
		cube2->Create(renderer, bbox2);
		cube1->AddChild(cube2);
	}

	return true;
}


void cGridEdit::Render(cRenderer &renderer)
{
	if (ImGui::CollapsingHeader("Grid Edit"))
	{
		if (ImGui::InputInt("Row", &m_row))
		{
			m_row = max(m_row, 1);
			ChangeGrid(renderer, m_row, m_col);
			g_root.m_cfg.SetValue("Grid-Row", m_row);
		}
		if (ImGui::InputInt("Col", &m_col))
		{
			m_col = max(m_col, 1);
			ChangeGrid(renderer, m_row, m_col);
			g_root.m_cfg.SetValue("Grid-Col", m_col);
		}
		if (ImGui::DragFloat2("TileSize W-H", (float*)&m_tileSize))
		{
			ChangeTileSize();
			//g_root.m_cfg.SetValue("Grid-Size", m_tileSize);
			g_root.m_cfg.SetValue("Grid-SizeW", m_tileSize.x);
			g_root.m_cfg.SetValue("Grid-SizeH", m_tileSize.y);
		}
		//if (ImGui::DragFloat("TileSize", &m_tileSize))
		//{
		//	ChangeTileSize();
		//	g_root.m_cfg.SetValue("Grid-Size", m_tileSize);
		//}
		ImGui::InputText("Formatter", m_fileNameFomatter.m_str, sizeof(m_fileNameFomatter.m_str));
		ImGui::SameLine(); 
		if (ImGui::SmallButton("read"))
		{
			LoadFileByFomatter(renderer, m_fileNameFomatter);
			g_root.m_cfg.SetValue("Grid-Fomatter", m_fileNameFomatter.c_str());
		}

		if (ImGui::TreeNode("Tile Fomatter Option"))
		{
			ImGui::RadioButton("col-row", &m_tileFmtOption.order, 0); 
			ImGui::SameLine(); ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("row-col", &m_tileFmtOption.order, 1);

			ImGui::RadioButton("row-increment", &m_tileFmtOption.rowInc, 1);
			ImGui::SameLine(); ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("row-decrement", &m_tileFmtOption.rowInc, 0);

			ImGui::RadioButton("col-increment", &m_tileFmtOption.colInc, 1);
			ImGui::SameLine(); ImGui::Dummy(ImVec2(20, 0)); ImGui::SameLine();
			ImGui::RadioButton("col-decrement", &m_tileFmtOption.colInc, 0);

			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		
		int selectTile = -1;

		//ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
		if (ImGui::TreeNode("Grid Select"))
		{
			for (int y = 0; y < m_row; ++y)
			{
				for (int x=0; x < m_col; ++x)
				{
					const int idx = x + y*m_col;
					ImGui::PushID(idx);
					bool select = m_gridSelect[idx];

					Str64 name;
					name.Format("%d-%d", y, x);
					if (ImGui::Selectable(name.c_str(), &select, 0, ImVec2(50, 40)))
					{
						if (select)
						{
							// All Tile Clear
							for (u_int i = 0; i < m_gridSelect.size(); ++i)
								m_gridSelect[i] = false;
						}

						selectTile = select ? idx : -1;
						m_selectTileIdx = select? idx : -1;
						m_gridSelect[idx] = select;
					}
					ImGui::SameLine();
					ImGui::PopID();
				}
				ImGui::NewLine();
			}
			ImGui::TreePop();
		}

		if (m_selectTileIdx >= 0)
		{
			Str64 infos;
			infos.Format("Tile Information [ %d-%d ]", m_selectTileIdx / m_col, m_selectTileIdx%m_col);
			ImGui::Text(infos.c_str());

			auto it = m_tiles.find(m_selectTileIdx);
			assert(m_tiles.end() != it);
			cTile *tile = it->second;

			g_root.m_selectTile = m_gridSelect[m_selectTileIdx]? tile : NULL;

			if (tile->m_ground->m_texture)
			{
				infos.Format("FileName : %s", tile->m_ground->m_texture->m_fileName.c_str());
				WStrPath wstr = infos.wstr().c_str();
				ImGui::Text(wstr.utf8().c_str());
			}
			if (ImGui::Button("Change Texture"))
			{
				tile->m_ground->m_texture = cResourceManager::Get()->LoadTexture(renderer
					, g_root.m_resWindow->m_selectPath.c_str());
				tile->m_ground->m_mtrl.InitWhite();
			}
		}
		else
		{
			m_selectTileIdx = -1;
			g_root.m_selectTile = NULL;
		}

		ImGui::Spacing();
	}
}


void cGridEdit::SelectTile(cTile *tile)
{
	RET(!tile);

	// All Tile Clear
	for (u_int i = 0; i < m_gridSelect.size(); ++i)
		m_gridSelect[i] = false;

	const int tileIdx = tile->m_location.x * m_col + tile->m_location.y;
	m_selectTileIdx = tileIdx;
	m_gridSelect[tileIdx] = true;
}


void cGridEdit::ChangeGrid(cRenderer &renderer, const int row, const int col)
{
	m_selectTileIdx = -1;
	m_gridSelect.resize(row * col, false);

	g_root.m_terrain.Clear();
	g_root.m_selectTile = NULL;
	g_root.m_selectModel = NULL;
	g_root.m_gizmo->Cancel();
	g_root.m_terrain.Create(renderer, row*16, col*16, m_tileSize.x/16, m_tileSize.y / 16, row, col);

	//const Vector3 offset(0, 0, (row-1)*m_tileSize);
	const Vector3 offset(0, 0, 0);
	m_tiles.clear();

	//
	// + --------------- + --------------- + --------------- +
	// |                 |                 |                 |
	// | tile(r=2,c=0)   | tile(r=2,c=1)   |  tile(r=2,c=2)  |
	// |                 |                 |                 |
	// + --------------- + --------------- + --------------- +
	// |                 |                 |                 |
	// | tile(r=1,c=0)   | tile(r=1,c=1)   |  tile(r=1,c=2)  |
	// |                 |                 |                 |
	// + --------------- + --------------- + --------------- +
	// |                 |                 |                 |
	// | tile(r=0,c=0)   | tile(r=0,c=1)   |  tile(r=0,c=2)  |
	// |                 |                 |                 |
	// + --------------- + --------------- + --------------- +
	//
	// Tile Row-Col Axis
	//
	for (int y = 0; y < m_row; ++y)
	{
		for (int x = 0; x < m_col; ++x)
		{
			cTile *tile = new cTile();

			const sRectf rect(m_tileSize.x*x + offset.x
				, m_tileSize.y*y + offset.z
				, m_tileSize.x*x + offset.x + m_tileSize.x
				, m_tileSize.y*y + offset.z + m_tileSize.y);
			Transform transform;
			transform.pos = Vector3(m_tileSize.x*x + offset.x, 0, m_tileSize.y*y + offset.z);
			transform.scale = Vector3(m_tileSize.x/2.f, 1, m_tileSize.y/2.f);

			const StrPath fileName = "gray.dds";
			tile->Create(renderer
				, common::GenerateId()
				//, fileName.GetFileNameExceptExt().c_str()
				, format("Tile%d-%d", x, y).c_str()
				, y // row
				, x // col
				//, rect
				, transform
				, fileName.c_str()
				, 1
			);

			//tile->m_dbgTile.SetColor(cColor(1.f, 0, 0, 1));
			m_tiles[y*m_col + x] = tile;
			g_root.m_terrain.AddTile(tile);
		}
	}
}


void cGridEdit::ChangeTileSize()
{
	const Vector3 offset(0, 0, (m_row - 1)*m_tileSize.y);

	for (int y = 0; y < m_row; ++y)
	{
		for (int x = 0; x < m_col; ++x)
		{
			const int tileId = y*m_col + x;
			cTile *tile = m_tiles[tileId];

			const sRectf rect(m_tileSize.x*x + offset.x, offset.z - m_tileSize.y*y
				, m_tileSize.x*x + m_tileSize.x + offset.x
				, offset.z - m_tileSize.y*y + m_tileSize.y);

			tile->UpdatePosition(rect);
		}
	}
}


// 타일 파일을 읽어드린다.
// 연속된 이미지 파일이 row, col 값으로 이름되어있다면,
// 파일이름에서 row,col 값을 읽어들여, 해당 타일에 설정한다.
bool cGridEdit::LoadFileByFomatter(cRenderer &renderer, const Str64 &fmt)
{
	WStr64 wfmt = fmt.wstrUTF8();

	vector<WStrPath> files;
	common::CollectFiles({}, L"../media/", files);

	for (auto &str : files)
	{
		WStr64 fileName = str.GetFileName();

		int v1, v2;
		if (scanner(fileName.c_str(), wfmt.c_str(), &v1, &v2, NULL) != 2)
			continue;

		const int col = (m_tileFmtOption.order == 0) ? v1 : v2;
		const int row = (m_tileFmtOption.order == 0) ? v2 : v1;

		int r = row;
		int c = col;

		if (0 == m_tileFmtOption.rowInc) // decrement?
			r = m_row - r;
		else
			r -= 1;

		if (0 == m_tileFmtOption.colInc) // decrement?
			c = m_col - c;
		else
			c -= 1;

		if ((col < 0) || (row < 0))
			continue;

		const int idx = c + r*m_col;
		auto it = m_tiles.find(idx);
		if (m_tiles.end() == it)
			continue;

		cTile *tile = it->second;
		tile->m_ground->m_texture = cResourceManager::Get()->LoadTexture(renderer, str.str());
	}

	return true;
}
