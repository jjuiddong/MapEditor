//
// 2017-12-05, jjuiddong
// Path Edit Window
//
#pragma once


class cPathEditWindow : public framework::cDockWindow
{
public:
	cPathEditWindow();
	virtual ~cPathEditWindow();

	bool Init(graphic::cRenderer &renderer);
	virtual void OnUpdate(const float deltaSeconds) override;
	virtual void OnRender(const float deltaSeconds) override;


protected:
	void PathFind(graphic::cRenderer &renderer);


public:
	struct eState {
		enum Enum {
			NORMAL
			, SET_STARTPOS
			, SET_ENDPOS
		};
	};

	struct eNaviType {
		enum Enum {
			ASTAR
			, NAVIMESH
		};
	};

	eState::Enum m_state;
	eNaviType::Enum m_naviType; // 0=AStar, 1:Navigation Mesh
	Vector3 m_startPos;
	Vector3 m_endPos;
	vector<Vector3> m_path1; // optimize path (navigation mesh)
	vector<int> m_nodePath;
	//vector<Vector3> m_path2; 
	graphic::cDbgLineList m_pathLines1;
	graphic::cDbgLineList m_pathLines2; // optimize path (navigation mesh)
};
