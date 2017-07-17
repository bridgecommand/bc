varying vec4 vertexColor;

void main() {

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();

	vertexColor = gl_Color;

}

