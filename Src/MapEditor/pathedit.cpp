
#include "stdafx.h"
#include "pathedit.h"


cPathEdit::cPathEdit()
	: m_isEditVehiclePath(false)
	, m_editVehicleType(eEditVehicle::Edge)
	, m_selectVertex(-1)
	, m_isShowWorldPath(true)
	, m_fileName("../Media/vehiclepath.txt")
	, m_vertexSize(2.f)
	, m_offsetY(0)
{
}

cPathEdit::~cPathEdit()
{
}


bool cPathEdit::Init(graphic::cRenderer &renderer)
{
	m_lineList.Create(renderer, 1024*2);

	m_fileName = g_root.m_cfg.GetString("PathEdit-fileName", "../Media/vehiclepath.txt");
	m_vertexSize = g_root.m_cfg.GetFloat("PathEdit-vertexSize", 1.f);

	return true;
}


void cPathEdit::UpdateLineList(graphic::cRenderer &renderer)
{
	//if (!CheckVertexConnection())
	//{
	//	::MessageBoxA(NULL, "Vertex Connection Error", "Error", MB_OK);
	//}

	std::set<int> vertices; // key = vertex1 id * MAX_VERTEX + vertex2 id

	m_lineList.ClearLines();

	// 버텍스끼리 연결된 라인을 생성한다. 
	// 서로 마주보면서 연결되므로, 라인 2개를 겹치지 않게 그려야 한다. 
	// 이 때 사용되는 플래그가 isTwo 다.
	for (u_int k=0; k < m_pathFinder.m_vertices.size(); ++k)
	{
		ai::cPathFinder::sVertex &vtx = m_pathFinder.m_vertices[k];
		for (int i = 0; i < ai::cPathFinder::sVertex::MAX_EDGE; ++i)
		{
			if (vtx.edge[i].to < 0)
				break;

			const int id1 = (k * ai::cPathFinder::sVertex::MAX_VERTEX) + vtx.edge[i].to;
			const int id2 = (vtx.edge[i].to * ai::cPathFinder::sVertex::MAX_VERTEX) + k;
			const auto it1 = vertices.find(id1);
			const auto it2 = vertices.find(id2);
			const bool isTwo = (it1 != vertices.end()) || (it2 != vertices.end());

			ai::cPathFinder::sVertex &vtx2 = m_pathFinder.m_vertices[vtx.edge[i].to];

			Vector3 offset2;
			if (isTwo)
			{
				const Vector3 v = (vtx.pos - vtx2.pos).Normal();
				const Vector3 right = Vector3(0, 1, 0).CrossProduct(v).Normal();
				offset2 = right * (m_vertexSize * 0.5f);
			}

			m_lineList.AddLine(renderer
				, vtx.pos + Vector3(0, m_vertexSize + m_offsetY, 0) + offset2
				, vtx2.pos + Vector3(0, m_vertexSize + m_offsetY, 0) + offset2);

			vertices.insert(id1);
			vertices.insert(id2);
		}
	}
}


// 버텍스간 양방향 연결 체크
// 단방향으로 연결되었을 경우 false를 리턴한다.
bool cPathEdit::CheckVertexConnection(
	const bool isLog //=true
)
{
	bool checkMatch = true;
	std::set<int> vertices; // key = vertex1 id * MAX_VERTEX + vertex2 id
	for (u_int k = 0; k < m_pathFinder.m_vertices.size(); ++k)
	{
		ai::cPathFinder::sVertex &vtx = m_pathFinder.m_vertices[k];
		for (int i = 0; i < ai::cPathFinder::sVertex::MAX_EDGE; ++i)
		{
			if (vtx.edge[i].to < 0)
				break;

			const int nextId =vtx.edge[i].to;
			ai::cPathFinder::sVertex &vtx2 = m_pathFinder.m_vertices[nextId];

			bool isMatch = false;
			for (int m = 0; m < ai::cPathFinder::sVertex::MAX_EDGE; ++m)
			{
				if (vtx2.edge[m].to < 0)
					break;

				if (k == vtx2.edge[m].to)
				{
					isMatch = true;
					break;
				}
			}

			if (!isMatch)
			{
				// 에러
				// 상호참조가 안되어 있다.
				// 모든 버텍스를 검사하고 난 후, 종료한다.
				if (isLog)
					dbg::Log("Path Error!! no match %d - %d\n", k, nextId);

				checkMatch = false;
			}
		}
	}

	return checkMatch;
}


