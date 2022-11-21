#include<Windows.h>
#include<iostream>
#include<chrono>
#include<vector>
#include<thread>

#define DEBUG 0

//Get current time in milliseconds.
int64_t inline GetTime()  {
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

//Variables used for timing.
int64_t t = GetTime();
int64_t _t = t;
int64_t delta;

int64_t t1 = GetTime();
int64_t _t1 = t1;
int64_t delta1;

int64_t GetDelta() {
    _t = t;
    t = GetTime();
    return (delta = (t - _t));
}

int64_t GetDelta1() {
    _t1 = t1;
    t1 = GetTime();
    return (delta1 = (t1 - _t1));
}


// List of possible keystates
enum KeyState {
    KEYDOWN,
    KEYUP,
    INVALID,
};

struct KeyPress {
    int64_t delta;
    unsigned long code;
    uint16_t state;
};

KeyPress newKeyPress(WPARAM key_state, LPARAM key_id) {
    auto key_code = (KBDLLHOOKSTRUCT*)key_id;

    KeyPress kp;
    kp.state = KEYDOWN;
    if (key_state == 0x101)
        kp.state = KEYUP;
    else if (key_state != 0x100)
        kp.state = INVALID;
    kp.code = key_code->vkCode;
    kp.delta = GetDelta();
    return kp;
}

const uint16_t MAX_KEYPRESSES = 2;

//Holds MAX_KEYPRESSES last key presses
std::vector<KeyPress> KeyPresses;

bool allow_next_186 = false;

bool await_186() {
    auto total_time = GetDelta1();
    while (total_time <= 10) {
        total_time += GetDelta1();
        for (size_t i = 0; i < KeyPresses.size(); i++) {
            if (i + 1 <= KeyPresses.size()) {
                KeyPress kp0 = KeyPresses[i];
                KeyPress kp1 = KeyPresses[i + 1];
                if (kp0.code == 186 && kp1.code == 222 && kp1.delta < 5) {
                    return 0;
                }

            }
        }
    }
    return 1;
}

/*
1. Add KeyPress to the vector.
2.

return true if the event is to pass
return false if not
*/
bool ParseKeyPress(KeyPress kp) {
    //std::cout << "New Key Press: " << kp.delta << ' ' << kp.state << ' ' << kp.code << '\n';

    if (kp.state != 0)
        return 1;
    KeyPresses.push_back(kp);
    
    if (kp.delta <= 5 &&kp.code == 219) {
        KeyPresses.clear();
        return 0;
    }

    for (size_t i = 0; i < KeyPresses.size(); i++) {
        if(i+1 <= KeyPresses.size()){
            KeyPress kp0 = KeyPresses[i];
            KeyPress kp1 = KeyPresses[i+1];
            if (kp0.code == 186 && kp1.code != 222 && kp1.delta > 5) {
                INPUT ip;

                // Set up a generic keyboard event.
                ip.type = INPUT_KEYBOARD;
                ip.ki.wScan = 0; // hardware scan code for key
                ip.ki.time = 0;
                ip.ki.dwExtraInfo = 0;

                // Press the ";" key
                ip.ki.wVk = 186; // virtual-key code for the ";" key
                ip.ki.dwFlags = 0; // 0 for key press
                SendInput(1, &ip, sizeof(INPUT));

                // Release the ";" key
                ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
                SendInput(1, &ip, sizeof(INPUT));

                KeyPresses.clear();

            }

        }
    }

    if (kp.code == 186) {
        if (allow_next_186) {
            allow_next_186 = false;
            return 1;
        }
        KeyPresses.clear();
        KeyPresses.push_back(kp);
        //186 - ;
        //0 0 222 - '
        //KeyPresses.clear();
        return 0;
    }

    //std::cout << "{\n";
    //for (size_t i = 0; i < KeyPresses.size(); i++){
    //    KeyPress _kp = KeyPresses[i];
    //    std::cout<<"  " << _kp.delta << ' ' << _kp.state << ' ' << _kp.code << "\n";
    //    //Keypresses to prevent
    //    //[ right after p
    //   
    //}
    //std::cout << "}\n";

    if (KeyPresses.size() >= MAX_KEYPRESSES)
        KeyPresses.clear();

   /* if (char(kp.code) != 'A')
        return 1;*/

    //allow the key press
    return 1;
}

int64_t proc(int code, WPARAM key_state, LPARAM key_id) {

    auto kp = newKeyPress(key_state, key_id);

    if (ParseKeyPress(kp)) {
        //return success, pass to the next hook in the chain
        return CallNextHookEx(NULL, code, key_state, key_id);
    }
    return -1; //don't pass
}

//Main Function
#if DEBUG 1
int main()
#else
auto WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) -> int
#endif
{
    /*
    *  auto run_time = GetDelta();
    auto x = -1;
    ShowWindow(GetForegroundWindow(), SW_SHOW);
    while (run_time <= (21 * 60 * 1000)) {
        run_time += GetDelta();
        std::cout << run_time << std::endl;
        Sleep(100);
        x += rand() & 8;
        std::cout << x << std::endl;
    }
    //auto stealth = FindWindowA("ConsoleWindowClass", NULL);
    //ShowWindow(GetForegroundWindow(), SW_HIDE);

    //HOOKPROC hkprcSysMsg;
    //static HINSTANCE hinstDLL;
    //static HHOOK hhookSysMsg;
    //hinstDLL = LoadLibrary(TEXT("c:\\myapp\\sysmsg.dint64_t "));
    //hkprcSysMsg = (HOOKPROC)GetProcAddress(hinstDLL, "SysMessageProc");
    //hhookSysMsg = SetWindowsHookExW(
    //    WH_KEYBOARD_LL,
    //    proc,
    //    hinstDLL,
    //    0);
    */

    auto hook = SetWindowsHookEx(WH_KEYBOARD_LL, proc, NULL, 0);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
}