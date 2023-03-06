// Include
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

#define UNICODE
#define MAX_LOADSTRING  100

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// =========================================================================== GLOBAL VARIABLES AND FUNCTIONS DECLARATIONS ===============================================================

#pragma region Global Variables
HINSTANCE hInst;                                                            // Current Instance
WCHAR szTitle[MAX_LOADSTRING];                                              // Title Bar Text for Main Window
WCHAR szWindowClass[MAX_LOADSTRING];                                        // Class Name of Main Window
WCHAR szTitleB[MAX_LOADSTRING];                                             // Title Bar Text for Board Window
WCHAR szWindowClassB[MAX_LOADSTRING];                                       // Class Name of Board Window
WCHAR szTitleN[MAX_LOADSTRING];                                             // Title Bar Text for New Score Window
WCHAR szWindowClassN[MAX_LOADSTRING];                                       // Class Name of New Score Window
HWND CurrClicked = NULL;                                                    // Currently clicked Radio Button
HWND hwndMain;                                                              // Handler for Main Window
HWND hwndBoard;                                                             // Handler for Board Window
HWND hwndName;                                                              // Handler for Name Input Window
int boardSize = 2;                                                          // Size of the Buttons Board from Combo Box
int level;                                                                  // Level being increment after each correct click
int windowSize;                                                             // Main Window and Board Window size
HANDLE timerThreadHandle;                                                   // Handle for Timers
unsigned int tlIndicator = 30;                                              // Time Length Indicator. Bigger number, more time to click
bool firstTurn = false;                                                     // Flag for marking if inside first turn
HBITMAP hbmCircleGrey, hbmCircleGreen, hbmCircleOrange, hbmCircleRed, hbmCircleGreenL, hbmCircleOrangeL, hbmCircleRedL;     // Handles for Circles images with Color Lights
HBITMAP hbmBackArrow;                                                       // Handle for Back Arrow image
HBITMAP hbmBelt;                                                            // Handle for Belt image
bool btnClicked = false;                                                    // Flag to mark if Button 'NEXT' was clicked in the given turn
std::chrono::steady_clock::time_point startTime;                            // Timer starts when level starts
std::chrono::steady_clock::time_point endTime;                              // Timer ends when Radio Button is clicked
std::chrono::duration<float> timeCurrent, timeFastest, timeTotal;           // Time values for Current level speed, all levels fastest and total for all levels
int timerCount = 0;                                                         // Flag marking which big circle image part we are in
HWND hCircle1, hCircle2, hCircle3;                                          // Handles for Big Circles objects
HWND hBackArrow;                                                            // Handles for Back Arrow object
int swapPosition;                                                           // Position of the new score at which it has to be placed in the scores table
bool endPlay;                                                               // Flag to mark if play finished
#pragma endregion


#pragma region Functions
ATOM                    MyRegisterClass(HINSTANCE hInstance, WNDPROC wProc, WCHAR &wcName, int nbrWindow, int nbrColor);    // Window class
BOOL                    InitInstance(HINSTANCE, int);                       // Main Window Instance Initialization
LRESULT CALLBACK        WndProc(HWND, UINT, WPARAM, LPARAM);                // Process messages for Main Window
LRESULT CALLBACK        WndProcB(HWND, UINT, WPARAM, LPARAM);               // Process messages for Board Window
LRESULT CALLBACK        WndProcN(HWND, UINT, WPARAM, LPARAM);               // Process messages for New Name Window
INT_PTR CALLBACK        About(HWND, UINT, WPARAM, LPARAM);                  // Handle information about the game
void                    AddMainControls();                                  // Add controls to Main Window
void                    SetGlobalSize();                                    // Set global Board size and Board Window size
void                    AddBoardControls();                                 // Add controls to Board Window
BOOL                    OpenBoardWindow();                                  // Open Board Window
void                    OpenNameWindow();                                   // Open New Name Window
void                    SelectRandomButton();                               // Select random radio button which needs to be clicked
void                    SetLevelControls();                                 // Change level number on Board Window
unsigned int __stdcall  StartTimers(void*);                                 // Start three timers for Color Lights              
void                    ResetCircles();                                     // Reset Color Circles to grey
void                    LoadBoardImages();                                  // Load images on the Board Window
void                    LoadMainImages();                                   // Load images on the Main Window
void                    SetTimeControls();                                  // Add scores to Board Window game/round scores
std::string             TimeToString(std::chrono::duration<float>, int);    // Convert time to string
void                    SetScores();                                        // Set scores in the table on Main Window
int                     CheckScores();                                      // Check if score from the finished game should be in scores table
void                    SwapScores();                                       // Add new score record to scores table
std::string             GetName();                                          // Get name from the user
void                    ResetGame();                                        // Reset game
void                    LoadLightCircles();                                 // Load Light Circles
void                    EndMatch();                                         // End game
#pragma endregion



