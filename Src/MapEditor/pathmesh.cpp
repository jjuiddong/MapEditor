
#include "stdafx.h"
#include "pathmesh.h"

using namespace graphic;
using namespace tms;


cPathMesh::cPathMesh()
	: m_rows(0)
	, m_cols(0)
	, m_mesh(NULL)
{
}

cPathMesh::~cPathMesh()
{
	Clear();
}


bool cPathMesh::Create(const int rows, const int cols)
{
	Clear();

	m_rows = rows;
	m_cols = cols;

	m_mesh = new Vector3*[rows];
	for (int i = 0; i < rows; ++i)
		m_mesh[i] = new Vector3[cols];

	return true;
}


bool cPathMesh::SetPos(const int row, const int col, const Vector3 &pos)
{
	if ((row >= m_rows) || (col >= m_cols))
		return false;

	m_mesh[row][col] = pos;
	return true;
}


void cPathMesh::Clear()
{
	if (m_mesh)
	{
		for (int i = 0; i < m_rows; ++i)
			delete[] m_mesh[i];
		delete[] m_mesh;
		m_mesh = NULL;
	}
}


void cPathMesh::FindPath(const Vector2i &start, const Vector2i &end
	, OUT vector<Vector3> &out) const
{
	vector<Vector2i> locates;
	locates.reserve(32);

	Vector2i cur = start;
	while (cur.x != end.x)
	{
		locates.push_back(cur);
		cur.x = (start.x > end.x) ? cur.x - 1 : cur.x + 1;
	}

	while (cur.y != end.y)
	{
		locates.push_back(cur);
		cur.y = (start.y > end.y) ? cur.y - 1 : cur.y + 1;
	}

	locates.push_back(end);

	out.reserve(32);
	for (auto &coord : locates)
		if (m_mesh)
			out.push_back(m_mesh[coord.x][coord.y]);
}


// return Most Nearest Mesh Index
Vector2i cPathMesh::GetPosToIdx(const Vector3 &pos) const
{
	float minLen = FLT_MAX;
	Vector2i idx(0, 0);

	for (int i = 0; i < m_rows; ++i)
	{
		for (int k = 0; k < m_cols; ++k)
		{
			const float len = pos.LengthRoughly(m_mesh[i][k]);
			if (len < minLen)
			{
				minLen = len;
				idx = Vector2i(i, k);
			}
		}
	}
	return idx;
}
