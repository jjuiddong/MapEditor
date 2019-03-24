
#include "stdafx.h"
#include "rack.h"

using namespace graphic;


cRack::cRack()
	: m_id(-1)
{
	m_pillars.reserve(32);
	m_beams.reserve(256);
}

cRack::~cRack()
{
	Clear();
}


bool cRack::Create(const sRackInfo &info)
// dim = width, height, depth
{
	Clear();

	if ((info.row <= 0) || (info.col <= 0))
		return false;

	m_info = info;

	Matrix44 T;
	T.SetPosition(info.pos);
	const Matrix44 tm = info.rot.GetMatrix() * T;

	m_info.dir = info.dir.MultiplyNormal(tm); // Update Direction with Rotation

	const Vector3 dim = info.dim;
	const Vector3 pos(0, 0, 0);// = info.pos;
	const int row = info.row;
	const bool isColor = info.color;

	const float horzSize = Vector3(dim.x, 0, dim.z).Length() / 2;
	const Vector3 dir = Vector3(dim.x, 0, -dim.z).Normal();
	const Vector3 dir2 = Vector3(dim.x, 0, dim.z).Normal();

	//         Y axis     
	//         |         Z axis (Back)
	//         |      /
	//         |    /
	//         | /
	//           -------------------> X axis
	//         (Front)               
	//
	//             pillar2                       pillar3
	//	           ------------------------------
	//		     /                            /
	//	       /                            /
	//	     /            + pos           /
	//	   /                            /
	//  /                             /
	// ------------------------------
	// pillar1                        pillar4

	const Vector3 pillar1 = pos - dir2 * horzSize;
	const Vector3 pillar2 = pos - dir * horzSize;
	const Vector3 pillar4 = pos + dir * horzSize;
	const Vector3 pillar3 = pos + dir2 * horzSize;
	const Vector3 toDir = (pillar4 - pillar1).Normal();
	const float pillarSize = (info.pillarSize == 0) ? 0.02f : info.pillarSize;
	const float beamSize = (info.beamSize == 0) ? 0.04f : info.beamSize;

	// Create Pillar
	Vector3 p1, p2, p3, p4;
	for (int i = 0; i < info.col+1; ++i)
	{
		if (i == 0)
		{
			p1 = pillar1;
			p2 = pillar2;
		}
		else
		{
			p1 = p1 + toDir*info.width[i-1];
			p2 = p2 + toDir*info.width[i-1];
		}

		cBoundingBox frontPillar;
		frontPillar.SetLineBoundingBox(p1, p1 + Vector3(0, dim.y, 0), pillarSize);

		cBoundingBox rearPillar;
		rearPillar.SetLineBoundingBox(p2, p2 + Vector3(0, dim.y, 0), pillarSize);

		m_pillars.push_back(frontPillar);
		m_pillars.push_back(rearPillar);
	}


	// Create Beam
	//             p2                             p3
	//	           ------------------------------
	//		     /                             /
	//	       /                             /
	//	     /            + pos            /
	//	   /                             /
	//  /                              /
	// ------------------------------
	// p1                             p4
	vector<float> height(info.col, 0);
	for (int i = 0; i < info.row; ++i)
	{
		for (int k = 0; k < info.col; ++k)
		{
			height[k] += info.height[i];

			if (k == 0)
			{
				p1 = pillar1;
				p2 = pillar2;
				p3 = pillar2 + toDir*info.width[k];
				p4 = pillar1 + toDir*info.width[k];
			}
			else
			{
				p1 = p4;
				p2 = p3;
				p3 = p2 + toDir*info.width[k];
				p4 = p1 + toDir*info.width[k];
			}

			// Front
			{
				Vector3 pt1 = p1 + Vector3(0, 1, 0) * height[k];
				Vector3 pt2 = p4 + Vector3(0, 1, 0) * height[k];

				cBoundingBox line;
				line.SetLineBoundingBox(pt1, pt2, beamSize);
				m_beams.push_back(line);
			}

			// Rear
			{
				Vector3 pt1 = p2 + Vector3(0, 1, 0) * height[k];
				Vector3 pt2 = p3 + Vector3(0, 1, 0) * height[k];

				cBoundingBox line;
				line.SetLineBoundingBox(pt1, pt2, beamSize);
				m_beams.push_back(line);
			}

			// Support Bar
			{
				Vector3 pt1 = p1 + Vector3(0, 1, 0) * height[k];
				Vector3 pt2 = p2 + Vector3(0, 1, 0) * height[k];
				Vector3 pt3 = p4 + Vector3(0, 1, 0) * height[k];
				Vector3 pt4 = p3 + Vector3(0, 1, 0) * height[k];

				cBoundingBox line1;
				line1.SetLineBoundingBox(pt1, pt2, beamSize);
				cBoundingBox line2;
				line2.SetLineBoundingBox(pt3, pt4, beamSize);

				m_beams.push_back(line1);
				m_beams.push_back(line2);
			}
		}
	}


	// Create Diagonal Bar
	//              ||               ||       /||
	//             /-----------------||----- / ||  
	// ||       /   p6               ||  p7 /  ||
	// ||    /                       ||   /    ||
	// || /	p5                    p8 || / --- /||
	// ||---------------------------- /      / ||
	// ||     /    p2                || p3 /   ||
	// ||   /                        ||  /     ||
	// ||/                           ||/       ||
	// ||----------------------------||           
	// ||   p1                   p4  ||           
	// ||                            ||
	//
	Vector3 p5, p6, p7, p8;
	height.clear();
	height.resize(info.col+1, 0);
	for (int i = 0; i < info.row; ++i)
	{
		for (int k = 0; k < info.col+1; ++k)
		{
			if (k == 0)
			{
				p1 = pillar1 + Vector3(0, 1, 0) * height[k];
				p2 = pillar2 + Vector3(0, 1, 0) * height[k];
				p3 = pillar2 + toDir*info.width[k] + Vector3(0, 1, 0) * height[k];
				p4 = pillar1 + toDir*info.width[k] + Vector3(0, 1, 0) * height[k];

				p5 = p1 + Vector3(0, 1, 0) * info.height[i];
				p6 = p2 + Vector3(0, 1, 0) * info.height[i];
				p7 = p3 + Vector3(0, 1, 0) * info.height[i];
				p8 = p4 +Vector3(0, 1, 0) * info.height[i];
			}
			else
			{
				p1 = p4;
				p2 = p3;
				p3 = p2 + toDir*info.width[k];
				p4 = p1 + toDir*info.width[k];

				p5 = p1 + Vector3(0, 1, 0) * info.height[i];
				p6 = p2 + Vector3(0, 1, 0) * info.height[i];
				p7 = p3 + Vector3(0, 1, 0) * info.height[i];
				p8 = p4 + Vector3(0, 1, 0) * info.height[i];
			}

			cBoundingBox line;
			if (0 == (i % 2))
				line.SetLineBoundingBox(p1, p6, beamSize);
			else
				line.SetLineBoundingBox(p2, p5, beamSize);

			m_beams.push_back(line);

			height[k] += info.height[i];
		}
	}

	const float totalHeight = dim.y;// +(dim.y / info.row);
	const Vector3 _min = pillar1;
	const Vector3 _max = pillar3 + Vector3(0, totalHeight, 0);
	sMinMax minMax;
	minMax._min = _min;
	minMax._max = _max;
	m_boundingBox.SetBoundingBox(minMax);
	m_boundingBox *= tm;
	m_tm = tm;

	return true;
}


