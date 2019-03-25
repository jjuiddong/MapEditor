
#include "stdafx.h"
#include "terrainloader2.h"

using namespace graphic;


cTerrainLoader2::cTerrainLoader2(cTerrain *terrain)
	: cTerrainLoader(terrain)
{
}

cTerrainLoader2::~cTerrainLoader2()
{
}


bool cTerrainLoader2::WriteNode(cNode *node, INOUT boost::property_tree::ptree &tree)
{
	if (cCCtv *cctv = dynamic_cast<cCCtv*>(node))
	{
		tree.put("type", eSubType::ToString(eSubType::CUBE2));
		tree.put("moviefilename", cctv->m_movieFileName.c_str());
		return true;
	}
	else if (cRacker *racker = dynamic_cast<cRacker*>(node))
	{
		tree.put("type", eSubType::ToString(eSubType::RACK));

		if (racker->m_rackes.empty())
			return true;

		cRack *rack = racker->m_rackes.front();
		auto &rackInfo = rack->m_info;

		Str64 dim;
		dim.Format("%f %f %f", rackInfo.dim.x, rackInfo.dim.y, rackInfo.dim.z);
		tree.put("dim", dim.c_str());
		tree.put("row", rackInfo.row);
		tree.put("col", rackInfo.col);
		tree.put("pillarsize", rackInfo.pillarSize);
		tree.put("beamsize", rackInfo.beamSize);

		for (int i = 0; i < rackInfo.col; ++i)
		{
			StrId width;
			width.Format("width-%d", i);
			tree.put(width.c_str(), rackInfo.width[i]);
		}

		return true;
	}

	return false;
}


cNode* cTerrainLoader2::CreateNode(cRenderer &renderer, const boost::property_tree::ptree &tree)
{
	const string type = tree.get<string>("type", eSubType::ToString(eSubType::NONE));
	if (type == eSubType::ToString(eSubType::CUBE2))
	{
		cCCtv *cctv = new cCCtv();
		cctv->Create(renderer, "CCTV");
		cctv->m_movieFileName = tree.get<string>("moviefilename");
		cctv->m_transform = ParseTransform(tree);
		return cctv;
	}
	else if (type == eSubType::ToString(eSubType::RACK))
	{
		cRacker *racker = new cRacker();
		StrId name = tree.get<string>("name", "Rack");
		racker->Create(renderer, name);
		racker->m_transform = ParseTransform(tree);

		cRack::sRackInfo rackInfo;

		std::stringstream ss1(tree.get<string>("dim"));
		ss1 >> rackInfo.dim.x >> rackInfo.dim.y >> rackInfo.dim.z;

		rackInfo.name = name;
		rackInfo.pos = Vector3(0, 0, 0);
		rackInfo.dir = Vector3(0, 0, 1);
		rackInfo.row = tree.get<int>("row", 1);
		rackInfo.col = tree.get<int>("col", 1);

		for (int i = 0; i < ARRAYSIZE(rackInfo.width); ++i)
			rackInfo.width[i] = 1.f;
		for (int i = 0; i < ARRAYSIZE(rackInfo.height); ++i)
			rackInfo.height[i] = 1.f;

		for (int i = 0; i < rackInfo.col; ++i)
		{
			StrId width;
			width.Format("width-%d", i);
			rackInfo.width[i] = tree.get<float>(width.c_str(), 1.f);
		}

		const float h = rackInfo.dim.y / (float)rackInfo.row;
		for (int i = 0; i < rackInfo.row + 1; ++i)
			rackInfo.height[i] = h;
		rackInfo.pillarSize = tree.get<float>("pillarsize", 0.05f);
		rackInfo.beamSize = tree.get<float>("beamsize", 0.08f);

		cRack *rack = new cRack();
		rack->Create(rackInfo);

		racker->AddRack(renderer, rack);
		racker->Optimize(renderer);
		return racker;
	}
	else
	{
		return __super::CreateNode(renderer, tree);
	}
}

