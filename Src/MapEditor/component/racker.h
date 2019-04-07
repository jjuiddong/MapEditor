//
// 2017-02-22, jjuiddong
// Managment Warehouse Rackes and Palete
//
// 2017-11-13, jjuiddng
//	- Upgrade DX11
//
#pragma once


namespace graphic
{
	class cRack;
	class cPalete;

	class cRacker : public graphic::cNode
	{
	public:
		cRacker();
		virtual ~cRacker();

		bool Create(cRenderer &renderer, const StrId &name);

		virtual bool Update(cRenderer &renderer, const float deltaSeconds) override;
		virtual bool Render(cRenderer &renderer, const XMMATRIX &parentTm = XMIdentity, const int flags = 1) override;
		virtual void Clear() override;

		bool AddRack(cRenderer &renderer, cRack *rack);
		bool AddPalete(const cRack *rack, cPalete *palete);
		cPalete* Pick(const Ray &ray, const Matrix44 &tm = Matrix44::Identity);
		void Optimize(cRenderer &renderer);
		void ChangeColor(cRenderer &renderer);
		void CalculateBoundingBox();
		void ClearOptimizeBuffer();


	protected:
		void RackOptimize(cRenderer &renderer);
		void BoxOptimize(cRenderer &renderer);


	public:
		bool m_isLoad;
		bool m_isShowBoxTexture;
		bool m_isShowText;
		float m_alpha; // alpha blending
		WStrId m_wName;
		vector<cRack*> m_rackes;
		vector<cPalete*> m_paletes;
		cMaterial m_rackMtrl;
		cMaterial m_boxMtrl;
		cMeshBuffer m_rackOptimize;
		cMeshBuffer m_boxOptimize;
		cTexture *m_boxTex; // reference
		cTexture *m_whiteTex; // reference
	};

}
