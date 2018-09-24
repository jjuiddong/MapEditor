//
// 2017-07-04, jjuiddong
// Grid Edit
//
#pragma once


class cGridEdit
{
public:
	cGridEdit();
	virtual ~cGridEdit();

	bool Init(graphic::cRenderer &renderer);
	void Render(graphic::cRenderer &renderer);
	void SelectTile(graphic::cTile *tile);


protected:
	void ChangeGrid(graphic::cRenderer &renderer, const int row, const int col);
	void ChangeTileSize();
	bool LoadFileByFomatter(graphic::cRenderer &renderer, const Str64 &fmt);


public:
	int m_row;
	int m_col;
	//float m_tileSize;
	Vector2 m_tileSize; // Width, Height
	Str64 m_fileNameFomatter; // UTF-8
	struct sTileFomatter {
		int order; // 0 = col-row, 1 = row-col
		int rowInc; // 0 = row decrement, 1 =row increment
		int colInc; // 0 = col decrement, 1 = col increment
	};
	sTileFomatter m_tileFmtOption;

	vector<bool> m_gridSelect;
	map<int, graphic::cTile*> m_tiles; // row*m_col + col, reference
	int m_selectTileIdx;
};
