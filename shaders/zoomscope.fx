/* 
This shader is a simple combination of two open-source ReShade shaders:
https://github.com/mhgar/ReShade-Magnifier
(Which has the magnifier function)
and
https://github.com/luluco250/FXShaders/commit/982a96c6b5bda14cd7da3eeec5902af1bbca91b4
(Which has logic for mouse button controls)

The above two project adopts the MIT license.
*/
/* 
LICENSE

MIT License

Copyright (c) 2024 Northwind071

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ReShade.fxh"

#ifndef MAGNIFIER_KEY
#define MAGNIFIER_KEY 2
#endif

/* 
Modify the value of MAGNIFIER_KEY to modify the button that enables zoom. This can also be changed in ReShade in-game UI.
Common button values:
    LMB: 0
    RMB: 1
    MMB: 2
    Mouse Back (side button): 3
    Mouse Forward (side button): 4

e.g.
#define MAGNIFIER_KEY 2 --> Middle mouse activation

For more button corresponding values, please refer to the link https://github.com/luluco250/FXShaders/blob/master/Shaders/KeyCodes.fxh 
Please pay attention! You need to convert the hexadecimal number in the website to Decimal number for normal use
For example, in the website, the #define KEY_ALT 0x12, also known as the ALT button, needs to be converted from 0x12 to 18 in order to function properly. If you are not familiar with decimal conversion, you can use the built-in calculator in Windows for conversion.

This can also be changed in ReShade in-game UI.
*/

#ifndef MAGNIFIER_MODE
#define MAGNIFIER_MODE "hold"
#endif

/* 
Set MAGNIFIER_MODE as "hold" for hold mode, "toggle" for toggle mode. This can also be changed in ReShade in-game UI.

e.g.
#define MAGNIFIER_KEY 2
#define MAGNIFIER_MODE "toggle"
    --> Single click MMB to toggle magnifier.

e.g.
#define MAGNIFIER_KEY 1
#define MAGNIFIER_MODE "hold"
    --> Hold RMB to enable magnifier.

Other settings are recommended to change in ReShade in-game UI.
*/

#ifndef MAGNIFIER_DEVICE
#define MAGNIFIER_DEVICE "mousebutton"
#endif

/*
Set MAGNIFIER_DEVICE as "mousebutton" to use mouse, "key" to use keyboard.

If you decide to change device, revisit MAGNIFIER_KEY and change your key bind.
*/

uniform bool EnableMagnifier <
    source  = MAGNIFIER_DEVICE;
    keycode = MAGNIFIER_KEY;
    mode = MAGNIFIER_MODE;
    ui_label = "Enable Magnifier";
    ui_tooltip = "Enable the magnifier.";
>;

uniform float2 DrawPosition <
    ui_type = "drag";
    ui_min = 0.0; 
    ui_max = 1.0;
    ui_step = 0.001;
    ui_tooltip = "The position on your on screen where the magnifier will draw (does not work when the magnifier is set to fullscreen).";
> = float2(0.5, 0.5);

uniform float2 MagnifyPosition <
    ui_type = "drag";
    ui_min = 0.0; 
    ui_max = 1.0;
    ui_step = 0.001;
    ui_tooltip = "The position on your on screen that the magnifier will magnify (you'll probably want to leave this at (0.5, 0.5)).";
> = float2(0.5, 0.5);

uniform int Shape <
    ui_type = "combo";
    ui_items = "Circle\0Rectangle\0Fullscreen\0";
    ui_tooltip = "Choose the shape of the magnifier.";
> = 0;

uniform int Filtering <
    ui_type = "combo";
    ui_items = "Linear\0Point\0";
    ui_tooltip = "Choose either linear or no filtering for the output image.";
> = 0;

uniform float CircleRadius <
    ui_type = "drag";
    ui_min = 0.0; 
    ui_max = 1000.0;
    ui_step = 1.0;
    ui_tooltip = "The radius in pixels of the magnifier when it is drawn as a circle.";
> = 260.0;

uniform float2 RectangleHalfExtent <
    ui_type = "drag";
    ui_min = 0.0;
    ui_max = 1000.0;
    ui_step = 1.0;
    ui_tooltip = "The half size of the magnifier in pixels when it is drawn as a rectangle.";
> = float2(300.0, 200.0);

uniform float BaseZoomLevel <
    ui_type = "drag";
    ui_min = 1.0; 
    ui_max = 10.0;
    ui_step = 0.01;
    ui_tooltip = "How much the magnifier will scale things at start.";
> = 2;

