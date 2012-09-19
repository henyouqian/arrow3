//
//  Shader.vsh
//  xxxx
//
//  Created by Li Wei on 12-9-19.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

attribute vec4 position;
attribute vec3 normal;

varying lowp vec4 colorVarying;

uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;

void main()
{
    vec3 worldNormal = normalize(normalMatrix * normal);
    vec3 lightPosition = vec3(0.3, 1.0, 0.6);
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    
    float nDotVP = max(0.0, dot(worldNormal, normalize(lightPosition)));
                 
    colorVarying = diffuseColor * nDotVP;
    gl_Position = modelViewProjectionMatrix * position;
}
