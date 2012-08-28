//
//  Shader.fsh
//  xxx
//
//  Created by Li Wei on 12-8-26.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

uniform lowp vec4 u_color;

uniform sampler2D u_texture;
varying highp vec2 v_texCoord;

void main()
{
    gl_FragColor = texture2D(u_texture, v_texCoord)*u_color;
    //gl_FragColor = lwcolor;
}
