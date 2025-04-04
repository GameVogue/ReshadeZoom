uniform float Toggle <
    ui_type = "slider";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 1.0;
> = 0.0;

float4 main(float4 pos : SV_Position, float2 uv : TexCoord) : SV_Target
{
    if (Toggle > 0.5)
        return float4(1, 0, 0, 1); // Red screen
    return float4(0, 0, 1, 1);     // Blue screen
}
