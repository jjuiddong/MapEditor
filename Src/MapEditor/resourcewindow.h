//
// 2017-07-05, jjuiddong
// Resource Window
//
#pragma once


class cResourceWindow : public framework::cDockWindow
{
public:
	cResourceWindow();
	virtual ~cResourceWindow();

	bool Init();
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;


protected:
	void UpdateResourceFile();


public:
	StrPath m_selectPath;
	vector<StrPath> m_textureFilesUTF8; // UTF-8 encoding
	vector<StrPath> m_modelFilesUTF8; // UTF-8 encoding
};
