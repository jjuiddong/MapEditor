//
// 2017-11-08, jjuiddong
// terrain loader 2
//
#pragma once


class cTerrainLoader2 : public graphic::cTerrainLoader
{
public:
	cTerrainLoader2(graphic::cTerrain *terrain);
	virtual ~cTerrainLoader2();


protected:
	virtual bool WriteNode(graphic::cNode *node
		, INOUT boost::property_tree::ptree &tree) override;

	virtual graphic::cNode* CreateNode(graphic::cRenderer &renderer
		, const boost::property_tree::ptree &tree) override;

};
