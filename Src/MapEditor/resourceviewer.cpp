
#include "stdafx.h"
#include "resourceviewer.h"

using namespace graphic;
using namespace framework;


cResourceViewer::cResourceViewer()
	: cDockWindow("Resource Viewer")
	, m_camera("resource viewer camera")
	, m_isAdjustTextureSize(false)
	, m_state(eState::NONE)
	, m_groundPlane1(Vector3(0, 1, 0), 0)
	, m_groundPlane2(Vector3(0, -1, 0), 0)
	, m_isOpenHud(false)
	, m_isRenderWireFrame(false)
	, m_isRenderAxis(false)
	, m_isEditAnimationTime(false)
	, m_isOutline(false)
	, m_isShadow(true)
	, m_checkLoadModel(false)
{
}

cResourceViewer::~cResourceViewer()
{
}


bool cResourceViewer::Init()
{
	cRenderer &renderer = GetRenderer();
	sf::Vector2u size((u_int)m_rect.Width(), (u_int)m_rect.Height() - 45);

	m_camera.SetCamera(Vector3(10,10,10), Vector3(0,0,0), Vector3(0, 1, 0));
	m_camera.SetProjection(MATH_PI / 4.f, (float)size.x/ (float)size.y, 1.f, 100000.0f);
	m_camera.SetViewPort((float)size.x, (float)size.y);

	cViewport vp = renderer.m_viewPort;
	vp.m_vp.Width = m_rect.Width();
	vp.m_vp.Height = m_rect.Height();
	vp.m_vp.MinDepth = 0.f;
	vp.m_vp.MaxDepth = 1.f;
	m_renderTarget.Create(renderer, vp, DXGI_FORMAT_R8G8B8A8_UNORM, false);

	m_quad.Create(renderer, 0, 0, (float)size.x, (float)size.y);
	m_ground.Create(renderer, 100, 100, 10, 10, eVertexType::POSITION | eVertexType::COLOR
		, cColor(0.6f, 0.6f, 0.6f, 1.f)
		, cColor(1.f, 1.f, 1.f, 1.f)
	);
	m_ground.m_lineColor = cColor(0.7f, 0.7f, 0.7f, 0.7f);

	m_ccsm.Create(renderer);
	m_depthQuad.Create(renderer, 0, 200, 256, 256);

	return true;
}


void cResourceViewer::OnUpdate(const float deltaSeconds)
{
	cRenderer &renderer = GetRenderer();
	cAutoCam cam(&m_camera);
	switch (m_state)
	{
	case eState::NONE:
	case eState::TEXTURE: break;
	case eState::MODEL:
		m_model.Update(renderer, m_isEditAnimationTime ? 0 : deltaSeconds);
		break;
	}
	m_camera.Update(deltaSeconds);
}