// ================================================================================ CLASSES, MAIN AND INITIALIZATIONS ====================================================================

// Class for scores records displayed in the table on the main page
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
        // Constructor for the empty record place holder
        name = "_ _ _ _ _";
        level = "_ _";
        total = "0.000s";
        fast = "0.000s";
        average = "0.000s";
    };
    Scores(int pos)
    {
        // Constructor taking record from the file on the given position in the scores table

        std::ifstream hFile("Scores\\Scores.txt");      // Load file
        int iterStop = 25 * (boardSize - 2) + 5 * pos;  // Calculate beginig of the records for the given position in the table
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
        
        // Fastest
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


// Entry point function
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR lpCmdLine,_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    
    // Initiation of global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLICKER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance, WndProc, *szWindowClass, 0, 1);

    LoadStringW(hInstance, IDS_BOARD_TITLE, szTitleB, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_BOARD, szWindowClassB, MAX_LOADSTRING);
    MyRegisterClass(hInstance, WndProcB, *szWindowClassB, 1, 1);

    LoadStringW(hInstance, IDS_NAME_TITLE, szTitleN, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_NAME, szWindowClassN, MAX_LOADSTRING);
    MyRegisterClass(hInstance, WndProcN, *szWindowClassN, 2, 0);

    // Do aplication initiation:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLICKER));

    MSG msg;

    // Main messages loop:
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


// Register Window class function
ATOM MyRegisterClass(HINSTANCE hInstance, WNDPROC wProc, WCHAR &wcName, int nbrWindow, int nbrColor)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = wProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + nbrColor);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLICKER);
    wcex.lpszClassName = &wcName;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONCHKN));

    return RegisterClassExW(&wcex);
}


// Saves the instance handle in global variable and creates and displays main window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Global variable

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



// =================================================================================== WINDOW CALLBACK FUNCTIONS =========================================================================

// Process messgaes for Main Window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND: // Buttons and Controls
        {
            int wmId = LOWORD(wParam);
            
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
                {
                    // Size combobox
                    if (HIWORD(wParam) == CBN_SELENDOK)
                    {
                        SetGlobalSize();
                        SetScores();
                    }

                    return DefWindowProc(hWnd, message, wParam, lParam);
                }
            }
        } break; 

        case WM_GETMINMAXINFO: // Window size
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 500;
            lpMMI->ptMinTrackSize.y = 500;
            lpMMI->ptMaxTrackSize.x = 500;
            lpMMI->ptMaxTrackSize.y = 500;
        } break;

        case WM_CTLCOLORSTATIC: //Window color
        {
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
            return (INT_PTR)hBrush;
        } break;

        case WM_DESTROY: // Exit
        {
            PostQuitMessage(0);
        } break;
        
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return 0;
}


// Process messgaes for Board Window
LRESULT CALLBACK WndProcB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    std::string texta;
    switch (message)
    {
        case WM_COMMAND: // Buttons and controls
        {
            int wmId = LOWORD(wParam);

            // One of the radio buttons
            if ((wmId >= IDC_RBTN) && (wmId < IDC_RBTN + 100))
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
                    }
                }
            }
            else
            {
                switch (wmId)
                {
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
                                timerThreadHandle = (HANDLE)_beginthreadex(0, 0, &StartTimers, 0, 0, 0);
                                startTime = std::chrono::high_resolution_clock::now();

                            }

                            btnClicked = false;
                        }
                    } break;

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
                    } break;

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
                    } break;
                
                    default:
                    {
                        return DefWindowProc(hWnd, message, wParam, lParam);
                    }  
                }
            } break;
            
        } break;

        case WM_TIMER: // Three timers
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
            }
        } break;
        
        case WM_GETMINMAXINFO: // Window size
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = windowSize;
            lpMMI->ptMinTrackSize.y = windowSize;
            lpMMI->ptMaxTrackSize.x = windowSize;
            lpMMI->ptMaxTrackSize.y = windowSize;
        } break;
        
        case WM_CTLCOLORSTATIC: // Buttons and controls color
        {
            if (GetDlgCtrlID((HWND)lParam) == 511)
            {
                HDC hdcStatic = (HDC)wParam;
                
                SetBkColor(hdcStatic, RGB(0, 0, 210));
            }
        } break;
    
        case WM_CLOSE: // Exit
        {
            SetScores();
            timeCurrent = std::chrono::duration<float>(0.000);
            timeFastest = std::chrono::duration<float>(0.000);
            timeTotal = std::chrono::duration<float>(0.000);
            DestroyWindow(hWnd);
        } break;
    
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        } 
    }

    return 0;
}


