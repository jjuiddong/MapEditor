//
// 2017-06-09, jjuiddong
// Vehicle Path Mesh
//
//      + .............. + ................ + .................. + ............... +
//      |                  |                    |                     |                  |
//      |                  |                    |                     |                  |
//      + .............. + ................ + .................. + ............... +
//      |                  |                    |                     |                  |
//      |                  |                    |                     |                  |
//      + .............. + ................ + .................. + ............... +
//      |                  |                    |                     |                  |
//      |                  |                    |                     |                  |
//      + .............. + ................ + .................. + ............... +
//      |                  |                    |                     |                  |
//      |                  |                    |                     |                  |
//      + .............. + ................ + .................. + ............... +
//
//
#pragma once


namespace tms
{

	class cPathMesh
	{
	public:
		cPathMesh();
		virtual ~cPathMesh();
		
		bool Create(const int rows, const int cols);
		bool SetPos(const int row, const int col, const Vector3 &pos);
		void FindPath(const Vector2i &start, const Vector2i &end, OUT vector<Vector3> &out) const;
		Vector2i GetPosToIdx(const Vector3 &pos) const;
		void Clear();


	public:
		int m_rows;
		int m_cols;
		Vector3 **m_mesh;
	};

}
