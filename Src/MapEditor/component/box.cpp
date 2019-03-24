
#include "stdafx.h"
#include "box.h"

using namespace graphic;


cBox::cBox() 
	: m_attrId(randint(0, 19))
{
	m_info.warehouseCode = 1;
	m_info.floorCode = 0;
	m_info.zoneCode = 0;
	m_info.locationCode = 0;
	m_info.cell = 0;
	m_info.pallet = 0;
	m_info.box;
	m_info.companyCode = m_attrId;
	m_info.companyName = format("Company%d", randint(1, 100));
	m_info.merchandiseCode = randint(0, 100000);
	m_info.temperature = randint(17, 36);
	m_info.stockState = "Wait";
	m_info.stockCount = randint(0, 100);
	m_info.availableCount = randint(0, m_info.stockCount);
	m_info.reserveCount = randint(0, 100);
	m_info.holdCount = randint(0, 10);
	m_info.inputDate = format("2017-0%d-%d", randint(1, 3), randint(1, 28));
	m_info.makeDate = format("2016-0%d-%d", randint(1, 3), randint(1, 28));;
	m_info.availableDate = m_info.inputDate;
	m_info.boxUnit = randint(10, 20);
	m_info.color = cColor::WHITE;
}

cBox::~cBox()
{
}


void cBox::SetBox(const cBoundingBox &bbox)
{
	m_boundingBox = bbox;
}


void cBox::SetBox(const cBox &box)
{
	m_attrId = box.m_attrId;
	m_info = box.m_info;
	m_boundingBox = box.m_boundingBox;
}
