//
// MapEditor 
//
#include "stdafx.h"

using namespace graphic;
using namespace framework;

cRoot g_root;


class cMapEditor : public framework::cGameMain2
{
public:
	cMapEditor();
	virtual ~cMapEditor();

	virtual bool OnInit() override;
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnEventProc(const sf::Event &evt) override;
};

INIT_FRAMEWORK3(cMapEditor);


cMapEditor::cMapEditor()
{
	m_windowName = L"MapEditor";
	m_isLazyMode = true;
	const float scale = 1.f;
	const RECT r = { 0, 0, (int)(1280 * scale), (int)(1024 * scale) };
	m_windowRect = r;
}

cMapEditor::~cMapEditor()
{
}


bool cMapEditor::OnInit()
{
	DragAcceptFiles(m_hWnd, TRUE);

	cResourceManager::Get()->SetMediaDirectory("../media/");
	m_renderer.m_shaderMgr.SetShaderRootDirectory("D:/Project/Common/Graphic11/shader/");

	dbg::RemoveLog();
	dbg::RemoveErrLog();
	dbg::Log("Start MapTool\n");

	GetMainLight().Init(cLight::LIGHT_DIRECTIONAL);
	const Vector3 lightPos(-2000, 5000, -1000);
	GetMainLight().SetPosition(lightPos);
	GetMainLight().SetDirection((Vector3(0, 0, 0) - lightPos).Normal());

	g_root.Init();

	const int cx = GetSystemMetrics(SM_CXSCREEN);
	const int cy = GetSystemMetrics(SM_CYSCREEN);

	m_gui.SetContext();
	m_gui.SetStyleColorsDark();

	return true;
}


void cMapEditor::OnUpdate(const float deltaSeconds)
{
	__super::OnUpdate(deltaSeconds);
}


void cMapEditor::OnRender(const float deltaSeconds)
{
	__super::OnRender(deltaSeconds);
}


void cMapEditor::OnEventProc(const sf::Event &evt)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (evt.type)
	{
	case sf::Event::KeyPressed:
		switch (evt.key.code)
		{
		case sf::Keyboard::Escape: close(); break;
		}
		break;
	}
}
