#include "Menu.hpp"

#include <string>
#include <vector>

namespace win31 {
namespace {

HMENU g_menu = nullptr;
// Parallel array of action ids matching AppendMenu item IDs (1-based).
std::vector<int> g_actionIds;

}// namespace

void ClearMenu(HWND hwnd) {
  if (hwnd && g_menu) {
    SetMenu(hwnd, nullptr);
  }
  if (g_menu) {
    DestroyMenu(g_menu);
    g_menu = nullptr;
  }
  g_actionIds.clear();
}

void ApplyMenu(HWND hwnd, std::span<const platform::MenuItem> items) {
  if (!hwnd) {
    return;
  }

  ClearMenu(hwnd);

  if (items.empty()) {
    return;
  }

  g_menu = CreateMenu();
  if (!g_menu) {
    return;
  }

  // Simple flat menu: group by a single top-level "Game" popup, plus any item
  // whose label contains a tab-separated parent name "Parent\tChild".
  // For the example we just put everything under a top-level popup named by
  // the text before the first '/', else under "Menu".
  HMENU popup = CreatePopupMenu();
  std::string popupName = "Menu";

  UINT_PTR id = 1;
  g_actionIds.clear();
  g_actionIds.push_back(0);// dummy at index 0 so ids are 1-based

  for (const auto &item: items) {
    if (item.label.empty()) {
      continue;
    }

    // Support "File/Exit" style labels: first segment is popup name.
    std::string label(item.label);
    const auto slash = label.find('/');
    std::string entry = label;
    if (slash != std::string::npos) {
      popupName = label.substr(0, slash);
      entry = label.substr(slash + 1);
    }

    if (entry == "-") {
      AppendMenuA(popup, MF_SEPARATOR, 0, nullptr);
      continue;
    }

    AppendMenuA(popup, MF_STRING | MF_ENABLED, id, entry.c_str());
    g_actionIds.push_back(item.actionId);
    ++id;
  }

  AppendMenuA(g_menu, MF_POPUP | MF_STRING, reinterpret_cast<UINT_PTR>(popup), popupName.c_str());
  SetMenu(hwnd, g_menu);
  DrawMenuBar(hwnd);
}

bool HandleMenuCommand(e00::Engine &engine, WPARAM wParam) {
  const UINT_PTR id = LOWORD(wParam);
  if (id == 0 || id >= g_actionIds.size()) {
    return false;
  }

  const int actionId = g_actionIds[id];
  if (actionId == 0) {
    // Convention: actionId 0 means Quit
    engine.QueueActionForNextTick(e00::Engine::BuiltInAction_Quit());
    return true;
  }

  // For non-zero ids, still queue quit only when matching BuiltIn quit sentinel.
  // Games can extend this later; for now treat any menu action as a logged no-op
  // unless it is 0 (quit).
  e00::GetDefaultLogger().Info(e00::source_location::current(),
                               "Menu action selected: {}", actionId);
  return true;
}

}// namespace win31
