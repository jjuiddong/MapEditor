//
// 2017-07-19, jjuiddong
// path edit
//
#pragma once


struct eEditVehicle {
	enum Enum {
		Vertex, Edge, Dest
	};
};


class cPathEdit
{
public:
	cPathEdit();
	virtual ~cPathEdit();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer);
	void UpdateLineList(graphic::cRenderer &renderer);


protected:
	bool CheckVertexConnection(const bool isLog=true);
	StrPath OpenFileDialog();


public:
	ai::cPathFinder m_pathFinder;
	graphic::cDbgLineList m_lineList;

	StrPath m_fileName;
	bool m_isShowWorldPath;
	bool m_isEditVehiclePath;
	eEditVehicle::Enum m_editVehicleType;
	int m_selectVertex;
	Vector3 m_editVehicleStartPoint;
	Vector3 m_editVehicleEndPoint;
	float m_vertexSize;
	float m_offsetY; // default:0
};
