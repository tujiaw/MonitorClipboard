#include "stdafx.h"
#include "HookEvent.h"
#include "Constant.h"
#include <map>

HHOOK HookEvent::s_mouseHook = nullptr;
HHOOK HookEvent::s_keyboardHook = nullptr;
std::map<UINT, std::function<void(WPARAM, LPARAM)>> s_cbMap;
HookEvent::HookEvent()
{
}


HookEvent::~HookEvent()
{
}

HookEvent* HookEvent::instance()
{
	static HookEvent s_inst;
	return &s_inst;
}

void HookEvent::start()
{
	s_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, HookEvent::MouseProc, NULL, 0);
	if (NULL == s_mouseHook) {
		MessageBox(NULL, L"hook mouse failed", L"tip", MB_OK);
	}

	s_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, HookEvent::KeyboardProc, NULL, 0);
	if (NULL == s_keyboardHook) {
		MessageBox(NULL, L"hook keyboard failed", L"tip", MB_OK);
	}
}

void HookEvent::stop()
{
	UnhookWindowsHookEx(s_mouseHook);
	UnhookWindowsHookEx(s_keyboardHook);
}

LRESULT CALLBACK HookEvent::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	UINT msg = wParam;
	auto iter = s_cbMap.find(msg);
	if (iter != s_cbMap.end()) {
		s_cbMap[msg](wParam, lParam);
	}
	return CallNextHookEx(s_mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK HookEvent::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	UINT msg = wParam;
	auto iter = s_cbMap.find(msg);
	if (iter != s_cbMap.end()) {
		s_cbMap[msg](wParam, lParam);
	}
	return CallNextHookEx(s_keyboardHook, nCode, wParam, lParam);
}

void HookEvent::addMessageCallback(UINT msg, const std::function<void(WPARAM, LPARAM)> &cb)
{
	s_cbMap[msg] = cb;
}

void HookEvent::clearMessageCallback()
{
	s_cbMap.clear();
}