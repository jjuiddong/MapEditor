//
// 2017-07-06, jjuiddong
// Hierarchy Edit
//
#pragma once


class cHierarchyWindow : public framework::cDockWindow
{
public:
	cHierarchyWindow();
	virtual ~cHierarchyWindow();

	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
	virtual void OnEventProc(const sf::Event &evt) override;
	void Clear();

	bool AddModel();
	bool AddModel(graphic::cNode *node);
};
