#pragma once
#include<vector>
#include<DirectXMath.h>

//3角形メッシュの頂点法線の計算
template<typename VertexList,typename FaceList>
auto GetVertexNormal(const VertexList& vertexList,const FaceList& faceList)
{
	using namespace DirectX;

	std::vector<XMVECTOR> normal(vertexList.size(), XMVectorSet(0.f, 0.f, 0.f, 0.f));
	std::vector<std::size_t> cnt(vertexList.size());


	XMVECTOR v[3];
	for (const auto& face : faceList)
	{
		for (std::size_t i = 0; i < 3; i++) 
			v[i] = XMVectorSet(vertexList[face[i]][0], vertexList[face[i]][1], vertexList[face[i]][2], 0.f);

		auto faceNormal = XMVector3Cross(v[1] - v[0], v[2] - v[0]);
		
		for (std::size_t i = 0; i < 3; i++) {
			normal[face[i]] += faceNormal;
			cnt[face[i]]++;
		}
	}

	for (std::size_t i = 0; i < vertexList.size(); i++) {
		if (cnt[i] > 0)
			normal[i] = XMVectorDivide(normal[i], XMVectorSet(static_cast<float>(cnt[i]), static_cast<float>(cnt[i]), static_cast<float>(cnt[i]), 0.f));
		normal[i] = XMVector3Normalize(normal[i]);
	}

	return normal;
}
