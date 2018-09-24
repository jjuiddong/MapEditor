
#include "stdafx.h"
#include "naviedit.h"

using namespace graphic;

cNaviEdit::cNaviEdit()
	: m_showNaviMesh(false)
	, m_showNaviAdjacent(true)
{
	m_color.SetColor(Vector4(1, 0, 0, 0.5f));
	m_transform.pos.y = 0.3f;
}

cNaviEdit::~cNaviEdit()
{
}


bool cNaviEdit::Init(cRenderer &renderer)
{
	m_naviAdjLines.Create(renderer, 1024, cColor::BLUE);
	return true;
}


void cNaviEdit::Render(cRenderer &renderer)
{
	if (ImGui::CollapsingHeader("Navigation Edit"))
	{
		ImGui::InputText(" ", m_fileName.m_str, sizeof(m_fileName.m_str));

		ImGui::SameLine();
		if (ImGui::Button("Read Path File"))
		{
			if (!ReadFromPathFile(renderer, m_fileName.c_str()))
				MessageBox(NULL, L"Read Error", L"Error", MB_OK);
		}

		ImGui::SameLine();
		ImGui::PushID(112);
		if (ImGui::Button("..."))
		{
			StrPath path = OpenFileDialog();
			if (!path.empty())
			{
				if (!ReadFromPathFile(renderer, path.c_str()))
					MessageBox(NULL, L"Read Error", L"Error", MB_OK);
				m_fileName = path;
			}
		}
		ImGui::PopID();

		ImGui::Checkbox("Show Navigation Mesh", &m_showNaviMesh);
		ImGui::Checkbox("Show Navigation Adjacent", &m_showNaviAdjacent);
		ImGui::DragFloat("Mesh Offset Y", &m_transform.pos.y, 0.01f);
		Vector4 color = m_color.GetColor();
		if (ImGui::ColorEdit4("Mesh Color", (float*)&color))
			m_color.SetColor(color);

		Str32 strVtxCnt;
		strVtxCnt.Format("%d", m_naviMesh.m_vertices.size());
		ImGui::InputText("Vertex Count", strVtxCnt.m_str, sizeof(strVtxCnt.m_str), ImGuiInputTextFlags_ReadOnly);

		Str32 strTriCnt;
		strTriCnt.Format("%d", m_naviMesh.m_naviNodes.size());
		ImGui::InputText("Polygon Count", strTriCnt.m_str, sizeof(strTriCnt.m_str), ImGuiInputTextFlags_ReadOnly);

	}
}


// pathFinder 파일을 읽어서, Navigation Mesh를 생성한다.
bool cNaviEdit::ReadFromPathFile(cRenderer &renderer, const char *fileName)
{
	if (!m_naviMesh.ReadFromPathFile(fileName))
		return false;

	// Navigation Mesh를 생성한다.
	vector<WORD> indices;
	indices.reserve(m_naviMesh.m_naviNodes.size() * 3);
	for (auto &node : m_naviMesh.m_naviNodes)
	{
		indices.push_back(node.idx1);
		indices.push_back(node.idx2);
		indices.push_back(node.idx3);
	}

	// Vertex, Index 버퍼 생성
	m_vtxBuff.Create(renderer, m_naviMesh.m_vertices.size(), sizeof(Vector3), &m_naviMesh.m_vertices[0]);
	m_idxBuff.Create(renderer, indices.size() / 3, (BYTE*)&indices[0]);

	// 인접노드 라인 생성
	for (auto &node : m_naviMesh.m_naviNodes)
		for (int i = 0; i < 3; ++i)
			if (node.adjacent[i] >= 0)
				m_naviAdjLines.AddLine(renderer, node.center
					, m_naviMesh.m_naviNodes[node.adjacent[i]].center);

	return true;
}


void cNaviEdit::RenderNavigationMesh(cRenderer &renderer
	, const XMMATRIX &tm //= graphic::XMIdentity
)
{
	cShader11 *shader = renderer.m_shaderMgr.FindShader(eVertexType::POSITION);
	assert(shader);
	shader->SetTechnique("Unlit");
	shader->Begin();
	shader->BeginPass(renderer, 0);

	const XMMATRIX transform = m_transform.GetMatrixXM() * tm;
	renderer.m_cbPerFrame.m_v->mWorld = XMMatrixTranspose(transform);
	renderer.m_cbPerFrame.Update(renderer);
	renderer.m_cbLight.Update(renderer, 1);

	const Vector4 diffuse = m_color.GetColor();
	renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(diffuse.x, diffuse.y, diffuse.z, diffuse.w);
	renderer.m_cbMaterial.Update(renderer, 2);

	CommonStates states(renderer.GetDevice());
	renderer.GetDevContext()->OMSetBlendState(states.NonPremultiplied(), 0, 0xffffffff);

	m_vtxBuff.Bind(renderer);
	m_idxBuff.Bind(renderer);
	renderer.GetDevContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer.GetDevContext()->DrawIndexed(m_idxBuff.GetFaceCount()*3, 0, 0);

	renderer.GetDevContext()->OMSetBlendState(NULL, 0, 0xffffffff);

	if (m_showNaviAdjacent)
		m_naviAdjLines.Render(renderer, transform);
}


// 파일열기 다이얼로그를 띄운다.
StrPath cNaviEdit::OpenFileDialog()
{
	IFileOpenDialog *pFileOpen;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	if (SUCCEEDED(hr))
	{
		COMDLG_FILTERSPEC filter[] = {
			{ L"Path File (*.txt)", L"*.txt" }
			,{ L"All File (*.*)", L"*.*" }
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
