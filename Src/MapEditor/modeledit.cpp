
#include "stdafx.h"
#include "modeledit.h"


using namespace graphic;
using namespace framework;


cModelEdit::cModelEdit()
	: m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_showBoundingBox(false)
	, m_showAxis(false)
{
}

cModelEdit::~cModelEdit()
{
}


void cModelEdit::Render()
{
	if (ImGui::CollapsingHeader("Model Edit"))
	{
		cNode *selectModel = g_root.m_selectModel;
		RET(!selectModel);

		Transform transformL = selectModel->m_transform;

		ImGui::Spacing();
		ImGui::PushItemWidth(150);
		if (ImGui::InputText("Name", selectModel->m_name.m_str, ARRAYSIZE(selectModel->m_name.m_str)))
		{
			cRacker *racker = (cRacker*)dynamic_cast<cRacker*>(selectModel);
			if (racker)
				racker->m_wName = racker->m_name.wstr();
		}
		ImGui::PopItemWidth();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.f, 0, 1));
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			if (selectModel->m_parent)
			{
				selectModel->m_parent->RemoveChild(selectModel);
				g_root.m_selectModel = NULL;
				g_root.m_gizmo->Cancel();
				selectModel = NULL;
			}
		}
		ImGui::PopStyleColor(3);
		RET(!selectModel);

		ImGui::Checkbox("Show Model", &selectModel->m_isEnable);
		ImGui::Checkbox("Show Axis", &m_showAxis);
		ImGui::Checkbox("BoundingBox", &m_showBoundingBox);

		Transform transformW = selectModel->GetWorldTransform();

		const bool edit1 = ImGui::DragFloat3("Pos Local", (float*)&transformL.pos, 0.01f);
		const bool edit2 = ImGui::DragFloat3("Pos World (read only)", (float*)&transformW.pos, 0.01f);
		const bool edit3 = ImGui::DragFloat3("Scale", (float*)&transformL.scale, 0.001f);

		float angle = transformL.rot.GetRotationAngleXZ();
		Vector3 ryp = transformL.rot.Euler();
		const bool edit4 = ImGui::DragFloat3("Roll-Yaw-Pitch", (float*)&ryp, 0.001f);
		const bool edit5 = ImGui::DragFloat("Angle (Radian)", &angle, 0.01f);
		const bool edit6 = ImGui::DragFloat4("Quaternion (read only)", (float*)&transformL.rot, 0.01f);

		if (edit4)
			selectModel->m_transform.rot.Euler2(ryp);

		if (cModel *mod = dynamic_cast<cModel*>(selectModel))
			ImGui::DragFloat("Animation Speed", &mod->m_animationSpeed, 0.001f);
		else if (cCCtv *cctv = dynamic_cast<cCCtv*>(selectModel))
			ImGui::InputText("Movie File", cctv->m_movieFileName.m_str, sizeof(cctv->m_movieFileName.m_str));
		else if (cCube *cube = dynamic_cast<cCube*>(selectModel))
		{
			Vector4 color = cube->m_color.GetColor();
			if (ImGui::ColorEdit4("Color", (float*)&color))
				cube->m_color.SetColor(color);
			//if (ImGui::ColorEdit3("Direction", (float*)&color))
			//	cube->m_color.SetColor(color);
		}
		else if (cRect3D *rect3D = dynamic_cast<cRect3D*>(selectModel))
		{
			if (ImGui::InputInt("Rect ID", &rect3D->m_rectId, 1))
				rect3D->m_rectId = max(0, rect3D->m_rectId); // Always Positive
		}
		else if (cLine *line = dynamic_cast<cLine*>(selectModel))
		{
			Vector4 color = line->m_color.GetColor();
			if (ImGui::ColorEdit4("Color", (float*)&color))
				line->m_color.SetColor(color);
		}
		else if (cRacker *racker = (cRacker*)dynamic_cast<cRacker*>(selectModel))
		{
			if (!racker->m_rackes.empty())
			{
				cRack *rack = racker->m_rackes.front();

				ImGui::Separator();
				ImGui::Text("Rack Information");
				ImGui::Spacing();

				bool isEditRack = false;

				if (ImGui::DragFloat3("Dimension", (float*)&rack->m_info.dim, 0.01f, 0.f))
				{
					isEditRack = true;
				}

				if (ImGui::InputInt("Row-", &rack->m_info.row))
				{
					rack->m_info.row = max(rack->m_info.row, 1);
					isEditRack = true;
				}

				if (ImGui::InputInt("Col-", &rack->m_info.col))
				{
					rack->m_info.col = max(rack->m_info.col, 1);
					isEditRack = true;
				}

				if (ImGui::DragFloat("PillarSize", &rack->m_info.pillarSize, 0.001f, 0.01f, 1.f))
					isEditRack = true;

				if (ImGui::DragFloat("BeamSize", &rack->m_info.beamSize, 0.001f, 0.01f, 1.f))
					isEditRack = true;

				ImGui::SetNextTreeNodeOpen(true, ImGuiCond_FirstUseEver);
				if (ImGui::TreeNode("Column Width"))
				{
					for (int i = 0; i < rack->m_info.col; ++i)
					{
						StrId width;
						width.Format("width-%d", i);
						if (ImGui::DragFloat(width.c_str(), &rack->m_info.width[i], 0.001f, 0.1f, 100.f))
							isEditRack = true;
					}

					ImGui::TreePop();
				}

				// update rack model
				if (isEditRack)
				{
					cRack::sRackInfo rackInfo = rack->m_info;
					const float h = rackInfo.dim.y / (float)rackInfo.row;
					for (int i = 0; i < rackInfo.row + 1; ++i)
						rackInfo.height[i] = h;

					racker->Clear();
					cRack *newRack = new cRack();
					newRack->Create(rackInfo);
					racker->AddRack(g_root.m_mapView->GetRenderer(), newRack);
					racker->Optimize(g_root.m_mapView->GetRenderer());
				}
			}			
		}

		if (edit1)
			selectModel->m_transform.pos = transformL.pos;
		if (edit3)
			selectModel->m_transform.scale = transformL.scale;

		ImGui::Spacing();
	}
}
