//
// 2017-12-03, jjuiddong
// Navigation Mesh Editor
//
#pragma once


class cNaviEdit
{
public:
	cNaviEdit();
	virtual ~cNaviEdit();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer);
	void RenderNavigationMesh(graphic::cRenderer &renderer, const XMMATRIX &tm=graphic::XMIdentity);


protected:
	bool ReadFromPathFile(graphic::cRenderer &renderer, const char *fileName);
	StrPath OpenFileDialog();


public:
	// Navigation Mesh Information
	bool m_showNaviMesh;
	bool m_showNaviAdjacent;
	StrPath m_fileName;
	ai::cNavigationMesh m_naviMesh;
	
	// Navigation Mesh 3D Information
	Transform m_transform;
	graphic::cColor m_color;
	graphic::cVertexBuffer m_vtxBuff;
	graphic::cIndexBuffer m_idxBuff;
	graphic::cDbgLineList m_naviAdjLines;
};
