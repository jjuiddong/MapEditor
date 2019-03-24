
#include "stdafx.h"
#include "palete.h"
#include "rack.h"

using namespace graphic;


cPalete::cPalete()
	: m_id(-1)
	, m_rack(NULL)
	, m_racker(NULL)
{
}

cPalete::~cPalete()
{
}


bool cPalete::Create(cRenderer &renderer
	, const StrId &name, const cRack *rack
	, const int row, const int col, const int index)
{
	RETV(!rack, false);

	cBoundingBox bbox;
	if (!rack->GetPosition(row, col, index, bbox))
		return false;

	m_name = name;
	m_rack = rack;
	m_boundingBox = bbox;

	const float boxWidth = bbox.GetDimension().x - 0.1f;
	cBoundingBox bbox2 = bbox;

	// 랜덤하게 박스 크기를 설정한다.
	const float rvalue = randfloat3();
	const float height = bbox.m_bbox.Extents.y * 2;
	const float newHeight = height * min(1.f, rvalue + 0.3f);
	bbox2.m_bbox.Extents.x *= 0.95f;
	bbox2.m_bbox.Extents.y = newHeight / 2.f;
	bbox2.m_bbox.Center.y -= (bbox.m_bbox.Extents.y - newHeight / 2.f);
	m_box[0].SetBox(bbox2);

	return true;
}


cBox* cPalete::Pick(const Ray &ray
	, const Matrix44 &tm //= Matrix44::Identity
)
{
	{
		cBoundingBox bbox = m_boundingBox;
		bbox *= tm;
		if (!bbox.Pick(ray))
			return NULL;
	}

	int focusBoxIndex = -1;
	vector<int> boxes;
	for (int i = 0; i < 1; ++i)
	{
		cBoundingBox bbox = m_box[i].m_boundingBox;
		bbox *= tm;
		if (bbox.Pick(ray))
		{
			focusBoxIndex = i;
			boxes.push_back(i);
		}
	}

	if (boxes.empty())
		return NULL;

	if (boxes.size() > 1)
	{
		int idx = -1;
		float minLength = FLT_MAX;

		for (auto &p : boxes)
		{
			cBoundingBox bbox = m_box[p].m_boundingBox;
			bbox *= tm;
			float len;
			if (bbox.Pick(ray, &len))
			{
				if (len < minLength)
				{
					minLength = len;
					focusBoxIndex = p;
				}
			}
		}
	}
	else
	{
		focusBoxIndex = boxes[0];
	}

	if (focusBoxIndex >= 0)
	{
		return &m_box[focusBoxIndex];
	}
	else
	{
		return NULL;
	}

	return NULL;
}

