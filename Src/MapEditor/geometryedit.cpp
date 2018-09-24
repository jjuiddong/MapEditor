
#include "stdafx.h"
#include "geometryedit.h"

using namespace graphic;


cGeometryEdit::cGeometryEdit()
	: m_isEdit(false)
	, m_isHiLightTile(false)
{
}

cGeometryEdit::~cGeometryEdit()
{
}


bool cGeometryEdit::Init(graphic::cRenderer &renderer)
{
	return true;
}


void cGeometryEdit::Render(graphic::cRenderer &renderer)
{
	if (ImGui::CollapsingHeader("Geometry Edit"))
	{
		bool editCircleInfo = false;
		ImGui::Checkbox("Edit Geometry", &m_isEdit);
		ImGui::Checkbox("Hilight Tile", &m_isHiLightTile);
		static const char *brushType = { "Normal\0Flat\0\0" };
		ImGui::Combo("Brush", (int*)&m_brushType, brushType);		

		if (ImGui::DragFloat("Inner Circle", &g_root.m_tCursor.m_innerRadius, 0.1f))
			editCircleInfo = true;
		if (ImGui::DragFloat("Outter Circle", &g_root.m_tCursor.m_outerRadius, 0.1f))
			editCircleInfo = true;

		ImGui::DragFloat("Brush Speed", &g_root.m_tCursor.m_brushSpeed, 0.1f);

		//ImGui::DragFloat2("MicroBump TexCoord Scale", g_root.m_ocean.m_microBumpTexScale, 0.1f);
		//ImGui::DragFloat2("WaterBump TexCoord Scale", g_root.m_ocean.m_waterBumpTexScale, 0.1f);

		if (editCircleInfo) 
		{
			Plane ground(Vector3(0, 1, 0), 9);
			const Ray ray = g_root.m_camWorld.GetRay();
			Vector3 pos = ground.Pick(ray.orig, ray.dir);
			g_root.m_tCursor.UpdateCursor(renderer, g_root.m_terrain, pos);
		}
	}
}
