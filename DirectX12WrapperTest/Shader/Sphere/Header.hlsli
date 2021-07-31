

cbuffer SceneData : register(b0) {
	matrix view;
	matrix proj;
	float3 eye;
	float3 lightDir;
};

cbuffer SphereData : register(b1){
	matrix world[2];
}

//ê[Ç≥éÊìæóp
cbuffer ViewProj : register(b2) {
	matrix upViewproj;
}


struct PSInput {
	float4 svpos : SV_POSITION;
	float4 pos :POSITION;
	float4 normal :NORMAL;
	float3 ray : RAY;
};