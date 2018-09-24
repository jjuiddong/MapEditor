
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
		tree.put("type", GetSubtypeStr(eSubType::CUBE2));
		tree.put("moviefilename", cctv->m_movieFileName.c_str());
		return true;
	}

	return false;
}


cNode* cTerrainLoader2::CreateNode(cRenderer &renderer, const boost::property_tree::ptree &tree)
{
	const string type = tree.get<string>("type", GetSubtypeStr(eSubType::NONE));
	if (type == GetSubtypeStr(eSubType::CUBE2))
	{
		cCCtv *cctv = new cCCtv();
		cctv->Create(renderer, "CCTV");
		cctv->m_movieFileName = tree.get<string>("moviefilename");
		cctv->m_transform = ParseTransform(tree);
		return cctv;
	}
	else
	{
		return __super::CreateNode(renderer, tree);
	}
}

