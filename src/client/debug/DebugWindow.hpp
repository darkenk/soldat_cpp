#pragma once

#include <functional>
#include <vector>

class FSdlApp;
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;

class FDebugWindow
{
public:
	using TImGuiDrawFunction = std::function<void()>;

	FDebugWindow(const FDebugWindow&) = delete;
	FDebugWindow(FDebugWindow&&) = delete;
	FDebugWindow& operator=(const FDebugWindow&) = delete;
	FDebugWindow& operator=(FDebugWindow&&) = delete;
	explicit FDebugWindow(FSdlApp& InApp);
	~FDebugWindow();

	template <typename ServiceLocator>
	static void DrawStatic(TImGuiDrawFunction InFunc)
	{
		ServiceLocator::Get().DebugWindow().Draw(InFunc);
	}

	void Draw(const TImGuiDrawFunction& InFunc);
	void DrawEverything(SDL_GPUCommandBuffer* InCommandBuffer, SDL_GPUTexture* InTexture);

private:
	std::vector<TImGuiDrawFunction> PendingDrawCalls;
	FSdlApp& App;
};
