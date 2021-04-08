#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

// shader resource handler, note that all the resource should be deleted after it is useless inorder to avoid memory leak
class Shader
{
public:
	Shader();

	unsigned int CreateVS(const char* vertexPath);

	unsigned int CreatePS(const char* fragmentPath);

	unsigned int CreateCS(const char* computePath);

	// attach shader
	inline void Attach()
	{
		for (auto shader : shaders)
			glAttachShader(program, shader);
	}

	// link shader to program
	inline void Link()
	{
		glLinkProgram(program);
		CheckCompileErrors(program, "PROGRAM");
		for (auto shader : shaders)
			glDeleteShader(shader);
		shaders.clear();
	}

	// use program
	inline void Use() 
	{ 
		glUseProgram(program); 
	}

	// maybe useless, free all the resource about shader
	void Finalize();

	// utility uniform functions
	inline void SetBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(program, name.c_str()), (int)value);
	}

	inline void SetInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(program, name.c_str()), value);
	}

	inline void SetFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(program, name.c_str()), value);
	}

	inline void SetVec2(const std::string& name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
	}

	inline void SetVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
	}

	inline void SetVec3(const std::string& name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
	}

	inline void SetVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
	}

	inline void SetVec4(const std::string& name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
	}

	inline void SetVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w);
	}

	inline void SetMat2(const std::string& name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	inline void SetMat3(const std::string& name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	inline void SetMat4(const std::string& name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// utility function for checking shader compilation/linking errors.
	void CheckCompileErrors(GLuint shader, std::string type);

private:
	// program ID
	unsigned int program;
	std::vector<unsigned int> shaders;
};