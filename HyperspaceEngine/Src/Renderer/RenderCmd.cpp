#include "hypch.h"
#include "Renderer/RenderCmd.h"

hyRenderAPIContext* hyRenderCmd::RenderAPIContext;

void hyRenderCmd::CreateContext(hyRenderAPI api)
{
	RenderAPIContext = hyRenderAPIContext::CreateContext(api);
	RenderAPIContext->Init();
}

void hyRenderCmd::DestroyContext()
{
	RenderAPIContext->Shutdown();
}