void cPathEdit::Render(graphic::cRenderer &renderer)
{
	if (ImGui::CollapsingHeader("Vehicle Path"))
	{
		ImGui::Spacing();
		ImGui::PushID(10); 
		ImGui::InputText("", m_fileName.m_str, ARRAYSIZE(m_fileName.m_str));
		ImGui::PopID();

		ImGui::SameLine(); 
		ImGui::PushID(11);
		if (ImGui::Button("Read"))
		{
			m_pathFinder.Read(m_fileName);
			UpdateLineList(renderer);
			g_root.m_terrainEditWindow->m_naviEdit.m_fileName = m_fileName;
			g_root.m_cfg.SetValue("PathEdit-fileName", m_fileName.c_str());
		}
		ImGui::PopID();

		ImGui::SameLine();
		ImGui::PushID(12);
		if (ImGui::Button("..."))
		{
			StrPath path = OpenFileDialog();
			if (!path.empty())
			{
				m_fileName = path;
				m_pathFinder.Read(path);
				UpdateLineList(renderer);
				g_root.m_terrainEditWindow->m_naviEdit.m_fileName = path;
				g_root.m_cfg.SetValue("PathEdit-fileName", path.c_str());
			}
		}
		ImGui::PopID();

		ImGui::Checkbox("Show World Path", &m_isShowWorldPath);
		ImGui::Checkbox("Edit Path", &m_isEditVehiclePath);
		ImGui::Separator();

		ImGui::RadioButton("Edit Vertex", (int*)&m_editVehicleType, eEditVehicle::Vertex);
		ImGui::SameLine();
		ImGui::RadioButton("Edit Edge", (int*)&m_editVehicleType, eEditVehicle::Edge);
		ImGui::SameLine();
		ImGui::RadioButton("Edit Destination", (int*)&m_editVehicleType, eEditVehicle::Dest);

		if (ImGui::DragFloat("Vertex Size", &m_vertexSize, 0.01f, 0, 100.f))
			g_root.m_cfg.SetValue("PathEdit-vertexSize", m_vertexSize);
		if (ImGui::DragFloat("Offset Y", &m_offsetY, 0.01f))
		{
			UpdateLineList(renderer);
		}

		ImGui::Spacing();

		if (m_selectVertex >= 0)
		{
			ImGui::Text("Select Vertex = %d", m_selectVertex);
			ImGui::SameLine(150);

			const bool isRemove = ImGui::Button("Remove");
			ai::cPathFinder::sVertex &vtx = m_pathFinder.m_vertices[m_selectVertex];

			ImGui::InputInt("type", &vtx.type);
			ImGui::DragFloat3("pos", (float*)&vtx.pos, 0.01f);
			ImGui::Spacing();

			for (int i = 0; i < ai::cPathFinder::sVertex::MAX_EDGE; ++i)
			{
				if (vtx.edge[i].to < 0)
					break;

				const ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen,
					"%d", vtx.edge[i]);

				ImGui::SameLine(100);
				if (ImGui::Button(format("Remove-%d", i).c_str())) // remove edge  (rotation left)
				{
					m_pathFinder.RemoveEdgeEachOther(m_selectVertex, vtx.edge[i].to);
					UpdateLineList(renderer);
				}
			}

			if (isRemove)
			{
				m_pathFinder.RemoveVertex(m_selectVertex);
				UpdateLineList(renderer);
				m_selectVertex = -1;
			}
		}

		if (eEditVehicle::Dest == m_editVehicleType)
		{
			ImGui::DragFloat3("Start Point (Left Click)", (float*)&m_editVehicleStartPoint, 0.01f);
			ImGui::DragFloat3("End Point (Right Click)", (float*)&m_editVehicleEndPoint, 0.01f);
		}

		if (ImGui::Button("Update Path Line"))
			UpdateLineList(renderer);

		ImGui::SameLine();

		if (ImGui::Button("Save Path"))
		{
			m_pathFinder.Write(m_fileName);
		}

		ImGui::Spacing();
	}
}


// 파일열기 다이얼로그를 띄운다.
StrPath cPathEdit::OpenFileDialog()
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
