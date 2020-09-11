#include "hypch.h"
#include "Renderer/RenderCmd.h"

hyRenderAPIContext* hyRenderCmd::RenderAPIContext;

void hyRenderCmd::CreateContext(hyRenderAPI api, hyWindow& window)
{
	RenderAPIContext = hyRenderAPIContext::CreateContext(api);
	RenderAPIContext->Init(window);
}

void hyRenderCmd::DestroyContext()
{
	RenderAPIContext->Shutdown();
}