void cResourceViewer::OnPreRender(const float deltaSeconds)
{
	if (!m_isAdjustTextureSize)
		AdjustTextureSize();

	cRenderer &renderer = GetRenderer();
	cAutoCam cam(&m_camera);

	renderer.UnbindTextureAll();

	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);

	renderer.m_textMgr.NewFrame();

	if (m_isOutline && (eState::MODEL == m_state))
	{
		GetMainCamera().Bind(renderer);
		cCamera &cam = GetMainCamera();
		if (g_root.m_depthBuff.Begin(renderer, true))
		{
			m_model.m_techniqueName = "DepthTech";
			m_model.Render(renderer);
		}
		g_root.m_depthBuff.End(renderer);
	}

	// 모델 로드가 완료되면, 모델 크기에 맞게, 카메라 위치를 보정한다.
	if (eState::MODEL == m_state)
	{
		if (m_checkLoadModel != m_model.m_isLoad)
		{
			m_checkLoadModel = m_model.m_isLoad;
			if (m_model.m_isLoad)
				m_camera.Move(m_model.m_boundingBox);
		}
	}

	GetMainCamera().Bind(renderer);
	GetMainLight().Bind(renderer);

	// Shadow Map 생성
	if (m_isShadow && (eState::MODEL == m_state))
	{
		m_ccsm.BuildShadowMap(renderer, &m_model);
		m_ccsm.BuildShadowMap(renderer, &m_ground, XMIdentity, false);
	}

	// Render
	if (m_renderTarget.Begin(renderer))
	{
		GetMainCamera().Bind(renderer);
		GetMainLight().Bind(renderer);

		switch (m_state)
		{
		case eState::NONE: break;
		case eState::TEXTURE: 
			m_quad.Render(renderer); 
			break;

		case eState::MODEL:
		{
			DirectX::CommonStates state(renderer.GetDevice());
			renderer.GetDevContext()->RSSetState(m_isRenderWireFrame ? state.Wireframe() : state.CullCounterClockwise());
			
			if (m_isShadow)
			{
				m_ccsm.RenderShadowMap(renderer, &m_model);
				m_ccsm.RenderShadowMap(renderer, &m_ground);
			}
			else
			{
				m_ground.m_techniqueName = "Unlit";
				m_model.m_techniqueName = "Unlit";
				m_ground.Render(renderer);
				m_model.Render(renderer);
			}

			if (m_isOutline)
			{
				renderer.GetDevContext()->OMSetDepthStencilState(state.DepthNone(), 0);
				renderer.BindTexture(g_root.m_depthBuff, 7);
				m_model.m_techniqueName =  "Outline";
				m_model.Render(renderer);
				renderer.GetDevContext()->OMSetDepthStencilState(state.DepthDefault(), 0);
			}

			if (renderer.m_isDbgRender)
			{
				static cTexture shadowTex(g_root.m_depthBuff.m_depthSRV);
				shadowTex.m_texSRV = g_root.m_depthBuff.m_depthSRV;
				m_depthQuad.m_texture = &shadowTex;
				m_depthQuad.Render(renderer);
			}
			if (m_isRenderAxis)
				renderer.RenderAxis();

			renderer.GetDevContext()->RSSetState(state.CullCounterClockwise());
		}
		break;
		}
	}
	m_renderTarget.End(renderer);
}


void cResourceViewer::OnRender(const float deltaSeconds)
{
	ImVec2 pos = ImGui::GetCursorScreenPos();
	m_viewPos = { (int)(pos.x), (int)(pos.y) };
	m_viewRect = { pos.x + 5, pos.y, pos.x + m_rect.Width() - 30, pos.y + m_rect.Height() - 50};
	ImGui::Image(m_renderTarget.m_resolvedSRV, ImVec2(m_rect.Width(), m_rect.Height() - 50));

	// HUD
	const float windowAlpha = 0.5f;
	bool isOpen = true;
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
	ImGui::SetNextWindowPos(pos);
	ImGui::SetNextWindowSize(ImVec2(min(m_viewRect.Width(), 500), m_isOpenHud? m_viewRect.Height() : 110));
	ImGui::SetNextWindowBgAlpha(windowAlpha);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	if (ImGui::Begin("Information", &isOpen
		//, ImVec2(min(m_viewRect.Width(), 500), m_isOpenHud? m_viewRect.Height() : 110.f), windowAlpha
		, flags))
	{
		m_isOpenHud = false;

		StrPath str;
		switch (m_state)
		{
		case eState::NONE: break;
		case eState::TEXTURE: 
			if (m_quad.m_texture)
				str.Format("FileName = %s", m_quad.m_texture->m_fileName.utf8().c_str()); 
			ImGui::Text(str.c_str());
			ImGui::Text("FileSize = %s", m_fileSize.c_str());
			break;

		case eState::MODEL: 
		{
			str.Format("FileName = %s", m_model.m_fileName.utf8().c_str());
			ImGui::Text(str.c_str());
			ImGui::Text("FileSize = %s", m_fileSize.c_str());
			ImGui::Checkbox("WireFrame", &m_isRenderWireFrame);
			ImGui::SameLine();
			ImGui::Checkbox("Axis", &m_isRenderAxis);
			ImGui::SameLine();
			ImGui::Checkbox("Shadow", &m_isShadow);
			ImGui::SameLine();
			ImGui::Checkbox("Outline", &m_isOutline);
			ImGui::Spacing();

			RenderModelInfo();
		}
		break;
		}

		ImGui::End();
	}
	ImGui::PopStyleColor();
}


void cResourceViewer::RenderModelInfo()
{
	RET(!m_model.m_model);

	if (ImGui::CollapsingHeader("Detail Information"))
	{
		m_isOpenHud = true;

		if (!m_model.m_model->m_nodes.empty())
		{
			if (ImGui::TreeNode("Node Hierarchy"))
			{
				ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Always);
				RenderModelNode(m_model.m_model->m_nodes[0]);
				ImGui::TreePop();
			}
		}

		RenderLocalTransformInfo();
		RenderMeshInfo();
		RenderAnimationInfo();
		RenderShader();
	}
}


