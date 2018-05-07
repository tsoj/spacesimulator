#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.hpp"

inline void renderTexture(GLuint textureID)
{
	GLuint textureRenderProgramID = compileShaders("shader/renderTexture.vert", "shader/renderTexture.frag");

	glm::vec2 vertices[] =
	{
	  glm::vec2(-1.0, 1.0),
		glm::vec2(0.0, 0.0),

		glm::vec2(-1.0, -1.0),
		glm::vec2(0.0, 1.0),

		glm::vec2(1.0, -1.0),
		glm::vec2(1.0, 1.0),


	  glm::vec2(1.0, -1.0),
		glm::vec2(1.0, 1.0),

		glm::vec2(1.0, 1.0),
		glm::vec2(1.0, 0.0),

		glm::vec2(-1.0, 1.0),
		glm::vec2(0.0, 0.0),
	};

	glUseProgram(textureRenderProgramID);

	GLuint bufferID;
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) , vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,     																		// size
		GL_FLOAT,          												// type
		GL_FALSE,																	// normalized
		sizeof(glm::vec2)*2,											// stride
		(void*)0            											// array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  											// attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,     																		// size
		GL_FLOAT,          												// type
		GL_FALSE,																	// normalized
		sizeof(glm::vec2)*2,											// stride
		(void*)sizeof(glm::vec2)            											// array buffer offset
	);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(textureRenderProgramID, "texture"), 0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteBuffers(1, &bufferID);
	glDeleteProgram(textureRenderProgramID);
}
