// Clicker.cpp : Definiuje punkt wejścia dla aplikacji.
//

#define UNICODE
#include "pch.h"
#include "framework.h"
#include "Clicker.h"
#include "commctrl.h"
#include <string>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <ctime>

#define MAX_LOADSTRING  100

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Zmienne globalne:
HINSTANCE hInst;                                    // Bieżące wystąpienie
WCHAR szTitle[MAX_LOADSTRING];                      // Tekst paska tytułu
WCHAR szWindowClass[MAX_LOADSTRING];                // Nazwa klasy okna głównego
WCHAR szTitleB[MAX_LOADSTRING];                     // Tekst paska tytułu
WCHAR szWindowClassB[MAX_LOADSTRING];               // Nazwa klasy okna planszy
WCHAR szTitleN[MAX_LOADSTRING];                     // Tekst paska tytułu
WCHAR szWindowClassN[MAX_LOADSTRING];               // Nazwa klasy okna zapytania o nazwę
HWND CurrClicked = NULL;                            // Currently clicked Radio Button
HWND hwndMain;                                      // Handler for Main Window
HWND hwndBoard;                                     // Handler for Board Window
HWND hwndName;                                      // Handler for Name Input Window
int boardSize = 2;                                  // Size of the Buttons Board from Combo Box
int level;                                          // Level being increment after each correct click
int windowSize;
HANDLE newThreadHandle;
unsigned int pbRange;
bool firstRun = false;
HBITMAP hbmCircleGrey, hbmCircleGreen, hbmCircleOrange, hbmCircleRed, hbmCircleGreenL, hbmCircleOrangeL, hbmCircleRedL; //, hbmRadioGreen
HBITMAP hbmBackArrow;
HBITMAP hbmBelt;
bool btnClicked = false;
std::chrono::steady_clock::time_point startTime;    // Timer starts when level starts
std::chrono::steady_clock::time_point endTime;      // Timer ends when Radio Button is clicked
std::chrono::duration<float> timeCurrent, timeFastest, timeTotal;
int timerCount = 0;
HWND hCircle1, hCircle2, hCircle3;
HWND hBackArrow;
int swapPosition;
bool endPlay;

// Przekaż dalej deklaracje funkcji dołączone w tym module kodu:
ATOM                    MyRegisterClass(HINSTANCE hInstance);
ATOM                    MyRegisterClassB(HINSTANCE hInstance);
ATOM                    MyRegisterClassN(HINSTANCE hInstance);
BOOL                    InitInstance(HINSTANCE, int);
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK        WndProcB(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK        WndProcN(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK        About(HWND, UINT, WPARAM, LPARAM);
void                    AddMainControls();
void                    SetGlobalSize();
void                    AddBoardControls();
BOOL                    OpenBoardWindow();
void                    OpenNameWindow();
void                    SelectRandomButton();
void                    SetLevelControls();
unsigned int __stdcall  StartTimers(void*);
void                    ResetCircles();
void                    LoadBoardImages();
void                    LoadMainImages();
void                    SetTimeControls();
std::string             TimeToString(std::chrono::duration<float>, int);
void                    SetScores();
int                     CheckScores();
void                    SwapScores();
std::string             GetName();
void                    ResetGame();
void                    LoadLightCircles();
void                    EndMatch();

class Scores
{
public:
    std::string name;
    std::string level;
    std::string total;
    std::string fast;
    std::string average;
    Scores()
    {
        name = "_ _ _ _ _";
        level = "_ _";
        total = "0.000s";
        fast = "0.000s";
        average = "0.000s";
        //MessageBoxA(NULL, name.c_str(), "testx", MB_OK);
    };
    Scores(int pos)
    {
        std::ifstream hFile("Scores\\Scores.txt");
        int iterStop = 25 * (boardSize - 2) + 5 * pos;
        char lineNameTime[7];
        char lineLevel[3];

        // Finding starting position
        for (int i = 0; i < iterStop; i++)
        {
            hFile.getline(lineNameTime, 7);
        }

        // Name
        hFile.getline(lineNameTime, 6);
        name = lineNameTime;
        if (name == "_____")
        {
            name = "_ _ _ _ _";
        }

        // Level
        hFile.getline(lineLevel, 3);
        level = lineLevel;
        if (level == "__")
        {
            level = "_ _";
        }
        else if (lineLevel[0] == '0')
        {
            level = lineLevel[1];
        }

        // Total
        hFile.getline(lineNameTime, 7);
        total = lineNameTime;
        total = total + "s";
        
        // Fast
        hFile.getline(lineNameTime, 7);
        fast = lineNameTime;
        fast = fast + "s";

        // Average
        hFile.getline(lineNameTime, 7);
        average = lineNameTime;
        average = average + "s";

        hFile.close();
    };
    ~Scores() {};

private:

};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: W tym miejscu umieść kod.
    
    // Inicjuj ciągi globalne
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLICKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    LoadStringW(hInstance, IDS_BOARD_TITLE, szTitleB, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BOARD, szWindowClassB, MAX_LOADSTRING);
    MyRegisterClassB(hInstance);
    LoadStringW(hInstance, IDS_NAME_TITLE, szTitleN, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NAME, szWindowClassN, MAX_LOADSTRING);
    MyRegisterClassN(hInstance);

    // Wykonaj inicjowanie aplikacji:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLICKER));

    MSG msg;

    // Główna pętla komunikatów:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNKCJA: MyRegisterClass()
//
//  PRZEZNACZENIE: Rejestruje klasę okna.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLICKER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLICKER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterClassB(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcB;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClassB;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));

    return RegisterClassExW(&wcex);
}

ATOM MyRegisterClassN(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcN;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClassN;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));

    return RegisterClassExW(&wcex);
}