static float DynamicZoomLevel = 1.0;
uniform float2 MouseWheelDelta < source = "mousewheel"; min = 0.0; max = 10.0; > = 0.0;
uniform float ZoomLevelDelta <
    ui_type = "drag";
    ui_min = 0.01;
    ui_max = 10.0;
    ui_step = 0.01;
    ui_tooltip = "How much to increase zoom with scroll wheel.";
> = 1;

uniform float CircleFeathering <
    ui_type = "drag";
    ui_min = 0.0; 
    ui_max = 1000.0;
    ui_step = 1.0;
    ui_tooltip = "Size of feathered edges in pixels, only works for the circle setting right now.";
> = 10;

uniform float MagnifierOpacity <
    ui_type = "drag";
    ui_min = 0.0; 
    ui_max = 1.0;
    ui_step = 0.001;
    ui_tooltip = "How much opacity the magnifier has.";
> = 1.0;

#define MODE_CIRCLE 0
#define MODE_RECTANGLE 1
#define MODE_FULLSCREEN 2
#define FILTER_LINEAR 0
#define FILTER_POINT 1

sampler2D pointBuffer {
    Texture   = ReShade::BackBufferTex;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = POINT;
    AddressU  = BORDER;
    AddressV  = BORDER;
};

float2 uv_to_screen(float2 uv) { return float2(uv.x * ReShade::ScreenSize.x, uv.y * ReShade::ScreenSize.y); }
bool outside_bounds(float2 p) { return p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0; }

float circle_feathering_amt(float feather_size, float2 offset, float circle_radius) {
    float magnitude = sqrt(offset.x * offset.x + offset.y * offset.y);
    float startRadius = circle_radius - feather_size;
    
    if (magnitude >= circle_radius) return 1.0;    
    if (magnitude <= startRadius) return 0.0;
    
    float falloff = (magnitude - startRadius) / feather_size;
    
    return falloff * falloff; // Squared falloff instead of linear
}

bool is_in_circle(float2 p, float2 centre, float radius) {
    return (p.x - centre.x) * (p.x - centre.x) + 
           (p.y - centre.y) * (p.y - centre.y) <=
           radius * radius;
}

bool is_in_rect(float2 p, float2 centre, float2 half_extent) {
    return abs(p.x - centre.x) <= half_extent.x && abs(p.y - centre.y) <= half_extent.y;
}


void UpdateZoom() {
    if (EnableMagnifier) {
        if (MouseWheelDelta.x > 0.0 || MouseWheelDelta.x < 0.0) {
            DynamicZoomLevel = max(min(DynamicZoomLevel + ZoomLevelDelta * MouseWheelDelta.x, 10.0), 1);
        }
    } else {
        DynamicZoomLevel = BaseZoomLevel;
    }
}

float4 PS_Magnifier(float4 pos : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
	UpdateZoom();
	
    // check mag enable
    if (!EnableMagnifier) {
        return tex2D(ReShade::BackBuffer, uv);
    }

    float scale = 1.0 / DynamicZoomLevel;
    
    // check fullcsrn if true ignore drawpos
    float2 draw_pos = Shape == MODE_FULLSCREEN ? float2(0.5, 0.5) : DrawPosition;
    
    // check pixels inside
    bool magnify = (Shape == MODE_CIRCLE && is_in_circle(uv_to_screen(uv), uv_to_screen(draw_pos), CircleRadius)) ||
                   (Shape == MODE_RECTANGLE && is_in_rect(uv_to_screen(uv), uv_to_screen(draw_pos), RectangleHalfExtent)) ||
                   (Shape == MODE_FULLSCREEN);    
    
    if (magnify) {
        float2 offset = uv - draw_pos;
        float2 take_pos = MagnifyPosition + offset * scale;
        
        if (outside_bounds(take_pos)) {
            return float4(0.0, 0.0, 0.0, 1.0);
        } else {
            float4 behind_pixel = tex2D(ReShade::BackBuffer, uv);
            float opacity = (Shape == MODE_CIRCLE ? circle_feathering_amt(CircleFeathering, uv_to_screen(offset), CircleRadius) : 0.0);
            float4 magnified_pixel = Filtering == FILTER_LINEAR ? tex2D(ReShade::BackBuffer, take_pos) : tex2D(pointBuffer, take_pos);
            float4 final_pixel = lerp(behind_pixel, lerp(magnified_pixel, behind_pixel, opacity), MagnifierOpacity);
            return final_pixel;
        }
    } else {
        return tex2D(ReShade::BackBuffer, uv);
    }
}

technique Magnifier {
    pass {
        VertexShader = PostProcessVS;
        PixelShader = PS_Magnifier;
    }
}
