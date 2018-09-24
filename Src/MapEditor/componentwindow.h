//
// 2017-11-08, jjuiddong
// Component Window, cube, sphere, cctv, etc..
//
#pragma once


class cComponentWindow : public framework::cDockWindow
{
public:
	cComponentWindow();
	virtual ~cComponentWindow();

	bool Init();
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;
};
