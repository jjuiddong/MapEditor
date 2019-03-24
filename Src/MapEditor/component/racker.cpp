
#include "stdafx.h"
#include "palete.h"
#include "rack.h"
#include "racker.h"


using namespace graphic;

cRacker::cRacker()
	: cNode(common::GenerateId(), "racker", eNodeType::MODEL)
	, m_boxTex(NULL)
	, m_isLoad(false)
	, m_isShowBoxTexture(false)
{
}

cRacker::~cRacker()
{
	Clear();
}


bool cRacker::Create(cRenderer &renderer, const StrId &name)
{
	m_name = name;
	m_boxTex = cResourceManager::Get()->LoadTexture(renderer, "box.dds");
	m_whiteTex = cResourceManager::Get()->LoadTexture(renderer, "white.dds");

	m_wName = m_name.wstr();

	//m_renderFlags |= eRenderFlag::SHADOW;

	return true;
}


bool cRacker::AddRack(cRenderer &renderer, cRack *rack)
{
	auto it = std::find(m_rackes.begin(), m_rackes.end(), rack);
	if (m_rackes.end() != it)
		return false; // already exist

	rack->m_rackerId = m_id;

	m_rackes.push_back(rack);	

	CalculateBoundingBox();

	return true;
}


bool cRacker::AddPalete(const cRack *rack, cPalete *palete)
{
	// todo : check already exist box

	palete->m_racker = this;
	palete->m_rackId = rack->m_id;
	m_paletes.push_back(palete);
	return true;
}


bool cRacker::Update(cRenderer &renderer, const float deltaSeconds)
{
	return true;
}


bool cRacker::Render(cRenderer &renderer
	, const XMMATRIX &parentTm //= XMIdentity
	, const int flags // = 1
)
{
	RETV(!m_isEnable, false);
	RETV(!IsVisible(), false);
	RETV(((m_renderFlags & flags) != flags), false);

	RETV(!m_isLoad, false);
	
	const XMMATRIX transform = m_transform.GetMatrixXM() * parentTm;
	
	{
		cShader11 *shader = renderer.m_shaderMgr.FindShader(m_rackOptimize.m_vtxType);
		assert(shader);
		shader->SetTechnique(m_techniqueName.c_str());
		shader->Begin();
		shader->BeginPass(renderer, 0);
		renderer.m_cbClipPlane.Update(renderer, 4);

		renderer.m_cbPerFrame.m_v->mWorld = XMMatrixTranspose(transform);
		renderer.m_cbPerFrame.Update(renderer);
		renderer.m_cbLight.Update(renderer, 1);
		renderer.m_cbMaterial = m_rackMtrl.GetMaterial();
		renderer.m_cbMaterial.Update(renderer, 2);
		m_rackOptimize.Render(renderer);
	}

	{
		cShader11 *shader = renderer.m_shaderMgr.FindShader(m_boxOptimize.m_vtxType);
		assert(shader);
		shader->SetTechnique(m_techniqueName.c_str());
		shader->Begin();
		shader->BeginPass(renderer, 0);
		renderer.m_cbClipPlane.Update(renderer, 4);

		renderer.m_cbPerFrame.m_v->mWorld = XMMatrixTranspose(transform);
		renderer.m_cbPerFrame.Update(renderer);
		renderer.m_cbLight.Update(renderer, 1);
		renderer.m_cbMaterial = m_boxMtrl.GetMaterial();
		renderer.m_cbMaterial.Update(renderer, 2);

		if (m_isShowBoxTexture)
		{
			if (m_boxTex)
				m_boxTex->Bind(renderer, 0);
		}
		else
		{
			if (m_whiteTex)
				m_whiteTex->Bind(renderer, 0);
		}

		m_boxOptimize.Render(renderer);
	}

	// No Render ShadowMap
	if (!(flags & eRenderFlag::SHADOW))
	{
		const Matrix44 tm = transform;
		Transform tfm;
		tfm.pos = (m_boundingBox.Center() + Vector3(0, m_boundingBox.GetDimension().y, 0)) * tm;
		tfm.scale = m_transform.scale * 0.3f;
		renderer.m_textMgr.AddTextRender(renderer, m_id, m_wName.c_str()
			, cColor(0.f, 1.f, 0.f)
			, cColor(0.f, 0.f, 0.f)
			, BILLBOARD_TYPE::ALL_AXIS
			, tfm);
	}

	__super::Render(renderer, parentTm, flags);

	// Debugging
	//if (g_root.m_dbgWindow->m_isShowRackDirection)
	//	for (auto &p : m_rackes)
	//		p->m_direction.Render(renderer, transform);

	return true;
}


