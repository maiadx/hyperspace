#include "hypch.h"
#include "Renderer.h"

void hyRenderer::Init(hyRenderAPI api, hyWindow& window)
{
	hyRenderCmd::CreateContext(api, window);
	

}

void hyRenderer::Shutdown()
{
	hyRenderCmd::DestroyContext();
}