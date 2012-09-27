//
//  Shader.vsh
//  xxx
//
//  Created by Li Wei on 12-8-26.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

attribute vec3 a_position;
attribute vec2 a_uv;

uniform mat4 u_mvpmat;

varying highp vec2 v_texCoord;

void main()
{
    gl_Position = u_mvpmat * vec4(a_position, 1.0);
    v_texCoord = a_uv;
}
