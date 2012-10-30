//vertex shader function, takes in one vertex and outputs another
float4 VS(float4 Pos: POSITION): SV_POSITION
{
	return Pos;
}

//pixel shader function, outputs a yellow colour with Alpha 1
float4 PS(float4 Pos:SV_POSITION):SV_TARGET
{
	return float4(1.0f,1.0f,0.0f,1.0f);
}

//specifies a direct3d10 technique called render.
technique10 Render
{
	//specifies a pass called P0
	pass P0
	{
		//specifies shader model that the shaders should be compiled against and where it is impemented.
		SetVertexShader(CompileShader(vs_4_0,VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0,PS()));
	}
}