void cRack::Update(const float deltaSeconds)
{
}


void cRack::Edit(const sRackInfo &info)
{
	Create(info);
}


void cRack::Clear()
{
}


bool cRack::GetPosition(const int row, const int col, const int index
	, OUT cBoundingBox &out) const
{
	// row 보다, 한 층 높은 곳 까지 쌓을 수 있다.
	if (((m_info.row+1) <= row) || (m_info.col <= col))
		return false;

	const Vector3 dim = m_info.dim;
	const Vector3 pos(0, 0, 0);
	const bool isColor = m_info.color;

	const float hsize = Vector3(dim.x, 0, dim.z).Length() / 2;
	const Vector3 dir = Vector3(dim.x, 0, -dim.z).Normal();
	const Vector3 dir2 = Vector3(dim.x, 0, dim.z).Normal();

	const Vector3 pillar1 = pos - dir2*hsize;
	const Vector3 pillar2 = pos - dir*hsize;
	const Vector3 pillar4 = pos + dir*hsize;
	const Vector3 pillar3 = pos + dir2*hsize;
	const Vector3 toDir = (pillar4 - pillar1).Normal();
	const float pillarSize = (m_info.pillarSize == 0) ? 0.05f : m_info.pillarSize;
	const float beamSize = (m_info.beamSize == 0) ? 0.08f : m_info.beamSize;

	Vector3 p1, p2, p3, p4;
	Vector3 p5, p6, p7, p8;
	vector<float> height(m_info.col, 0);

	const float minZ = pillar1.z;
	const float maxZ = pillar2.z;

	float minX = pillar1.x + pillarSize;
	for (int i = 0; i < col; ++i)
		minX += m_info.width[i];

	float minY = m_info.pos.y;
	for (int i = 0; i < row; ++i)
		minY += m_info.height[i];

	float maxX = minX;
	if (m_info.col > col + 1)
		maxX += m_info.width[col + 1];
	else
		maxX += m_info.width[col];
	maxX -= pillarSize;

	float maxY = minY;
	if (m_info.row > row + 1)
		maxY += m_info.height[row + 1];
	else
		maxY += (m_info.row == row)? m_info.height[row-1] : m_info.height[row];
	maxY -= beamSize;

	const float width = maxX - minX;

	Vector3 minP(minX + (width / 3 * index),             minY - m_info.pos.y, minZ);
	Vector3 maxP(minX + (width / 3 * index) + width / 3, maxY - m_info.pos.y, maxZ);

	sMinMax minMax;
	minMax._min = minP;
	minMax._max = maxP;
	out.SetBoundingBox(minMax);
	out *= m_tm;
	return true;
}


void cRack::MemClear()
{
	m_pillars.clear();
	m_beams.clear();
}