//
//   FUNKCJA: InitInstance(HINSTANCE, int)
//
//   PRZEZNACZENIE: Zapisuje dojście wystąpienia i tworzy okno główne
//
//   KOMENTARZE:
//
//        W tej funkcji dojście wystąpienia jest zapisywane w zmiennej globalnej i
//        jest tworzone i wyświetlane okno główne programu.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej

   hwndMain = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hwndMain)
   {
      return FALSE;
   }

   ShowWindow(hwndMain, nCmdShow);
   UpdateWindow(hwndMain);
   SetGlobalSize();
   LoadMainImages();
   AddMainControls();

   return TRUE;
}

//
//  FUNKCJA: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PRZEZNACZENIE: Przetwarza komunikaty dla okna głównego.
//
//  WM_COMMAND  - przetwarzaj menu aplikacji
//  WM_PAINT    - Maluj okno główne
//  WM_DESTROY  - opublikuj komunikat o wyjściu i wróć
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            // Analizuj zaznaczenia menu:
            if (HIWORD(wParam) == CBN_SELENDOK)
            {
                SetGlobalSize();
                SetScores();
            }

            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_HOWTO:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_HOWTOBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case BTN_START:
                level = 1;
                endPlay = false;
                OpenBoardWindow();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 500;
            lpMMI->ptMinTrackSize.y = 500;
            lpMMI->ptMaxTrackSize.x = 500;
            lpMMI->ptMaxTrackSize.y = 500;
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            return (INT_PTR)hBrush;
        }
        break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::string texta;
    switch (message)
    {
    case WM_COMMAND:
        {    
            int wmId = LOWORD(wParam);
            // Analizuj zaznaczenia menu:


            if ((wmId >= IDC_RBTN) && (wmId < IDC_RBTN+100))
            {
                if (btnClicked == false)
                {
                    endTime = std::chrono::high_resolution_clock::now();
                    timeCurrent = endTime - startTime;

                    btnClicked = true;

                    KillTimer(hwndBoard, IDT_TIMER);
                    KillTimer(hwndBoard, IDT_TIMER1);
                    KillTimer(hwndBoard, IDT_TIMER2);
                    KillTimer(hwndBoard, IDT_TIMER3);

                    HWND tmpBtn = GetDlgItem(hWnd, wmId);

                    if (tmpBtn == CurrClicked)
                    {
                        LoadLightCircles();
                        SetTimeControls();
                        level++;
                    }
                    else
                    {
                        EndMatch();
                        /*
                        endPlay = true;
                        swapPosition = CheckScores();
                        if (swapPosition < 0)
                        {
                            MessageBoxA(NULL, "You're Lame!", "Looser", MB_OK);
                        }
                        else
                        {
                            OpenNameWindow();
                        }
                        */
                    }
                }
            }
            
            switch (wmId)
            {
            //case IDM_EXIT:
            //    DestroyWindow(hWnd);
            //    break;
            case BTN_NEXT:
            {
                if (btnClicked || level == 1)
                {
                    if (endPlay)
                    {
                        if (MessageBox(hWnd, L"It's done. Do you want to reset the game?", L"Give yourself another chance", MB_OKCANCEL) == IDOK)
                        {
                            ResetGame();
                        }
                    }
                    else
                    {
                        SetLevelControls();
                        SelectRandomButton();

                        ResetCircles();
                        newThreadHandle = (HANDLE)_beginthreadex(0, 0, &StartTimers, 0, 0, 0);
                        startTime = std::chrono::high_resolution_clock::now();

                    }

                    btnClicked = false;
                }
            }
            break;
            case BTN_RESET:
            {
                if (endPlay == false)
                {
                    if (MessageBox(hWnd, L"But your'e still playing.\nDo you really want to give up?", L"Don't give up", MB_OKCANCEL) == IDOK)
                    {
                        ResetGame();
                    }
                }
                else
                {
                    ResetGame();
                }
            }
            break;
            case BTN_BACK:
            {
                bool goBack = false;

                if (endPlay == false && level != 1)
                {
                    if (MessageBox(hWnd, L"But your'e still playing.\nDo you really want to give up?", L"Don't give up", MB_OKCANCEL) == IDOK)
                    {
                        goBack = true;
                    }
                }
                else
                {
                    goBack = true;
                }

                if (goBack)
                {
                    SetScores();
                    timeCurrent = std::chrono::duration<float>(0.000);
                    timeFastest = std::chrono::duration<float>(0.000);
                    timeTotal = std::chrono::duration<float>(0.000);
                    DestroyWindow(hWnd);
                } 
            }
            break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        {
            if (timerCount == 1)
            {
                SendMessageW(hCircle1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGreen);
                KillTimer(hwndBoard, IDT_TIMER1);
                timerCount = 2;
            }
            else if(timerCount == 2)
            {
                KillTimer(hwndBoard, IDT_TIMER2);
                SendMessageW(hCircle2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleOrange);
                timerCount = 3;
            }
            else if (timerCount == 3)
            {
                KillTimer(hwndBoard, IDT_TIMER3);
                SendMessageW(hCircle3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleRed);
                timerCount = 4;
            }
            else
            {
                EndMatch();
                /*
                KillTimer(hwndBoard, IDT_TIMER);
                endPlay = true;

                swapPosition = CheckScores();

                if (swapPosition < 0)
                {
                    MessageBoxA(NULL, "Not this time lame-o", "Looser", MB_OK);
                }
                else
                {
                    MessageBoxA(NULL, "Future generations will remember you", "Wow, so fast!", MB_OK);
                    OpenNameWindow();
                }
                */
            }

        }
        break;
    case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 500 + (boardSize - 2) * 20;
            lpMMI->ptMinTrackSize.y = 500 + (boardSize - 2) * 20;
            lpMMI->ptMaxTrackSize.x = 500 + (boardSize - 2) * 20;
            lpMMI->ptMaxTrackSize.y = 500 + (boardSize - 2) * 20;
        }
        break;
    case WM_CTLCOLORBTN:
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkColor(hdcStatic, RGB(255, 255, 255));
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            return (INT_PTR)hBrush;
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            //HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            //return (INT_PTR)hBrush;
            if (GetDlgCtrlID((HWND)lParam) == 511) // Target the specific component
            {
                HDC hdcStatic = (HDC)wParam; // or obtain the static handle in some other way
                //SetTextColor(hdcStatic, RGB(255, 0, 0)); // text color
                SetBkColor(hdcStatic, RGB(0, 0, 210));
                
                //HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 210));
                //return (INT_PTR)hBrush;
                //return (LRESULT)GetStockObject(NULL_BRUSH);
            }
        }
        break;
    case WM_CLOSE:
        {
            SetScores();
            timeCurrent = std::chrono::duration<float>(0.000);
            timeFastest = std::chrono::duration<float>(0.000);
            timeTotal = std::chrono::duration<float>(0.000);
            DestroyWindow(hWnd);
        }
        break;
    //case WM_DESTROY:
    //    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Analizuj zaznaczenia menu:
        std::string t;
        
        switch (wmId)
        {
        case EDT_NAME:
            
            break;
        case BTN_CONFIRM:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 200;
        lpMMI->ptMinTrackSize.y = 200;
        lpMMI->ptMaxTrackSize.x = 200;
        lpMMI->ptMaxTrackSize.y = 200;
    }
    break;
    case WM_CTLCOLORSTATIC:
    {
        if ((HWND)lParam == GetDlgItem(hWnd, EDT_NAME))
        {
            SetBkMode((HDC)wParam, TRANSPARENT);
            SetTextColor((HDC)wParam, RGB(255, 255, 255));
            return (LRESULT)((HBRUSH)GetStockObject(BLACK_BRUSH));
        }
        else
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CLOSE:
    {
        if (GetName() == "J.DOE")
        {
            if (MessageBox(hWnd, L"Hold on Baby Bull.\nDo You really want to quit without putting your name?", L"Don't be so shy", MB_OKCANCEL) == IDOK)
            {
                DestroyWindow(hWnd);
            }
        }
        else
        {
            SwapScores();
            DestroyWindow(hWnd);
        }
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Procedura obsługi komunikatów dla okna informacji o programie.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void AddMainControls()
{   
    HWND tmp;
//================== LABEL CHOOSE ===========================================================================================================================================
    tmp = CreateWindowW(L"static", L"CHOOSE BOARD SIZE", WS_VISIBLE | WS_CHILD | SS_CENTER,
        150, 40, 200, 20, hwndMain, NULL, nullptr, nullptr);

//============== COMBO BOX ===========================================================================================================================================
    tmp = CreateWindowW(L"ComboBox", nullptr, CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
                                      170, 60, 160, 100, hwndMain, (HMENU)CBX_SIZE, nullptr, nullptr);
 
    SendMessage(tmp, (UINT)CB_ADDSTRING, (WPARAM)0, reinterpret_cast<LPARAM>(_T("      2 X 2")));
    SendMessage(tmp, (UINT)CB_ADDSTRING, (WPARAM)0, reinterpret_cast<LPARAM>(_T("      3 X 3")));
    SendMessage(tmp, (UINT)CB_ADDSTRING, (WPARAM)0, reinterpret_cast<LPARAM>(_T("      4 X 4")));
    SendMessage(tmp, (UINT)CB_ADDSTRING, (WPARAM)0, reinterpret_cast<LPARAM>(_T("      5 X 5")));
    SendMessage(tmp, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

//============== BUTTON START ===============================================================================================================================================
    tmp = CreateWindowW(L"Button",L"START",WS_VISIBLE | WS_CHILD,
                                    150,120,200,50,hwndMain,(HMENU)BTN_START, nullptr, nullptr);

//============== HIGH SCORES LABELS ===============================================================================================================================================

  //------- HEADERS --------------
    tmp = CreateWindowW(L"static", L"HIGH SCORES", WS_VISIBLE | WS_CHILD | SS_RIGHT,
        40, 235, 100, 20, hwndMain, NULL, nullptr, nullptr);
    tmp = CreateWindowW(L"static", L"NAME", WS_VISIBLE | WS_CHILD | SS_LEFT,
        75, 260, 50, 20, hwndMain, NULL, nullptr, nullptr);
    tmp = CreateWindowW(L"static", L"LVL", WS_VISIBLE | WS_CHILD | SS_LEFT,
        160, 260, 30, 20, hwndMain, NULL, nullptr, nullptr);
    tmp = CreateWindowW(L"static", L"TOTAL", WS_VISIBLE | WS_CHILD | SS_LEFT,
        200, 260, 40, 20, hwndMain, NULL, nullptr, nullptr);
    tmp = CreateWindowW(L"static", L"FASTEST", WS_VISIBLE | WS_CHILD | SS_LEFT,
        270, 260, 75, 20, hwndMain, NULL, nullptr, nullptr);
    tmp = CreateWindowW(L"static", L"AVERAGE", WS_VISIBLE | WS_CHILD | SS_LEFT,
        340, 260, 75, 20, hwndMain, NULL, nullptr, nullptr);

  //------- CHAMPION --------------
    tmp = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP,
        25, 280, 40, 20, hwndMain, NULL, NULL, NULL);
    SendMessageW(tmp, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmBelt);

    tmp = CreateWindowW(L"edit", L"_ _ _ _ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        75, 280, 70, 20, hwndMain, (HMENU)EDT_HS0NAME, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        160, 280, 30, 20, hwndMain, (HMENU)EDT_HS0LVL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        200, 280, 55, 20, hwndMain, (HMENU)EDT_HS0TOTAL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        270, 280, 55, 20, hwndMain, (HMENU)EDT_HS0FAST, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        340, 280, 55, 20, hwndMain, (HMENU)EDT_HS0AVG, nullptr, nullptr);


  //------- 1. --------------
    tmp = CreateWindowW(L"static", L"1.", WS_VISIBLE | WS_CHILD | SS_CENTER,
        35, 310, 20, 20, hwndMain, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _ _ _ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        75, 310, 70, 20, hwndMain, (HMENU)EDT_HS1NAME, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        160, 310, 30, 20, hwndMain, (HMENU)EDT_HS1LVL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        200, 310, 55, 20, hwndMain, (HMENU)EDT_HS1TOTAL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        270, 310, 55, 20, hwndMain, (HMENU)EDT_HS1FAST, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        340, 310, 55, 20, hwndMain, (HMENU)EDT_HS1AVG, nullptr, nullptr);

  //------- 2. --------------
    tmp = CreateWindowW(L"static", L"2.", WS_VISIBLE | WS_CHILD | SS_CENTER,
        35, 340, 20, 20, hwndMain, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _ _ _ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        75, 340, 70, 20, hwndMain, (HMENU)EDT_HS2NAME, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        160, 340, 30, 20, hwndMain, (HMENU)EDT_HS2LVL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        200, 340, 55, 20, hwndMain, (HMENU)EDT_HS2TOTAL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        270, 340, 55, 20, hwndMain, (HMENU)EDT_HS2FAST, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        340, 340, 55, 20, hwndMain, (HMENU)EDT_HS2AVG, nullptr, nullptr);

  //------- 3. --------------
    tmp = CreateWindowW(L"static", L"3.", WS_VISIBLE | WS_CHILD | SS_CENTER,
        35, 370, 20, 20, hwndMain, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _ _ _ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        75, 370, 70, 20, hwndMain, (HMENU)EDT_HS3NAME, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        160, 370, 30, 20, hwndMain, (HMENU)EDT_HS3LVL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        200, 370, 55, 20, hwndMain, (HMENU)EDT_HS3TOTAL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        270, 370, 55, 20, hwndMain, (HMENU)EDT_HS3FAST, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        340, 370, 55, 20, hwndMain, (HMENU)EDT_HS3AVG, nullptr, nullptr);

  //------- 4. --------------
    tmp = CreateWindowW(L"static", L"4.", WS_VISIBLE | WS_CHILD | SS_CENTER,
        35, 400, 20, 20, hwndMain, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _ _ _ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        75, 400, 70, 20, hwndMain, (HMENU)EDT_HS4NAME, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"_ _", WS_VISIBLE | WS_CHILD | ES_READONLY,
        160, 400, 30, 20, hwndMain, (HMENU)EDT_HS4LVL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        200, 400, 55, 20, hwndMain, (HMENU)EDT_HS4TOTAL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        270, 400, 55, 20, hwndMain, (HMENU)EDT_HS4FAST, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        340, 400, 55, 20, hwndMain, (HMENU)EDT_HS4AVG, nullptr, nullptr);

    SetScores();
}

void SetGlobalSize()
{
    HWND lblSize = GetDlgItem(hwndMain, CBX_SIZE);
    int idxRow = SendMessage(lblSize, CB_GETCURSEL, 0, 0);

    boardSize = idxRow + 2;
    windowSize = 486 + (boardSize - 2) * 20;
}

void AddBoardControls()
{
    int x, y;
    HWND tmp;

    x = 50;
    y = windowSize / 20;

//========= BUTTON \ ARROW BACK ==============================================================================================================================

    hBackArrow = CreateWindowW(L"Button", nullptr, WS_VISIBLE | WS_CHILD | BS_BITMAP,
        20, 10, 80, 35, hwndBoard, (HMENU)BTN_BACK, nullptr, nullptr);
    SendMessageW(hBackArrow, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmBackArrow);

//======== LABEL LEVEL ============================================================================================================================
    tmp = CreateWindowW(L"static", L"LEVEL", WS_VISIBLE | WS_CHILD | SS_CENTER,
        (windowSize / 2) - 35, y , 50, 20, hwndBoard, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"1", WS_VISIBLE | WS_CHILD | ES_READONLY,
        (windowSize / 2) + 15, y, 20, 20, hwndBoard, (HMENU)EDT_LEVEL, nullptr, nullptr);

    y += 50;

//========= CIRCLES ==========================================================================================================================================
    hCircle1 = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, (windowSize - 190) / 2, y, 50, 50, hwndBoard, NULL, NULL, NULL);
    SendMessageW(hCircle1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);

    hCircle2 = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, ((windowSize - 190) / 2) + 70, y, 50, 50, hwndBoard, NULL, NULL, NULL);
    SendMessageW(hCircle2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);

    hCircle3 = CreateWindowW(L"Static", NULL, WS_VISIBLE | WS_CHILD | SS_BITMAP, ((windowSize - 190) / 2) + 140, y, 50, 50, hwndBoard, NULL, NULL, NULL);
    SendMessageW(hCircle3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
    
    y += 70;
//======== RADIO BUTTONS =========================================================================================================================
    x = (windowSize - (2 * boardSize - 1)*15) / 2;

    for (int i = 1; i <= boardSize; i++)
    {
        for (int j = 1; j <= boardSize; j++)
        {
            std::string name = std::to_string(i) + std::to_string(j);

            tmp = CreateWindowW(L"Button", (LPCTSTR)name.c_str(), WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                                           x + (j - 1)* 30, y + (i - 1) * 30, 15, 15, hwndBoard, (HMENU)(IDC_RBTN + 10 * i + j), nullptr, nullptr);
        }
    }

    y += 30 * boardSize + 20;
//========= BUTTON NEXT ==============================================================================================================================

    tmp = CreateWindowW(L"Button", L"NEXT", WS_VISIBLE | WS_CHILD,
        (windowSize - 100) / 2, y, 100, 50, hwndBoard, (HMENU)BTN_NEXT, nullptr, nullptr);

    y = windowSize - 150;

//========= BUTTON RESET ==============================================================================================================================

    tmp = CreateWindowW(L"Button", L"RESET", WS_VISIBLE | WS_CHILD,
        ((windowSize - 100) / 2) + 110, y, 80, 50, hwndBoard, (HMENU)BTN_RESET, nullptr, nullptr);

   //y = windowSize - 150;

//========= LABEL CURRENT ============================================================================================================================
    x = 50;
    
    tmp = CreateWindowW(L"static", L"CURRENT", WS_VISIBLE | WS_CHILD | SS_RIGHT,
        x, y, 80, 20, hwndBoard, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        x + 85, y, 60, 20, hwndBoard, (HMENU)EDT_CUR, nullptr, nullptr);

    y += 25;

//========= LABEL FASTEST =============================================================================================================================
    tmp = CreateWindowW(L"static", L"FASTEST", WS_VISIBLE | WS_CHILD | SS_RIGHT,
        x, y, 80, 20, hwndBoard, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        x + 85, y, 60, 20, hwndBoard, (HMENU)EDT_FST, nullptr, nullptr);

    y += 25;

//========= LABEL AVERAGE ==================================================================================================================================
    tmp = CreateWindowW(L"static", L"AVERAGE", WS_VISIBLE | WS_CHILD | SS_RIGHT,
        x, y, 80, 20, hwndBoard, NULL, nullptr, nullptr);

    tmp = CreateWindowW(L"edit", L"0.000s", WS_VISIBLE | WS_CHILD | ES_READONLY,
        x + 85, y, 60, 20, hwndBoard, (HMENU)EDT_AVG, nullptr, nullptr);

    y += 25;

}

BOOL OpenBoardWindow()
{

    hwndBoard = CreateWindowW(szWindowClassB, szTitleB, WS_OVERLAPPEDWINDOW, 200, 200, 500 + (boardSize - 2) * 20, 500 + (boardSize - 2) * 20, nullptr, nullptr, hInst, nullptr);

    LONG lStyle = GetWindowLong(hwndBoard, GWL_STYLE);
    lStyle &= ~(WS_MAXIMIZEBOX);
    SetWindowLong(hwndBoard, GWL_STYLE, lStyle);

    if (!hwndBoard)
    {
        return FALSE;
    }
    
    LoadBoardImages();
    AddBoardControls();
    swapPosition = -1;

    ShowWindow(hwndBoard, 5);

    RECT rcClient;
    GetClientRect(hwndBoard, &rcClient);
    pbRange = (rcClient.right - rcClient.left);
}

void OpenNameWindow()
{ 
    hwndName = CreateWindowW(szWindowClassN, szTitleN, WS_OVERLAPPEDWINDOW, 200, 200, 200, 200, nullptr, nullptr, hInst, nullptr);
    
    LONG lStyle = GetWindowLong(hwndName, GWL_STYLE);
    lStyle &= ~(WS_MAXIMIZEBOX);
    SetWindowLong(hwndName, GWL_STYLE, lStyle);

    HWND tmpControl;
    
    tmpControl = CreateWindowW(L"static", L"Type your name:", WS_VISIBLE | WS_CHILD | SS_LEFT,
        25, 20, 300, 20, hwndName, NULL, nullptr, nullptr);
    
    tmpControl = CreateWindowW(L"Button", L"Confirm", WS_VISIBLE | WS_CHILD,
        40, 80, 80, 40, hwndName, (HMENU)BTN_CONFIRM, nullptr, nullptr);
    
    tmpControl = CreateWindowW(L"edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        40, 50, 80, 20, hwndName, (HMENU)EDT_NAME, nullptr, nullptr);
    
    SendMessage(tmpControl, EM_SETLIMITTEXT, 5, 0);
   
    SetFocus(tmpControl);
   
    ShowWindow(hwndName, 5);
    
}

void SelectRandomButton()
{
    srand((int)time(0));
    int rXNumber = (rand() % boardSize) + 1;
    int rYNumber = (rand() % boardSize) + 1;


    if (CurrClicked != NULL)
    {
        SendMessage(CurrClicked, BM_SETCHECK, BST_UNCHECKED, 1);
    }

    CurrClicked = GetDlgItem(hwndBoard, 500 + rXNumber*10 + rYNumber);
    SendMessage(CurrClicked, BM_SETCHECK, BST_CHECKED, 1);
}

unsigned int __stdcall StartTimers(void* data)
{
    LRESULT iPos = 0;
    LRESULT tmp = 2;
    std::string texta;
    int msTimer;
    int msTimerCircle;


    msTimer = ((pbRange / level) - level) * 16 + pbRange;
    msTimerCircle = msTimer / 4;

    SetTimer(hwndBoard, IDT_TIMER, msTimer, NULL);

    SetTimer(hwndBoard, IDT_TIMER1, msTimerCircle, NULL);
    SetTimer(hwndBoard, IDT_TIMER2, 2 * msTimerCircle, NULL);
    SetTimer(hwndBoard, IDT_TIMER3, 3 * msTimerCircle, NULL);
    timerCount = 1;

    return 0;
}

void ResetCircles()
{
    if (firstRun)
    {
        SendMessageW(hCircle3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
        SendMessageW(hCircle2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
        SendMessageW(hCircle1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
    }
    firstRun = true;
}

void SetLevelControls()
{
    HWND tmpCtrl;

    tmpCtrl = GetDlgItem(hwndBoard, EDT_LEVEL);
    std::string val = std::to_string(level);
    SetWindowTextA(tmpCtrl, (LPCSTR)val.c_str());
}

void SetTimeControls()
{
    HWND tmp;
    std::string time;

    time = TimeToString(timeCurrent, 3);

// CURRENT LABEL
    tmp = GetDlgItem(hwndBoard, EDT_CUR);
    SetWindowTextA(tmp, (time + "s").c_str());

// FASTEST LABEL
    if ((level == 1) || (timeCurrent < timeFastest))
    {
        tmp = GetDlgItem(hwndBoard, EDT_FST);
        SetWindowTextA(tmp, (time + "s").c_str());
        timeFastest = timeCurrent;
    }

// AVERAGE LABEL
    timeTotal = timeTotal + timeCurrent;
    time = TimeToString(timeTotal / level, 3);
    
    tmp = GetDlgItem(hwndBoard, EDT_AVG);
    SetWindowTextA(tmp, (time + "s").c_str());

};

std::string TimeToString(std::chrono::duration<float> dTime, int decPlaces)
{
    std::string s = std::to_string(dTime.count());
    std::string sRound = s.substr(0, s.find(".") + decPlaces + 1);

    return sRound;
};

void LoadBoardImages()
{
    hbmCircleGrey   = (HBITMAP)LoadImageW(NULL, L"Images\\circleGrey.bmp",   IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleGreen  = (HBITMAP)LoadImageW(NULL, L"Images\\circleGreen.bmp",  IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleOrange = (HBITMAP)LoadImageW(NULL, L"Images\\circleOrange.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleRed    = (HBITMAP)LoadImageW(NULL, L"Images\\circleRed.bmp",    IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleGreenL = (HBITMAP)LoadImageW(NULL, L"Images\\circleGreenLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleOrangeL = (HBITMAP)LoadImageW(NULL, L"Images\\circleOrangeLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleRedL = (HBITMAP)LoadImageW(NULL, L"Images\\circleRedLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmBackArrow = (HBITMAP)LoadImageW(NULL, L"Images\\backArrow.bmp", IMAGE_BITMAP, 80, 35, LR_LOADFROMFILE);
    //hbmRadioGreen = (HBITMAP)LoadImageW(NULL, L"Images\\radioGreen.bmp", IMAGE_BITMAP, 14, 14, LR_LOADFROMFILE); 
}

void LoadMainImages()
{
    hbmBelt = (HBITMAP)LoadImageW(NULL, L"Images\\Belt.bmp", IMAGE_BITMAP, 40, 20, LR_LOADFROMFILE);
}

void SetScores()
{
    HWND tmp;

//=============== CHAMPION ===========================================================
    Scores s(0);

    tmp = GetDlgItem(hwndMain, EDT_HS0NAME);
    SetWindowTextA(tmp, (LPCSTR)s.name.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS0LVL);
    SetWindowTextA(tmp, (LPCSTR)s.level.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS0TOTAL);
    SetWindowTextA(tmp, (LPCSTR)s.total.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS0FAST);
    SetWindowTextA(tmp, (LPCSTR)s.fast.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS0AVG);
    SetWindowTextA(tmp, (LPCSTR)s.average.c_str());

    s.Scores::~Scores();
//=============== 1 ===========================================================
    s = Scores(1);

    tmp = GetDlgItem(hwndMain, EDT_HS1NAME);
    SetWindowTextA(tmp, (LPCSTR)s.name.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS1LVL);
    SetWindowTextA(tmp, (LPCSTR)s.level.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS1TOTAL);
    SetWindowTextA(tmp, (LPCSTR)s.total.c_str());
    
    tmp = GetDlgItem(hwndMain, EDT_HS1FAST);
    SetWindowTextA(tmp, (LPCSTR)s.fast.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS1AVG);
    SetWindowTextA(tmp, (LPCSTR)s.average.c_str());

    s.Scores::~Scores();
//=============== 2 ===========================================================
    s = Scores(2);

    tmp = GetDlgItem(hwndMain, EDT_HS2NAME);
    SetWindowTextA(tmp, (LPCSTR)s.name.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS2LVL);
    SetWindowTextA(tmp, (LPCSTR)s.level.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS2TOTAL);
    SetWindowTextA(tmp, (LPCSTR)s.total.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS2FAST);
    SetWindowTextA(tmp, (LPCSTR)s.fast.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS2AVG);
    SetWindowTextA(tmp, (LPCSTR)s.average.c_str());

    s.Scores::~Scores();
//=============== 3 ===========================================================
    s = Scores(3);

    tmp = GetDlgItem(hwndMain, EDT_HS3NAME);
    SetWindowTextA(tmp, (LPCSTR)s.name.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS3LVL);
    SetWindowTextA(tmp, (LPCSTR)s.level.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS3TOTAL);
    SetWindowTextA(tmp, (LPCSTR)s.total.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS3FAST);
    SetWindowTextA(tmp, (LPCSTR)s.fast.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS3AVG);
    SetWindowTextA(tmp, (LPCSTR)s.average.c_str());

    s.Scores::~Scores();
//=============== 4 ===========================================================
    s = Scores(4);

    tmp = GetDlgItem(hwndMain, EDT_HS4NAME);
    SetWindowTextA(tmp, (LPCSTR)s.name.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS4LVL);
    SetWindowTextA(tmp, (LPCSTR)s.level.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS4TOTAL);
    SetWindowTextA(tmp, (LPCSTR)s.total.c_str());
    
    tmp = GetDlgItem(hwndMain, EDT_HS4FAST);
    SetWindowTextA(tmp, (LPCSTR)s.fast.c_str());

    tmp = GetDlgItem(hwndMain, EDT_HS4AVG);
    SetWindowTextA(tmp, (LPCSTR)s.average.c_str());

    s.Scores::~Scores();
}

int CheckScores()
{
    std::string tot;
    std::string tmpLevel;
    int posSwap = -1;

    for (int i = 0; i < 5; i++)
    {
        Scores s(i);
        tmpLevel = s.level;
        tot = s.total;
        tot = tot.substr(0, tot.length() - 1);

        if (level > 1)
        {
            if ((tmpLevel == "_ _") || (level - 1 > std::stoi(tmpLevel)))
            {
                posSwap = i;
                break;
            }
            else if (level - 1 == std::stoi(tmpLevel))
            {
                if (timeTotal.count() < std::stof((s.total).substr(0,(s.total).length()-1)))
                {
                    posSwap = i;
                    break;
                }
                else if (timeFastest.count() < std::stof((s.fast).substr(0, (s.fast).length() - 1)))
                {
                    posSwap = i;
                    break;
                }
                else if ((timeTotal / (level - 1)).count() < std::stof((s.average).substr(0, (s.average).length() - 1)))
                {
                    posSwap = i;
                    break;
                }
            }
        }
    }

    return posSwap;
}

void SwapScores()
{
    std::vector<std::string> data;
    std::string line;
    char line1[7];
    int loopStart;
    std::ifstream iFile("Scores\\Scores.txt");

    loopStart = 25 * (boardSize - 2) + 5 * swapPosition;

    for (int i = 0; i < 100; i++)
    {
        iFile.getline(line1, 7);
        line = line1;
        data.push_back(line);
    }

    iFile.close();

    std::ofstream oFile;
    oFile.open("Scores\\Scores.txt", std::ios::out);
    for (int i = 0; i < 100; i++) // from i = loopStart
    {
        if (i == loopStart)
        {
            oFile << GetName() << std::endl;
            oFile << std::to_string(level - 1) << std::endl;
            oFile << TimeToString(timeTotal, 3) << std::endl;
            oFile << TimeToString(timeFastest, 3) << std::endl;
            oFile << TimeToString(timeTotal / (level - 1), 3) << std::endl;

            for (int j = 0; j < 5 * (4 - swapPosition); j++)
            {
                oFile << data[i] << std::endl;
                i++;
            }

            i = i + 4;
        }
        else
        {
            oFile << data[i] << std::endl;
        }
    }
}

std::string GetName()
{
    HWND tmp = GetDlgItem(hwndName, EDT_NAME);
    char textBuffer[6];

    SendMessageA(tmp, WM_GETTEXT, 6, (LPARAM)textBuffer);

    std::string text = textBuffer;

    if (text == "")
    {
        text = "J.DOE";
    }

    std::transform(text.begin(), text.end(), text.begin(), std::toupper);

    return text;
}

void ResetGame()
{
    endPlay = false;
    level = 1;
    timeCurrent = std::chrono::duration<float>(0.000);
    timeFastest = std::chrono::duration<float>(0.000);
    timeTotal = std::chrono::duration<float>(0.000);
    SendMessage(CurrClicked, BM_SETCHECK, BST_UNCHECKED, 1);
    SetLevelControls();
    SetTimeControls();
    ResetCircles();
}

void LoadLightCircles()
{
    if (timerCount >= 2)
    {
        SendMessageW(hCircle1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGreenL);
    }

    if (timerCount >= 3)
    {
        SendMessageW(hCircle2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleOrangeL);
    }

    if (timerCount == 4)
    {
        SendMessageW(hCircle3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleRedL);
    }
}

void EndMatch()
{
    KillTimer(hwndBoard, IDT_TIMER);
    LoadLightCircles();
    endPlay = true;

    swapPosition = CheckScores();

    if (swapPosition < 0)
    {
        MessageBoxA(NULL, "Not this time lame-o", "Looser", MB_OK);
    }
    else
    {
        MessageBoxA(NULL, "Future generations will remember you", "Wow, so fast!", MB_OK);
        OpenNameWindow();
    }
}