void cRacker::CalculateBoundingBox()
{
	RET(m_rackes.empty());

	cBoundingBox bbox;

	if (!m_rackes.empty())
		bbox = m_rackes.front()->m_boundingBox;
	for (auto &p : m_rackes)
		bbox += p->m_boundingBox;

	// total racks boundingbox
	m_boundingBox = bbox;

	CalcBoundingSphere();
}


cPalete* cRacker::Pick(const Ray &ray
	, const Matrix44 &tm //= Matrix44::Identity
)
{
	cBoundingBox bbox = m_boundingBox;
	bbox *= tm;
	if (!bbox.Pick(ray))
		return NULL;

	vector<cPalete*> paletes;
	for (auto &p : m_paletes)
	{
		if (p->Pick(ray, tm))
			paletes.push_back(p);
	}

	if (paletes.empty())
		return NULL;

	if (paletes.size() > 1)
	{
		cPalete *ret = NULL;
		float minLength = FLT_MAX;

		for (auto &p : paletes)
		{
			Vector3 center = p->m_boundingBox.Center();
			center = center * tm;
			const float len = ray.orig.LengthRoughly(center);
			if (len < minLength)
			{
				minLength = len;
				ret = p;
			}
		}

		return ret;
	}
	else
	{
		return paletes[0];
	}

	return NULL;
}


void cRacker::Optimize(cRenderer &renderer)
{
	RackOptimize(renderer);
	BoxOptimize3(renderer);
	m_isLoad = true;
}