// Process messgaes for New Name Window
LRESULT CALLBACK WndProcN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_COMMAND: // Buttons and controls
        {
            int wmId = LOWORD(wParam);
        
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
        } break;
        
        case WM_GETMINMAXINFO: // Window size
        {
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMinTrackSize.x = 200;
            lpMMI->ptMinTrackSize.y = 200;
            lpMMI->ptMaxTrackSize.x = 200;
            lpMMI->ptMaxTrackSize.y = 200;
        } break;
        
        case WM_CTLCOLORSTATIC: // Buttons and controls color
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
        } break;
    
        case WM_CLOSE: // Exit
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
        }break;
        
        default:
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
        }   
    }

    return 0;
}


// Process messages for menu items on Main Window
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            } break;
        }
    }

    return (INT_PTR)FALSE;
}



// ================================================================================= PREPARE WINDOWS AND CONTROLS ========================================================================

// Load images on Main Window
void LoadMainImages()
{
    hbmBelt = (HBITMAP)LoadImageW(NULL, L"Images\\Belt.bmp", IMAGE_BITMAP, 40, 20, LR_LOADFROMFILE);
}


// Load images on Board Window
void LoadBoardImages()
{
    hbmCircleGrey = (HBITMAP)LoadImageW(NULL, L"Images\\circleGrey.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleGreen = (HBITMAP)LoadImageW(NULL, L"Images\\circleGreen.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleOrange = (HBITMAP)LoadImageW(NULL, L"Images\\circleOrange.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleRed = (HBITMAP)LoadImageW(NULL, L"Images\\circleRed.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleGreenL = (HBITMAP)LoadImageW(NULL, L"Images\\circleGreenLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleOrangeL = (HBITMAP)LoadImageW(NULL, L"Images\\circleOrangeLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmCircleRedL = (HBITMAP)LoadImageW(NULL, L"Images\\circleRedLight.bmp", IMAGE_BITMAP, 50, 50, LR_LOADFROMFILE);
    hbmBackArrow = (HBITMAP)LoadImageW(NULL, L"Images\\backArrow.bmp", IMAGE_BITMAP, 80, 35, LR_LOADFROMFILE);
}


// Add controls to Main Window
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
#pragma region High Scores
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
#pragma endregion

    SetScores();
}


// Add controls to Board Window
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
        (windowSize / 2) - 35, y, 50, 20, hwndBoard, NULL, nullptr, nullptr);

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
    x = (windowSize - (2 * boardSize - 1) * 15) / 2;

    for (int i = 1; i <= boardSize; i++)
    {
        for (int j = 1; j <= boardSize; j++)
        {
            std::string name = std::to_string(i) + std::to_string(j);

            tmp = CreateWindowW(L"Button", (LPCTSTR)name.c_str(), WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
                x + (j - 1) * 30, y + (i - 1) * 30, 15, 15, hwndBoard, (HMENU)(IDC_RBTN + 10 * i + j), nullptr, nullptr);
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

}


// Set size of Board Window
void SetGlobalSize()
{
    HWND lblSize = GetDlgItem(hwndMain, CBX_SIZE);
    int idxRow = SendMessage(lblSize, CB_GETCURSEL, 0, 0);

    boardSize = idxRow + 2;
    windowSize = 486 + (boardSize - 2) * 20;
}


// Set up Board Window
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
}


// Set up New Name Window
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




// ====================================================================================== RUNTIME CHANGES ==============================================================================

// Highlight random radio button
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


// remove colors from three circles on Board Window
void ResetCircles()
{
    if (firstTurn)
    {
        SendMessageW(hCircle3, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
        SendMessageW(hCircle2, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
        SendMessageW(hCircle1, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmCircleGrey);
    }
    firstTurn = true;
}


// Change level in label on Board Window
void SetLevelControls()
{
    HWND tmpCtrl;

    tmpCtrl = GetDlgItem(hwndBoard, EDT_LEVEL);
    std::string val = std::to_string(level);
    SetWindowTextA(tmpCtrl, (LPCSTR)val.c_str());
}


// Change time records in time labels on Board Window
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


// Set up colors in cirles on Board Window based on the timers proceeding
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


// Start three timers
unsigned int __stdcall StartTimers(void* data)
{
    LRESULT iPos = 0;
    LRESULT tmp = 2;
    std::string texta;
    int msTimer;
    int msTimerCircle;


    msTimer = (((150*boardSize) / level) - level) * tlIndicator + (150 * boardSize);
    msTimerCircle = msTimer / 4;

    SetTimer(hwndBoard, IDT_TIMER, msTimer, NULL);

    SetTimer(hwndBoard, IDT_TIMER1, msTimerCircle, NULL);
    SetTimer(hwndBoard, IDT_TIMER2, 2 * msTimerCircle, NULL);
    SetTimer(hwndBoard, IDT_TIMER3, 3 * msTimerCircle, NULL);
    timerCount = 1;

    return 0;
}


// Set scores in labels on Main Window
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



// ========================================================================================= HELPERS =================================================================================


// Convert time to string
std::string TimeToString(std::chrono::duration<float> dTime, int decPlaces)
{
    std::string s = std::to_string(dTime.count());
    std::string sRound = s.substr(0, s.find(".") + decPlaces + 1);

    return sRound;
};


// Check if current score fits in high scores table and return position. 
int CheckScores()
{
    //first comapre level then total time then fastest time and average at the end

    std::string tot;
    std::string tmpLevel;
    int posSwap = -1;

    for (int i = 0; i < 5; i++) // Loop through scores table
    {
        Scores s(i);
        tmpLevel = s.level;
        tot = s.total;
        tot = tot.substr(0, tot.length() - 1);

        if (level > 1) // Only for started game
        {
            if ((tmpLevel == "_ _") || (level - 1 > std::stoi(tmpLevel))) // Choose if there is empty place in table or level is higher
            {
                posSwap = i;
                break;
            }
            else if (level - 1 == std::stoi(tmpLevel)) // If the same level, check times
            {
                if (timeTotal.count() < std::stof((s.total).substr(0,(s.total).length()-1))) // Choose if total time is smaller, if no then check fastest time
                {
                    posSwap = i;
                    break;
                }
                else if (timeFastest.count() < std::stof((s.fast).substr(0, (s.fast).length() - 1))) // Choose if fastest time is smaller, if no then check average time
                {
                    posSwap = i;
                    break;
                }
                else if ((timeTotal / (level - 1)).count() < std::stof((s.average).substr(0, (s.average).length() - 1))) // Choose if average time is smaller
                {
                    posSwap = i;
                    break;
                }
            }
        }
    }

    return posSwap;
}


// Swap scores in high scores table in txt file by placing new score and pushing next scores out of the file
void SwapScores()
{
    std::vector<std::string> data;
    std::string line;
    char line1[7];
    int loopStart;
    std::ifstream iFile("Scores\\Scores.txt");


    loopStart = 25 * (boardSize - 2) + 5 * swapPosition; // Set position in vector where scores should be placed

    // ============ READ OLD SCORES TABLE ==========================================================================================================
    for (int i = 0; i < 100; i++) // Build vector with scores by pushing lines from scores txt into the scores vector
    {
        iFile.getline(line1, 7);
        line = line1;
        data.push_back(line);
    }

    iFile.close();

    // ============ CREATE NEW SCORES TABLE ==============================================================================================================
    std::ofstream oFile;
    oFile.open("Scores\\Scores.txt", std::ios::out);

    for (int i = 0; i < 100; i++) // Loop through scores vector
    {
        if (i == loopStart) // If at the position where new scores should start, place new scores. If not, then just build scores table in txt from scores vector
        {
            // Insert scores
            oFile << GetName() << std::endl;
            oFile << std::to_string(level - 1) << std::endl;
            oFile << TimeToString(timeTotal, 3) << std::endl;
            oFile << TimeToString(timeFastest, 3) << std::endl;
            oFile << TimeToString(timeTotal / (level - 1), 3) << std::endl;

            for (int j = 0; j < 5 * (4 - swapPosition); j++) // Add old consecutive scores into the file
            {
                oFile << data[i] << std::endl;
                i++;
            }

            i = i + 4;
        }
        else // Build scores table in txt from scores vector
        {
            oFile << data[i] << std::endl;
        }
    }
}


// Get name from New Name Window
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



// ======================================================================================== CLEANERS ===============================================================================

// Reset play controls and variables
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
    CloseHandle(timerThreadHandle);
}


//Finish Match
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