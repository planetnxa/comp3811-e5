#version 430

layout( location = 0 ) uniform mat4 uProjCameraWorld;
layout( location = 1 ) uniform mat3 uNormalMatrix;

layout( location = 0 ) in vec3 iPosition;

// two inputs - position and colour
// in the vao we said that position is at location 0 and colour at location 1

layout( location = 1 ) in vec3 iColor;
// so the vao tells us that the colour coords come from location 1

layout(location=2) in vec3 iNormal;
out vec3 v2fColor;
out vec3 v2fNormal;

void main()
{
v2fNormal = normalize( uNormalMatrix * iNormal );
v2fColor = iColor;
gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );

 //official positioning
}