// Vertex + Normal + Diffuse
void cRacker::RackOptimize(cRenderer &renderer)
{
	RET(m_rackOptimize.IsLoaded());

	m_rackOptimize.Clear();

	// Caculate Total Vertex, Index Count
	int totalVertices = 0;
	int totalFace = 0;
	for (auto &p : m_rackes)
	{
		for (auto &b : p->m_beams)
		{
			totalVertices += 24;
			totalFace += 12;
		}

		for (auto &b : p->m_pillars)
		{
			totalVertices += 24;
			totalFace += 12;
		}
	}

	if ((totalVertices <= 0) || (totalFace <= 0))
		return;

	const int vtxType = eVertexType::POSITION | eVertexType::NORMAL | eVertexType::COLOR;
	cVertexLayout vtxLayout;
	vtxLayout.Create(vtxType);

	const int posOffset = vtxLayout.GetOffset("POSITION");
	const int normOffset = vtxLayout.GetOffset("NORMAL");
	const int colorOffset = vtxLayout.GetOffset("COLOR");
	const int texOffset = vtxLayout.GetOffset("TEXCOORD");
	const int vertexStride = vtxLayout.GetVertexSize();

	vector<BYTE> vtxBuff(totalVertices*vertexStride);
	vector<WORD> idxBuff(totalFace * 3);

	BYTE *pdstVtx = (BYTE*)&vtxBuff[0];
	WORD *pdstIdx = (WORD*)&idxBuff[0];

	float val = 0.4f;
	cColor color(val, val, val, 1.f);
	Vector4 colorVal = color.GetColor();
	m_rackMtrl.Init(colorVal, colorVal, colorVal);

	int dstVtxCount = 0;
	for (auto &p : m_rackes)
	{
		for (auto &b : p->m_beams)
		{
			cBoundingBox &bbox = b;
			const Matrix44 tm = bbox.GetMatrix() * p->m_tm;
			for (int i = 0; i < 24; ++i)
			{
				if ((vtxType & eVertexType::POSITION) || (vtxType & eVertexType::POSITION_RHW))
					*(Vector3*)(pdstVtx + posOffset) = g_cubeVertices1[i] * tm;
				if (vtxType & eVertexType::NORMAL)
					*(Vector3*)(pdstVtx + normOffset) = g_cubeNormals1[i].MultiplyNormal(tm);
				if (vtxType & eVertexType::COLOR)
					*(Vector4*)(pdstVtx + colorOffset) = colorVal;

				pdstVtx += vertexStride;
			}

			for (int i = 0; i < 36; ++i)
				*pdstIdx++ = g_cubeIndices1[i] + dstVtxCount;

			dstVtxCount += 24;
		}

		for (auto &b : p->m_pillars)
		{
			cBoundingBox &bbox = b;
			const Matrix44 tm = bbox.GetMatrix() * p->m_tm;
			for (int i = 0; i < 24; ++i)
			{
				if ((vtxType & eVertexType::POSITION) || (vtxType & eVertexType::POSITION_RHW))
					*(Vector3*)(pdstVtx + posOffset) = g_cubeVertices1[i] * tm;
				if (vtxType & eVertexType::NORMAL)
					*(Vector3*)(pdstVtx + normOffset) = g_cubeNormals1[i].MultiplyNormal(tm);
				if (vtxType & eVertexType::COLOR)
					*(Vector4*)(pdstVtx + colorOffset) = colorVal;

				pdstVtx += vertexStride;
			}

			for (int i = 0; i < 36; ++i)
				*pdstIdx++ = g_cubeIndices1[i] + dstVtxCount;
				
			dstVtxCount += 24;
		}
	}

	m_rackOptimize.m_vtxBuff.Create(renderer, totalVertices, vertexStride, &vtxBuff[0]);
	m_rackOptimize.m_idxBuff.Create(renderer, totalFace, (BYTE*)&idxBuff[0]);
	m_rackOptimize.m_vtxType = vtxType;
}


// Box : cCube * 1
// Vertex + Normal + Diffuse + Texture
void cRacker::BoxOptimize3(cRenderer &renderer)
{
	RET(m_boxOptimize.IsLoaded());

	m_boxOptimize.Clear();

	// Caculate Total Vertex, Index Count
	int totalVertices = 0;
	int totalFace = 0;
	for (auto &p : m_paletes)
	{
		totalVertices += 24 * 1;
		totalFace += 12 * 1;
	}

	if ((totalVertices <= 0) || (totalFace <= 0))
		return;

	// Create Optimize Vertex
	m_boxMtrl.Init(Vector4(1, 1, 1, 1)*1.0f, Vector4(1, 1, 1, 1)*0.9f, Vector4(1, 1, 1, 1)*1.f);

	const int vtxType = eVertexType::POSITION | eVertexType::NORMAL | eVertexType::COLOR | eVertexType::TEXTURE0;
	cVertexLayout vtxLayout;
	vtxLayout.Create(vtxType);

	const int posOffset = vtxLayout.GetOffset("POSITION");
	const int normOffset = vtxLayout.GetOffset("NORMAL");
	const int colorOffset = vtxLayout.GetOffset("COLOR");
	const int texOffset = vtxLayout.GetOffset("TEXCOORD");
	const int vertexStride = vtxLayout.GetVertexSize();

	vector<BYTE> vtxBuff(totalVertices*vertexStride);
	vector<WORD> idxBuff(totalFace * 3);

	BYTE *pdstVtx = (BYTE*)&vtxBuff[0];
	WORD *pdstIdx = (WORD*)&idxBuff[0];

	int dstVtxCount = 0;
	for (auto &p : m_paletes)
	{
		for (int k = 0; k < 1; ++k)
		{
			cBoundingBox &bbox = p->m_box[k].m_boundingBox;
			const Vector4 color = p->m_box[k].m_info.color.GetColor();

			const Matrix44 tm = bbox.GetMatrix();
			for (int i = 0; i < 24; ++i)
			{
				if ((vtxType & eVertexType::POSITION) || (vtxType & eVertexType::POSITION_RHW))
					*(Vector3*)(pdstVtx + posOffset) = g_cubeVertices1[i] * tm;
				if (vtxType & eVertexType::NORMAL)
					*(Vector3*)(pdstVtx + normOffset) = g_cubeNormals1[i].MultiplyNormal(tm);
				if (vtxType & eVertexType::TEXTURE0)
					*(Vector2*)(pdstVtx + texOffset) = g_cubeUV1[i];
				if (vtxType & eVertexType::COLOR)
					*(Vector4*)(pdstVtx + colorOffset) = color;

				pdstVtx += vertexStride;
			}

			for (int i = 0; i < 36; ++i)
				*pdstIdx++ = g_cubeIndices1[i] + dstVtxCount;

			dstVtxCount += 24;
		}
	}

	m_boxOptimize.m_vtxBuff.Create(renderer, totalVertices, vertexStride, &vtxBuff[0], D3D11_USAGE_DYNAMIC);
	m_boxOptimize.m_idxBuff.Create(renderer, totalFace, (BYTE*)&idxBuff[0]);
	m_boxOptimize.m_vtxType = vtxType;
}