void cResourceViewer::RenderModelNode(const sRawNode &node)
{
	if (ImGui::TreeNode(node.name.c_str()))
	{
		if (ImGui::TreeNode("<Local Matrix>"))
		{
			ImGui::Text("%.4f %.4f %.4f %.4f", node.localTm._11, node.localTm._12, node.localTm._13, node.localTm._14);
			ImGui::Text("%.4f %.4f %.4f %.4f", node.localTm._21, node.localTm._22, node.localTm._23, node.localTm._24);
			ImGui::Text("%.4f %.4f %.4f %.4f", node.localTm._31, node.localTm._32, node.localTm._33, node.localTm._34);
			ImGui::Text("%.4f %.4f %.4f %.4f", node.localTm._41, node.localTm._42, node.localTm._43, node.localTm._44);
			ImGui::TreePop();
		}

		for (u_int i = 0; i < node.children.size(); ++i)
			RenderModelNode(m_model.m_model->m_nodes[node.children[i]]);
		ImGui::TreePop();
	}
}


// Render Model Local Transform Information
void cResourceViewer::RenderLocalTransformInfo()
{
	if (ImGui::CollapsingHeader("Local Transform"))
	{
		ImGui::DragFloat3("Local Pos", (float*)&m_model.m_localTm.pos, 0.001f);
		ImGui::DragFloat3("Local Scale", (float*)&m_model.m_localTm.scale, 0.001f);
		Vector3 ryp = m_model.m_localTm.rot.Euler();
		if (ImGui::DragFloat3("Roll-Yaw-Pitch", (float*)&ryp, 0.001f))
			m_model.m_localTm.rot.Euler2(ryp);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0, 1));
		if (ImGui::Button("Save - Model Information"))
		{
			WriteModelInfo();
		}
		ImGui::PopStyleColor(3);
	}
}


// filename + .inf 파일에 Mesh Visible, Model LocalTm 정보를 저장한다.
// format
//	pos, x, y, z
//	scale, x, y, z
//	rot, x, y, z, w
//  meshvisible, mesh index, 0/1
bool cResourceViewer::WriteModelInfo()
{
	cSimpleData sd;

	vector<string> title;
	title.push_back("Model Information");
	
	vector<string> pos;
	pos.push_back("pos");
	pos.push_back(std::to_string(m_model.m_localTm.pos.x));
	pos.push_back(std::to_string(m_model.m_localTm.pos.y));
	pos.push_back(std::to_string(m_model.m_localTm.pos.z));

	vector<string> scale;
	scale.push_back("scale");
	scale.push_back(std::to_string(m_model.m_localTm.scale.x));
	scale.push_back(std::to_string(m_model.m_localTm.scale.y));
	scale.push_back(std::to_string(m_model.m_localTm.scale.z));

	vector<string> rot;
	rot.push_back("rot");
	rot.push_back(std::to_string(m_model.m_localTm.rot.x));
	rot.push_back(std::to_string(m_model.m_localTm.rot.y));
	rot.push_back(std::to_string(m_model.m_localTm.rot.z));
	rot.push_back(std::to_string(m_model.m_localTm.rot.w));

	sd.m_table.push_back(title);
	sd.m_table.push_back(pos);
	sd.m_table.push_back(scale);
	sd.m_table.push_back(rot);

	// store mesh invisible
	for (uint i = 0; i < m_model.m_model->m_meshes.size(); ++i)
	{
		auto &mesh = m_model.m_model->m_meshes[i];
		if (!mesh->IsVisible())
		{
			vector<string> invis;
			invis.push_back("meshvisible");
			invis.push_back(std::to_string(i));
			invis.push_back(std::to_string(0)); // invisible mesh
			sd.m_table.push_back(invis);
		}
	}

	return sd.Write((m_model.m_fileName + ".inf").c_str());
}


