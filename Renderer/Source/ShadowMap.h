#pragma once

class ShadowMap
{
public:
	ShadowMap(int width, int height);
	
	ShadowMap(const ShadowMap& rhs) = delete;
	ShadowMap operator=(const ShadowMap& rhs) = delete;

	// Return frame buffer ID
	void BuildFrameBuffer();

	inline int GetWidth() { return width; };
	inline int GetHeight() { return height; };

	inline unsigned int GetFrameBufferID() { return frameBufferID; };
	inline unsigned int GetDepthTextureID() { return depthTextrueID; };

private:
	unsigned int frameBufferID = -1;
	unsigned int depthTextrueID = -1;
	int width = 0;
	int height = 0;
};