#include <cstring>
#include <iostream>
#include <windows.h>

// Global variables
WORD targetScanCode = 0;
bool isEnabled = true;
HHOOK mouseHook;
HHOOK keyboardHook;
INPUT inputEvent;
DWORD mainThreadId;

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && isEnabled && targetScanCode != 0) {
    if (wParam == WM_RBUTTONDOWN) {
      inputEvent.ki.wScan = targetScanCode;
      inputEvent.ki.dwFlags = KEYEVENTF_SCANCODE;
      SendInput(1, &inputEvent, sizeof(INPUT));
      return 1; // Orijinal sağ tık sinyalini yut (kameranın kilitlenmesini
                // önler)
    } else if (wParam == WM_RBUTTONUP) {
      inputEvent.ki.wScan = targetScanCode;
      inputEvent.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
      SendInput(1, &inputEvent, sizeof(INPUT));
      return 1; // Orijinal sağ tık sinyalini yut
    }
  }
  return CallNextHookEx(NULL, nCode, wParam,
                        lParam); // MSDN standartlarına göre NULL
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode >= 0 && wParam == WM_KEYDOWN) {
    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;

    if (kbd->vkCode == VK_INSERT) {
      isEnabled = !isEnabled;
      if (isEnabled)
        std::cout << "\n[+] Makro AKTIF!" << std::endl;
      else
        std::cout << "\n[-] Makro DURAKLATILDI!" << std::endl;
    } else if (kbd->vkCode == VK_END) {
      std::cout << "\n[!] Program kapatiliyor..." << std::endl;
      PostThreadMessage(mainThreadId, WM_QUIT, 0, 0);
    }
  }
  return CallNextHookEx(NULL, nCode, wParam,
                        lParam); // MSDN standartlarına göre NULL
}

void OptimizeProcess() {
  // REALTIME biraz fazla agresif, HIGH_PRIORITY_CLASS genelde yeterlidir.
  SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  // İşletim sisteminin threadi farklı çekirdeklere atayarak yaratacağı
  // Context Switch gecikmesini bitirmek için 0. çekirdeğe sabitliyoruz.
  SetProcessAffinityMask(GetCurrentProcess(), 1);
}

void SetBind() {
  std::cout << "[?] Sag tika atanacak tusa basin... " << std::flush;

  // Klavyedeki tüm tuşları temizle
  for (int i = 0; i < 256; i++)
    GetAsyncKeyState(i);

  int boundKey = 0;
  while (boundKey == 0) {
    for (int i = 8; i <= 255; i++) {
      // Mouse sağ tıkı atamasına izin verme
      if (i == VK_RBUTTON || i == VK_LBUTTON)
        continue;

      if (GetAsyncKeyState(i) & 0x8000) {
        boundKey = i;
        break;
      }
    }
    Sleep(10);
  }

  targetScanCode = MapVirtualKeyA(boundKey, MAPVK_VK_TO_VSC);

  char keyName[128] = {0};
  if (GetKeyNameTextA(targetScanCode << 16, keyName, sizeof(keyName)) &&
      strlen(keyName) > 0) {
    std::cout << keyName << " olarak atandi!" << std::endl;
  } else {
    std::cout << "Tus Kodu: " << boundKey << " olarak atandi!" << std::endl;
  }

  // Tuşun bırakılmasını bekle
  while (GetAsyncKeyState(boundKey) & 0x8000) {
    Sleep(10);
  }
}

int main() {
  // Enable ANSI Colors
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);

  SetConsoleTitleA("Vergil's Auto Bind | Premium Edition");
  mainThreadId = GetCurrentThreadId();

  OptimizeProcess();

  inputEvent.type = INPUT_KEYBOARD;
  inputEvent.ki.wVk = 0;
  inputEvent.ki.time = 0;
  inputEvent.ki.dwExtraInfo = 0;

  // ANSI Color Codes
  std::cout << "\033[1;36m";
  std::cout << "  ____       _     _             ____  _           _ "
            << std::endl;
  std::cout << " |  _ \\ ___ | |__ | | _____  __ | __ )(_)_ __   __| |"
            << std::endl;
  std::cout << " | |_) / _ \\| '_ \\| |/ _ \\ \\/ / |  _ \\| | '_ \\ / _` |"
            << std::endl;
  std::cout << " |  _ < (_) | |_) | | (_) >  <  | |_) | | | | | (_| |"
            << std::endl;
  std::cout << " |_| \\_\\___/|_.__/|_|\\___/_/\\_\\ |____/|_|_| |_|\\__,_|"
            << std::endl;
  std::cout << "\033[0m";
  std::cout << "\033[1;30m"
            << "=========================================================="
            << "\033[0m" << std::endl;
  std::cout << "\033[1;32m"
            << "      ROBLOX DINAMIK MAKRO SISTEMI - VERGIL EDITION       "
            << "\033[0m" << std::endl;
  std::cout << "\033[1;30m"
            << "=========================================================="
            << "\033[0m" << std::endl;

  SetBind();

  std::cout << "\n\033[1;33m[!] BILGILER:\033[0m" << std::endl;
  std::cout << "\033[1;37m- INSERT : Makroyu Ac / Kapat\033[0m" << std::endl;
  std::cout << "\033[1;37m- END    : Programi Kapat\033[0m" << std::endl;
  std::cout << "\n\033[1;32m[+] MAKRO AKTIF! Iyi oyunlarr\033[0m" << std::endl;

  mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
  keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(mouseHook);
  UnhookWindowsHookEx(keyboardHook);
  return 0;
}