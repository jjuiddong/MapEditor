
#include "stdafx.h"
#include "lightedit.h"

using namespace graphic;


cLightEdit::cLightEdit()
	: m_showLightDir(true)
	, m_openLightEdit(false)
{
}

cLightEdit::~cLightEdit()
{
}


bool cLightEdit::Init(graphic::cRenderer &renderer)
{
	m_lightDir = GetMainLight().m_direction;
	return true;
}


void cLightEdit::Render(graphic::cRenderer &renderer)
{
	m_openLightEdit = false;

	//ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_FirstUseEver);
	if (ImGui::CollapsingHeader("Light Edit"))
	{
		m_openLightEdit = true;

		ImGui::Checkbox("Show Light Direction", &m_showLightDir);

		float v[2] = { m_lightDir.x, m_lightDir.z };
		if (ImGui::DragFloat2("Direction X - Z", v, 0.01f))
		{
			m_lightDir.x = v[0];
			m_lightDir.z = v[1];
			m_lightDir.Normalize();
			GetMainLight().m_direction = m_lightDir;
		}

		ImGui::ColorEdit3("Ambient", (float*)&GetMainLight().m_ambient);
		ImGui::ColorEdit3("Diffuse", (float*)&GetMainLight().m_diffuse);
		ImGui::ColorEdit3("Specular", (float*)&GetMainLight().m_specular);
	}
}
