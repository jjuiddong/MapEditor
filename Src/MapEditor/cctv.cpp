
#include "stdafx.h"
#include "cctv.h"

using namespace graphic;

cCCtv::cCCtv()
	: m_movieFileName("none")
{
}

cCCtv::~cCCtv()
{
}


bool cCCtv::Create(graphic::cRenderer &renderer
	, const char *name // = "CCTV"
)
{
	__super::Create(renderer
		, (eVertexType::POSITION | eVertexType::NORMAL | eVertexType::TEXTURE0 ));
	m_name = name;
	m_texture = cResourceManager::Get()->LoadTexture(renderer, "../media/warehouse/cctv2.png");
	return true;
}


cNode* cCCtv::Clone(cRenderer &renderer) const
{
	cCCtv *clone = new cCCtv();
	clone->m_shape.m_vtxBuff.Set(renderer, m_shape.m_vtxBuff);
	clone->m_shape.m_idxBuff.Set(renderer, m_shape.m_idxBuff);
	clone->m_shape.m_vtxType = m_shape.m_vtxType;
	clone->m_texture = m_texture;
	clone->m_boundingBox = m_boundingBox;
	clone->m_transform = m_transform;
	clone->m_color = m_color;
	clone->CalcBoundingSphere();
	clone->m_movieFileName = m_movieFileName;
	return clone;
}