// todo : we need optimize this code, only update box color if change box color
void cRacker::ChangeColor(cRenderer &renderer)
{
	cVertexLayout vtxLayout;
	vtxLayout.Create(m_boxOptimize.m_vtxType);
	const int vtxType = m_boxOptimize.m_vtxType;
	const int posOffset = vtxLayout.GetOffset("POSITION");
	const int normOffset = vtxLayout.GetOffset("NORMAL");
	const int colorOffset = vtxLayout.GetOffset("COLOR");
	const int texOffset = vtxLayout.GetOffset("TEXCOORD");
	const int vertexStride = vtxLayout.GetVertexSize();

	BYTE *pdstVtx = (BYTE*)m_boxOptimize.m_vtxBuff.Lock(renderer);
	RET(!pdstVtx);

	for (auto &p : m_paletes)
	{
		for (int k = 0; k < 1; ++k)
		{
			cBoundingBox &bbox = p->m_box[k].m_boundingBox;
			const Vector4 color = p->m_box[k].m_info.color.GetColor();
			const Matrix44 tm = bbox.GetMatrix();

			for (int i = 0; i < 24; ++i)
			{
				if ((vtxType & eVertexType::POSITION) || (vtxType & eVertexType::POSITION_RHW))
					*(Vector3*)(pdstVtx + posOffset) = g_cubeVertices1[i] * tm;
				if (vtxType & eVertexType::NORMAL)
					*(Vector3*)(pdstVtx + normOffset) = g_cubeNormals1[i].MultiplyNormal(tm);
				if (vtxType & eVertexType::TEXTURE0)
					*(Vector2*)(pdstVtx + texOffset) = g_cubeUV1[i];
				if (vtxType & eVertexType::COLOR)
					*(Vector4*)(pdstVtx + colorOffset) = color;

				pdstVtx += vertexStride;
			}
		}
	}

	m_boxOptimize.m_vtxBuff.Unlock(renderer);
}


void cRacker::ClearOptimizeBuffer()
{
	m_rackOptimize.Clear();
	m_boxOptimize.Clear();
}


void cRacker::Clear()
{
	for (auto &p : m_rackes)
		delete p;
	for (auto &p : m_paletes)
		delete p;
	m_rackes.clear();
	m_paletes.clear();
	m_rackOptimize.Clear();
	m_boxOptimize.Clear();
}
