//
// 2017-11-13, jjuiddong
//	- Upgrad DX11
//
#pragma once


namespace graphic
{
	class cBox
	{
	public:
		cBox();
		virtual ~cBox();

		void SetBox(const cBoundingBox &bbox);
		void SetBox(const cBox &box);


	public:
		struct sBoxInfo
		{
			int warehouseCode;
			int floorCode;
			int zoneCode;
			int locationCode;
			int cell;
			int pallet;
			int box;
			int companyCode;
			StrId companyName;
			int merchandiseCode;
			int temperature;
			StrId stockState;
			int stockCount;
			int availableCount;
			int reserveCount;
			int holdCount;
			StrId inputDate;
			StrId makeDate;
			StrId availableDate;
			int boxUnit;
			float weight;
			cColor color;
		};

		int m_attrId;
		sBoxInfo m_info;
		cBoundingBox m_boundingBox; // Warehous Local Space Coordinate
	};

}