void cResourceViewer::RenderMeshInfo()
{
	if (ImGui::CollapsingHeader("Mesh Information"))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0, 1));
		if (ImGui::Button("Save - Model Information"))
		{
			WriteModelInfo();
		}
		ImGui::PopStyleColor(3);
		ImGui::Spacing();

		for (auto &mesh : m_model.m_model->m_meshes)
		{
			ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Always);
			if (ImGui::TreeNode(mesh->m_name.c_str()))
			{
				bool isVisible = mesh->IsVisible();
				if (ImGui::Checkbox("Visible", &isVisible))
					mesh->SetRenderFlag(eRenderFlag::VISIBLE, isVisible);
				bool isAlphaBlend = (mesh->m_renderFlags & eRenderFlag::ALPHABLEND) ? true : false;
				if (ImGui::Checkbox("AlphaBlend", &isAlphaBlend))
					mesh->m_renderFlags = isAlphaBlend ? (eRenderFlag::VISIBLE | eRenderFlag::ALPHABLEND) : (eRenderFlag::VISIBLE | eRenderFlag::NOALPHABLEND);

				if (!mesh->m_colorMap.empty())
					ImGui::Text("ColorMap : %s", mesh->m_colorMap[0]->m_fileName.c_str());
				if (!mesh->m_normalMap.empty())
					ImGui::Text("NormalMap : %s", mesh->m_normalMap[0]->m_fileName.c_str());
				if (!mesh->m_specularMap.empty())
					ImGui::Text("SpecularMap : %s", mesh->m_specularMap[0]->m_fileName.c_str());
				if (!mesh->m_selfIllumMap.empty())
					ImGui::Text("EmissiveMap : %s", mesh->m_selfIllumMap[0]->m_fileName.c_str());

				if (!mesh->m_mtrls.empty())
				{
					ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Always);
					if (ImGui::TreeNode("Material"))
					{					
						ImGui::ColorEdit4("Ambient", (float*)&mesh->m_mtrls[0].m_ambient);
						ImGui::ColorEdit4("Diffuse", (float*)&mesh->m_mtrls[0].m_diffuse);
						ImGui::ColorEdit4("Specular", (float*)&mesh->m_mtrls[0].m_specular);
						ImGui::ColorEdit4("Emissive", (float*)&mesh->m_mtrls[0].m_emissive);
						ImGui::DragFloat("Power", &mesh->m_mtrls[0].m_power, 0.001f);
						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}
	}
}


