//
//  Shader.vsh
//  xxxx
//
//  Created by Li Wei on 12-9-19.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

attribute vec4 position;
varying lowp vec4 colorVarying;
uniform mat4 modelViewProjectionMatrix;

void main()
{
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
    float bright = position.w;
    colorVarying = diffuseColor*bright;
    
    vec4 pos = position;
    pos.w = 1.0;
    gl_Position = modelViewProjectionMatrix * pos;
}
