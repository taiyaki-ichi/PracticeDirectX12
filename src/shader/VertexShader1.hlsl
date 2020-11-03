struct Output {
	float4 pos:POSITION;
	float4 svpos:SV_POSITION;
};

Output main(float4 pos : POSITION) {
	Output output;
	output.pos = pos;
	output.svpos = pos;
	return output;
}