void cResourceViewer::RenderAnimationInfo()
{
	if (ImGui::CollapsingHeader("Animation Information"))
	{
		if (!m_model.m_model->m_animation.m_aniGroup)
			return;

		//const int aniIdx = m_model.m_model->m_animation.m_curAniIdx;
		//if (aniIdx < 0)
		//	return;

		// Animation ComboBox
		int idx = 0;
		char aniArray[512];
		ZeroMemory(aniArray, sizeof(aniArray));
		aniArray[0] = 'N';
		aniArray[1] = 'o';
		aniArray[2] = 'n';
		aniArray[3] = 'e';
		aniArray[4] = '\0';
		idx = 5;
		for (u_int i = 0; i < m_model.m_model->m_animation.m_aniGroup->anies.size(); ++i)
		{
			const sRawAni &ani = m_model.m_model->m_animation.m_aniGroup->anies[i];
			strcpy(&aniArray[idx], ani.name.c_str());
			idx += ani.name.size();
			aniArray[idx++] = NULL;
		}

		const int aniIdx = m_model.m_model->m_animation.m_curAniIdx;
		int selIdx = (aniIdx < 0)? 0 : aniIdx + 1;
		if (ImGui::Combo("Animation", &selIdx, aniArray))
		{
			if (0 == selIdx) // None
			{
				m_model.m_animation.Stop();
			}
			else
			{
				m_model.SetAnimation(m_model.m_model->m_animation.m_aniGroup->anies[selIdx-1].name);
			}
		}
		//

		if (aniIdx < 0)
			return;

		ImGui::Checkbox("Edit Animation Time", &m_isEditAnimationTime);
		if (m_isEditAnimationTime)
		{
			ImGui::DragFloat("Time", &m_model.m_aniIncT, 0.005f);
			m_model.m_aniIncT = max(0, m_model.m_aniIncT);
		}
		ImGui::Text("Start Time : %f", m_model.m_model->m_animation.m_start);
		ImGui::Text("End Time : %f", m_model.m_model->m_animation.m_end);

		ImGui::DragFloat("Animation Speed", &m_model.m_animationSpeed, 0.01f);
		ImGui::ProgressBar((m_model.m_aniIncT * m_model.m_animationSpeed) / m_model.m_model->m_animation.m_end);
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::TreeNode("Animation Node"))
		{
			for (auto &aniNode : m_model.m_model->m_animation.m_anies[aniIdx])
			{
				if (ImGui::TreeNode(aniNode.m_boneAni->name.c_str()
					, "%s [ Pos=%d, Rot=%d, Scale=%d ]"
					, aniNode.m_boneAni->name.c_str()
					, aniNode.m_boneAni->pos.size()
					, aniNode.m_boneAni->rot.size()
					, aniNode.m_boneAni->scale.size()
				))
				{
					if (ImGui::TreeNode("Position", "Position=%d", aniNode.m_boneAni->pos.size()))
					{
						for (auto &key : aniNode.m_boneAni->pos)
							ImGui::Text("t=%f, xyz=%f, %f, %f", key.t, key.p.x, key.p.y, key.p.z);
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Rotation", "Rotation=%d", aniNode.m_boneAni->rot.size()))
					{
						for (auto &key : aniNode.m_boneAni->rot)
							ImGui::Text("t=%f, xyzw=%f, %f, %f, %f", key.t, key.q.x, key.q.y, key.q.z, key.q.w);
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Scaling", "Scaling=%d ", aniNode.m_boneAni->scale.size()))
					{
						for (auto &key : aniNode.m_boneAni->scale)
							ImGui::Text("t=%f, xyz=%f, %f, %f", key.t, key.s.x, key.s.y, key.s.z);
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Current Animation Node"))
		{
			int genId = 0;
			for (auto &aniNode : m_model.m_model->m_animation.m_anies[aniIdx])
			{
				Vector3 pos(0, 0, 0);
				Quaternion rot(0, 0, 0, 1);
				Vector3 scale(1, 1, 1);
				aniNode.GetAnimationResult(m_model.m_aniIncT, pos, rot, scale);
				
				ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);

				Str32 name;
				if (aniNode.m_boneAni->name.empty())
					name.Format("tool generate id %d", genId++);
				else
					name = aniNode.m_boneAni->name.c_str();

				if (ImGui::TreeNode(name.c_str()))
				{
					ImGui::Checkbox("Enable", &aniNode.m_isEnable);
					ImGui::Text("Pos={%f,%f,%f}", pos.x, pos.y, pos.z);
					ImGui::Text("Rot={%f,%f,%f,%f}", rot.x, rot.y, rot.z, rot.w);
					ImGui::Text("Scale={%f,%f,%f}", scale.x, scale.y, scale.z);

					if (ImGui::TreeNode("<Matrix>"))
					{
						Matrix44 aniTm;
						aniNode.GetAnimationResult(m_model.m_aniIncT, aniTm);

						ImGui::Text("%.4f %.4f %.4f %.4f", aniTm._11, aniTm._12, aniTm._13, aniTm._14);
						ImGui::Text("%.4f %.4f %.4f %.4f", aniTm._21, aniTm._22, aniTm._23, aniTm._24);
						ImGui::Text("%.4f %.4f %.4f %.4f", aniTm._31, aniTm._32, aniTm._33, aniTm._34);
						ImGui::Text("%.4f %.4f %.4f %.4f", aniTm._41, aniTm._42, aniTm._43, aniTm._44);
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
	}
}


void cResourceViewer::RenderShader()
{
	if (ImGui::CollapsingHeader("Shader Information"))
	{
		cRenderer &renderer = GetRenderer();

		std::set<cShader11*> shaders;
		for (auto &mesh : m_model.m_model->m_meshes)
		{
			if (!mesh->m_buffers)
				continue;
			shaders.insert(renderer.m_shaderMgr.FindShader(mesh->m_buffers->m_vtxType));
		}

		ImGui::InputText("Shader Directory", renderer.m_shaderMgr.m_shaderRootPath.m_str
			, sizeof(renderer.m_shaderMgr.m_shaderRootPath.m_str));

		ImGui::Separator();
		ImGui::Spacing();

		for (auto &shader : shaders)
		{
			ImGui::Text(shader->m_name.c_str());
			ImGui::SameLine();
			ImGui::PushID((int)shader->m_name.c_str());
			if (ImGui::Button("Compile"))
				shader->CompileAndReload(renderer);
			ImGui::PopID();
			ImGui::SameLine();
			if (ImGui::Button("Open"))
				shader->OpenFile(renderer);
			ImGui::SameLine();
			if (ImGui::Button("Explore"))
				shader->OpenFile(renderer, "open");
		}
	}
}


void cResourceViewer::OnResizeEnd(const eDockResize::Enum type, const sRectf &rect)
{
	if (type == eDockResize::DOCK_WINDOW)
	{
		m_owner->RequestResetDeviceNextFrame();
	}
}


bool cResourceViewer::LoadResource(const StrPath &fileName)
{
	cRenderer &renderer = GetRenderer();

	if (g_root.IsModelExtendsName(fileName.c_str())) // check ext
	{
		m_state = eState::MODEL;
		m_model.Create(renderer, common::GenerateId(), fileName.c_str(), true);
		m_fileSize = fileName.FileSizeStr().c_str();
	}
	else
	{
		m_state = eState::TEXTURE;

		m_quad.m_texture = cResourceManager::Get()->LoadTextureParallel(renderer, fileName.c_str());
		cResourceManager::Get()->AddParallelLoader(new cParallelLoader(cParallelLoader::eType::TEXTURE
			, fileName.c_str(), (void**)&m_quad.m_texture));
		m_fileSize = fileName.FileSizeStr().c_str();

		if (m_quad.m_texture)
		{
			AdjustTextureSize();
		}
		else
		{
			m_isAdjustTextureSize = false;
		}
	}

	return true;
}


void cResourceViewer::AdjustTextureSize()
{
	RET(!m_quad.m_texture);

	cTexture *tex = m_quad.m_texture;

	// Static Rate Scaling
	const float hr = (float)m_rect.Height() / (float)tex->Height();
	const float wr = (float)m_rect.Width() / (float)tex->Width();
	const float r = min(hr, wr);

	m_quad.SetPosition(0, 0, tex->Width()*r, (tex->Height()*r) - 45);
	m_isAdjustTextureSize = true;
}



void cResourceViewer::UpdateLookAt()
{
	GetMainCamera().MoveCancel();

	const float centerX = GetMainCamera().m_width / 2;
	const float centerY = GetMainCamera().m_height / 2;
	const Ray ray = GetMainCamera().GetRay((int)centerX, (int)centerY);
	const float distance = m_groundPlane1.Collision(ray.dir);
	if (distance < -0.2f)
	{
		GetMainCamera().m_lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	}
	else
	{ // horizontal viewing
		const Vector3 lookAt = GetMainCamera().m_eyePos + GetMainCamera().GetDirection() * 50.f;
		GetMainCamera().m_lookAt = lookAt;
	}

	GetMainCamera().UpdateViewMatrix();
}


bool cResourceViewer::IsMouseHoverOnUI()
{
	return (ImGui::IsMouseHoveringRect(ImVec2(0, 0)
		, ImVec2(min(m_viewRect.Width(), 500), m_isOpenHud ? m_viewRect.Height() : 60)
		, false));
}


// 휠을 움직였을 때,
// 카메라 앞에 박스가 있다면, 박스 정면에서 멈춘다.
void cResourceViewer::OnWheelMove(const float delta, const POINT mousePt)
{
	if (IsMouseHoverOnUI())
		return;

	UpdateLookAt();

	float len = 0;
	const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
	Vector3 lookAt = m_groundPlane1.Pick(ray.orig, ray.dir);
	len = (ray.orig - lookAt).Length();

	// zoom in/out
	float zoomLen = 0;
	if (len > 100)
		zoomLen = 50;
	else if (len > 50)
		zoomLen = max(1.f, (len / 2.f));
	else
		zoomLen = (len / 3.f);

	//Vector3 eyePos = GetMainCamera().m_eyePos + ray.dir * ((delta <= 0) ? -zoomLen : zoomLen);
	GetMainCamera().Zoom(ray.dir, (delta < 0) ? -zoomLen : zoomLen);
}


// Handling Mouse Move Event
void cResourceViewer::OnMouseMove(const POINT mousePt)
{
	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	if (m_mouseDown[0])
	{
		Vector3 dir = GetMainCamera().GetDirection();
		Vector3 right = GetMainCamera().GetRight();
		dir.y = 0;
		dir.Normalize();
		right.y = 0;
		right.Normalize();

		GetMainCamera().MoveRight(-delta.x * m_rotateLen * 0.001f);
		GetMainCamera().MoveFrontHorizontal(delta.y * m_rotateLen * 0.001f);
	}
	else if (m_mouseDown[1])
	{
		m_camera.Yaw2(delta.x * 0.005f, Vector3(0, 1, 0));
		m_camera.Pitch2(delta.y * 0.005f, Vector3(0, 1, 0));

		//GetMainCamera().Yaw2(delta.x * 0.005f, Vector3(0, 1, 0));
		//GetMainCamera().Pitch2(delta.y * 0.005f, Vector3(0, 1, 0));
	}
	else if (m_mouseDown[2])
	{
		const float len = GetMainCamera().GetDistance();
		GetMainCamera().MoveRight(-delta.x * len * 0.001f);
		GetMainCamera().MoveUp(delta.y * len * 0.001f);
	}
}


// Handling Mouse Button Down Event
void cResourceViewer::OnMouseDown(const sf::Mouse::Button &button, const POINT mousePt)
{
	if (IsMouseHoverOnUI())
		return;

	m_mousePos = mousePt;
	UpdateLookAt();
	SetCapture();

	switch (button)
	{
	case sf::Mouse::Left:
	{
		m_mouseDown[0] = true;
		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		Vector3 p1 = m_groundPlane1.Pick(ray.orig, ray.dir);
		m_rotateLen = min(500.f, (p1 - ray.orig).Length());
	}
	break;

	case sf::Mouse::Right:
	{
		m_mouseDown[1] = true;
		const Ray ray = GetMainCamera().GetRay(mousePt.x, mousePt.y);
		Vector3 target = m_groundPlane1.Pick(ray.orig, ray.dir);
		const float len = (GetMainCamera().GetEyePos() - target).Length();
	}
	break;

	case sf::Mouse::Middle:
		m_mouseDown[2] = true;
		break;
	}
}


void cResourceViewer::OnMouseUp(const sf::Mouse::Button &button, const POINT mousePt)
{
	if (IsMouseHoverOnUI())
		return;

	const POINT delta = { mousePt.x - m_mousePos.x, mousePt.y - m_mousePos.y };
	m_mousePos = mousePt;

	switch (button)
	{
	case sf::Mouse::Left:
		ReleaseCapture();
		m_mouseDown[0] = false;
		break;
	case sf::Mouse::Right:
		m_mouseDown[1] = false;
		ReleaseCapture();
		break;
	case sf::Mouse::Middle:
		m_mouseDown[2] = false;
		ReleaseCapture();
		break;
	}
}


void cResourceViewer::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.code)
		{
		case sf::Keyboard::Return:
			break;

		case sf::Keyboard::Space:
			break;

		case sf::Keyboard::Left: m_camera.MoveRight(-0.5f); break;
		case sf::Keyboard::Right: m_camera.MoveRight(0.5f); break;
		case sf::Keyboard::Up: m_camera.MoveUp(0.5f); break;
		case sf::Keyboard::Down: m_camera.MoveUp(-0.5f); break;
		}
		break;

	case sf::Event::MouseMoved:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnMouseMove(pos);
	}
	break;

	case sf::Event::MouseButtonPressed:
	case sf::Event::MouseButtonReleased:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		if (sf::Event::MouseButtonPressed == evt.type)
		{
			if (m_viewRect.IsIn((float)curPos.x, (float)curPos.y))
				OnMouseDown(evt.mouseButton.button, pos);
		}
		else
		{
			OnMouseUp(evt.mouseButton.button, pos);
		}
	}
	break;

	case sf::Event::MouseWheelScrolled:
	{
		cAutoCam cam(&m_camera);

		POINT curPos;
		GetCursorPos(&curPos); // sf::event mouse position has noise so we use GetCursorPos() function
		ScreenToClient(m_owner->getSystemHandle(), &curPos);
		const POINT pos = { curPos.x - m_viewPos.x, curPos.y - m_viewPos.y };
		OnWheelMove(evt.mouseWheelScroll.delta, pos);
	}
	break;
	}
}


void cResourceViewer::OnResetDevice()
{
	cRenderer &renderer = GetRenderer();

	sRectf viewRect = { 0, 0, m_rect.Width() - 15, m_rect.Height() - 50 };
	m_camera.SetViewPort(viewRect.Width(), viewRect.Height());

	cViewport vp = GetRenderer().m_viewPort;
	vp.m_vp.Width = viewRect.Width();
	vp.m_vp.Height = viewRect.Height();
	m_renderTarget.Create(renderer, vp);

	AdjustTextureSize();
}
