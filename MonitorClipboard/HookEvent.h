#pragma once

#include <functional>

class HookEvent
{
public:
	static HookEvent* instance();
	void start();
	void stop();
	void addMessageCallback(UINT msg, const std::function<void(WPARAM, LPARAM)> &cb);
	void clearMessageCallback();

private:
	HookEvent();
	~HookEvent();
	static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
	static HHOOK s_mouseHook;
	static HHOOK s_keyboardHook;
};

