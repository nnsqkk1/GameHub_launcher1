// =============================================================
// GameHub Launcher v1.0 — иконки игр в списке
// =============================================================

#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shldisp.h>
#include <commdlg.h>
#include <urlmon.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define APP_VERSION L"1.0.0"
#define COVER_SIZE  36

using namespace Gdiplus;

// ============================================================
// Палитра
// ============================================================
struct Palette {
    COLORREF sidebar, navInactive, content, panel;
    COLORREF accent, accentP, green, greenP, red, redP;
    COLORREF neutral, neutralP, text, textDim, border;
    COLORREF disabled, disabledText;
};

static const Palette DARK = {
    RGB(23,26,33), RGB(42,46,55), RGB(27,40,56), RGB(19,28,41),
    RGB(66,143,210), RGB(45,100,155),
    RGB(92,155,105), RGB(65,115,75),
    RGB(200,70,70), RGB(160,50,50),
    RGB(54,60,72), RGB(40,45,55),
    RGB(218,220,225), RGB(140,148,160),
    RGB(50,55,65), RGB(70,75,88), RGB(110,115,125)
};

static const Palette LIGHT = {
    RGB(240,242,248), RGB(220,224,234), RGB(248,250,253), RGB(232,236,242),
    RGB(66,143,210), RGB(45,100,155),
    RGB(72,140,85), RGB(50,105,65),
    RGB(210,60,60), RGB(165,45,45),
    RGB(220,222,230), RGB(200,202,215),
    RGB(28,30,40), RGB(110,112,128),
    RGB(220,222,230), RGB(215,217,225), RGB(160,162,172)
};

Palette g_p = DARK;
bool    g_darkTheme = true;
ULONG_PTR g_gdiToken = 0;

// ============================================================
// ID
// ============================================================
#define ID_NAV_CATALOG   1001
#define ID_NAV_LIBRARY   1002
#define ID_NAV_STATS     1003
#define ID_NAV_PROFILE   1004
#define ID_NAV_SETTINGS  1005

#define ID_SOURCE_COMBO  3001
#define ID_SEARCH_EDIT   3002
#define ID_SEARCH_BTN    3003

#define ID_THEME_DARK    3101
#define ID_THEME_LIGHT   3102
#define ID_OPEN_INSTALL  3103
#define ID_CLEAR_LIBRARY 3104

#define ID_PROFILE_NAME      4001
#define ID_PROFILE_SAVE      4002
#define ID_PROFILE_UPLOAD    4003

#define ID_DLG_NAME_EDIT     5001
#define ID_DLG_VER_EDIT      5002
#define ID_DLG_DESC_EDIT     5003
#define ID_DLG_PICK_EXE      5004
#define ID_DLG_PICK_COVER    5005
#define ID_DLG_PUBLISH       5006
#define ID_DLG_CANCEL_BTN    5007
#define ID_DLG_EXE_INFO      5008
#define ID_DLG_COVER_INFO    5009

#define ID_URL_EDIT          6001
#define ID_URL_NAME_EDIT     6002
#define ID_URL_INSTALL       6004
#define ID_URL_CANCEL        6005

#define ID_UPD_VER_EDIT      7001
#define ID_UPD_PICK_FILE     7002
#define ID_UPD_INFO          7003
#define ID_UPD_FILE_NAME     7004
#define ID_UPD_APPLY         7005
#define ID_UPD_CANCEL        7006

#define ID_DET_PLAY          8001
#define ID_DET_UPDATE        8002
#define ID_DET_FOLDER        8003
#define ID_DET_DELETE        8004
#define ID_DET_DESC          8006

#define ID_TIMER_DOWNLOAD 4001

static const int SIDEBAR_W = 230;
static const int LIST_W = 340;
static const int PAD = 18;

// ============================================================
// Данные
// ============================================================
struct GameItem {
    int          id = 0;
    std::wstring name;
    std::wstring version;
    std::wstring description;
    std::wstring installPath;
    std::wstring installedAt;
    std::wstring lastLaunched;
    std::wstring coverPath;
    int          playTimeSec = 0;
    int          sizeMB = 0;
    bool         mine = false;
};

// ============================================================
// Глобалы
// ============================================================
HFONT g_fontLogo = nullptr, g_fontBody = nullptr, g_fontNav = nullptr;
HFONT g_fontHead = nullptr, g_fontSmall = nullptr;
HBRUSH g_brContent = nullptr, g_brSidebar = nullptr, g_brPanel = nullptr;
HWND g_hMainWnd = nullptr;

HIMAGELIST g_hCoverList = nullptr;

HWND g_hNav[5] = {};

HWND g_hSourceCombo = nullptr, g_hSearchEdit = nullptr, g_hSearchBtn = nullptr;
HWND g_hCatalogList = nullptr, g_hCatalogStatus = nullptr, g_hProgressBar = nullptr;

HWND g_hLibraryList = nullptr, g_hLibraryStatus = nullptr;

HWND g_hProfileTitle = nullptr, g_hProfileNameLbl = nullptr;
HWND g_hProfileNameEdit = nullptr, g_hProfileSaveBtn = nullptr;
HWND g_hProfileUploadBtn = nullptr, g_hProfileMyLbl = nullptr;
HWND g_hProfileMyList = nullptr, g_hProfileStatus = nullptr;

HWND g_hStatsTitle = nullptr, g_hStatsBody = nullptr;

HWND g_hSettingsTitle = nullptr, g_hSettingsBody = nullptr;
HWND g_hThemeDarkBtn = nullptr, g_hThemeLightBtn = nullptr;
HWND g_hOpenInstallBtn = nullptr, g_hClearLibBtn = nullptr;
HWND g_hSettingsStatus = nullptr;

HWND g_hDetTitle = nullptr;
HWND g_hDetInfo = nullptr;
HWND g_hDetDesc = nullptr;
HWND g_hDetBtn1 = nullptr;
HWND g_hDetBtn2 = nullptr;
HWND g_hDetBtn3 = nullptr;
HWND g_hDetBtn4 = nullptr;

int  g_currentPage = 0;

std::vector<GameItem> g_catalog;
std::vector<GameItem> g_library;
int g_nextId = 1;
int g_downloadTick = 0;
int g_downloadingCatalogIdx = -1;

std::wstring g_installRoot = L"C:\\Games\\GameHub";
std::wstring g_profileName = L"Пользователь";
std::wstring g_dlgExePath, g_dlgCoverPath;
std::wstring g_updNewFile;
int          g_updatingGameId = -1;

// ============================================================
// Утилиты
// ============================================================
HFONT MakeFont(int px, int weight) {
    return CreateFontW(-px, 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET,
        0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
}

std::wstring NowStr() {
    time_t t = time(nullptr);
    tm lt; localtime_s(&lt, &t);
    wchar_t buf[32];
    wcsftime(buf, 32, L"%d.%m.%Y %H:%M", &lt);
    return buf;
}

std::wstring HumanTime(int sec) {
    int h = sec / 3600, m = (sec % 3600) / 60;
    wchar_t buf[64];
    if (h > 0) swprintf_s(buf, L"%d ч %d мин", h, m);
    else       swprintf_s(buf, L"%d мин", m);
    return buf;
}

std::wstring HumanSize(long long bytes) {
    if (bytes > 1024LL * 1024 * 1024)
        return std::to_wstring(bytes / (1024LL * 1024 * 1024)) + L" ГБ";
    if (bytes > 1024LL * 1024)
        return std::to_wstring(bytes / (1024LL * 1024)) + L" МБ";
    if (bytes > 1024)
        return std::to_wstring(bytes / 1024) + L" КБ";
    return std::to_wstring(bytes) + L" Б";
}

std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), towlower);
    return s;
}

bool DirExists(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return (a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY));
}

bool FileExists(const std::wstring& p) {
    DWORD a = GetFileAttributesW(p.c_str());
    return (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY));
}

void EnsureGameFolder(const std::wstring& path) {
    std::wstring cur;
    for (size_t i = 0; i < path.size(); ++i) {
        cur += path[i];
        if (path[i] == L'\\' || i == path.size() - 1)
            if (cur.size() > 3) CreateDirectoryW(cur.c_str(), nullptr);
    }
}

std::wstring GetFileNameFromPath(const std::wstring& p) {
    size_t pos = p.find_last_of(L"\\/");
    return (pos == std::wstring::npos) ? p : p.substr(pos + 1);
}

std::wstring GetFileExt(const std::wstring& p) {
    size_t pos = p.find_last_of(L'.');
    if (pos == std::wstring::npos) return L"";
    return Lower(p.substr(pos));
}

std::wstring FindCoverFile(const std::wstring& folder) {
    if (folder.empty()) return L"";
    const wchar_t* names[] = { L"\\cover.png", L"\\cover.jpg", L"\\cover.jpeg", L"\\cover.bmp" };
    for (auto n : names) {
        std::wstring p = folder + n;
        if (FileExists(p)) return p;
    }
    return L"";
}

// ============================================================
// Версии
// ============================================================
int ParseVersionPart(const std::wstring& s, int& idx) {
    int val = 0;
    while (idx < (int)s.size() && s[idx] >= L'0' && s[idx] <= L'9') {
        val = val * 10 + (s[idx] - L'0');
        idx++;
    }
    while (idx < (int)s.size() && (s[idx] == L'.' || s[idx] == L' ')) idx++;
    return val;
}

bool IsNewerVersion(const std::wstring& current, const std::wstring& remote) {
    int ci = 0, ri = 0;
    for (int i = 0; i < 3; ++i) {
        int c = ParseVersionPart(current, ci);
        int r = ParseVersionPart(remote, ri);
        if (r > c) return true;
        if (r < c) return false;
    }
    return false;
}

// ============================================================
// Иконки игр в списке
// ============================================================
void InitCoverList() {
    if (g_hCoverList) ImageList_Destroy(g_hCoverList);
    g_hCoverList = ImageList_Create(COVER_SIZE, COVER_SIZE,
        ILC_COLOR32 | ILC_MASK, 16, 16);

    HWND lists[] = { g_hCatalogList, g_hLibraryList, g_hProfileMyList };
    for (HWND l : lists) {
        if (l) ListView_SetImageList(l, g_hCoverList, LVSIL_SMALL);
    }
}

int AddCoverIcon(const std::wstring& folder) {
    if (!g_hCoverList) return -1;

    std::wstring cover = FindCoverFile(folder);
    if (cover.empty()) return -1;

    Image* img = Image::FromFile(cover.c_str());
    if (!img || img->GetLastStatus() != Ok) { delete img; return -1; }

    UINT iw = img->GetWidth();
    UINT ih = img->GetHeight();
    if (iw == 0 || ih == 0) { delete img; return -1; }

    Bitmap bmp(COVER_SIZE, COVER_SIZE, PixelFormat32bppPARGB);
    Graphics g(&bmp);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);

    double sx = (double)COVER_SIZE / iw;
    double sy = (double)COVER_SIZE / ih;
    double s = (sx < sy) ? sx : sy;
    int dw = (int)(iw * s);
    int dh = (int)(ih * s);
    int dx = (COVER_SIZE - dw) / 2;
    int dy = (COVER_SIZE - dh) / 2;
    g.DrawImage(img, dx, dy, dw, dh);

    HBITMAP hbm = nullptr;
    bmp.GetHBITMAP(Color(0, 0, 0, 0), &hbm);

    int idx = -1;
    if (hbm) {
        idx = ImageList_Add(g_hCoverList, hbm, nullptr);
        DeleteObject(hbm);
    }
    delete img;
    return idx;
}

// ============================================================
// ZIP
// ============================================================
bool ExtractZipTo(const std::wstring& zipPath, const std::wstring& destFolder) {
    EnsureGameFolder(destFolder);
    HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    IShellDispatch* pShell = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
        IID_IShellDispatch, (void**)&pShell);
    if (FAILED(hr) || !pShell) {
        if (SUCCEEDED(hrInit)) CoUninitialize();
        return false;
    }

    VARIANT vZip; VariantInit(&vZip);
    vZip.vt = VT_BSTR;
    vZip.bstrVal = SysAllocString(zipPath.c_str());

    Folder* pZip = nullptr;
    hr = pShell->NameSpace(vZip, &pZip);
    VariantClear(&vZip);

    if (FAILED(hr) || !pZip) {
        pShell->Release();
        if (SUCCEEDED(hrInit)) CoUninitialize();
        return false;
    }

    FolderItems* pItems = nullptr;
    pZip->Items(&pItems);

    VARIANT vDest; VariantInit(&vDest);
    vDest.vt = VT_BSTR;
    vDest.bstrVal = SysAllocString(destFolder.c_str());

    Folder* pDest = nullptr;
    hr = pShell->NameSpace(vDest, &pDest);
    VariantClear(&vDest);

    bool ok = false;
    if (SUCCEEDED(hr) && pDest && pItems) {
        VARIANT vItems; VariantInit(&vItems);
        vItems.vt = VT_DISPATCH;
        vItems.pdispVal = pItems;
        VARIANT vOpt; VariantInit(&vOpt);
        hr = pDest->CopyHere(vItems, vOpt);
        if (SUCCEEDED(hr)) ok = true;
        pDest->Release();
    }

    if (pItems) pItems->Release();
    pZip->Release();
    pShell->Release();
    if (SUCCEEDED(hrInit)) CoUninitialize();

    if (ok) {
        for (int i = 0; i < 30; ++i) {
            Sleep(200);
            WIN32_FIND_DATAW fd;
            HANDLE hFind = FindFirstFileW((destFolder + L"\\*").c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                int count = 0;
                do { if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L"..")) count++; } while (FindNextFileW(hFind, &fd));
                FindClose(hFind);
                if (count > 0) break;
            }
        }
    }
    return ok;
}

// ============================================================
// Поиск exe
// ============================================================
bool FindExeRecursive(const std::wstring& folder, std::wstring& out, int depth = 0) {
    if (depth > 4) return false;
    const wchar_t* c[] = { L"\\game.exe", L"\\start.exe", L"\\run.exe",
                          L"\\launcher.exe", L"\\main.exe" };
    for (auto x : c) {
        std::wstring p = folder + x;
        if (FileExists(p)) { out = p; return true; }
    }
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((folder + L"\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            std::wstring lo = Lower(fd.cFileName);
            if (lo.size() < 5) continue;
            if (lo.substr(lo.size() - 4) != L".exe") continue;
            if (lo.find(L"unins") != std::wstring::npos) continue;
            if (lo.find(L"vcredist") != std::wstring::npos) continue;
            if (lo.find(L"setup") != std::wstring::npos) continue;
            out = folder + L"\\" + fd.cFileName;
            FindClose(h);
            return true;
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    h = FindFirstFileW((folder + L"\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            std::wstring sub = folder + L"\\" + fd.cFileName;
            if (FindExeRecursive(sub, out, depth + 1)) { FindClose(h); return true; }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    return false;
}

// ============================================================
// LocalStorage
// ============================================================
std::wstring GetDataFolder() {
    wchar_t* a = _wgetenv(L"APPDATA");
    std::wstring base = a ? a : L"C:\\";
    std::wstring p = base + L"\\GameHubLauncher";
    CreateDirectoryW(p.c_str(), nullptr);
    return p;
}
std::wstring GetLibraryFile() { return GetDataFolder() + L"\\library.dat"; }
std::wstring GetProfileFile() { return GetDataFolder() + L"\\profile.dat"; }
std::wstring GetCatalogFile() { return GetDataFolder() + L"\\catalog.dat"; }

void WriteWideFile(const std::wstring& path, const std::wstring& content) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    WORD bom = 0xFEFF; DWORD w;
    WriteFile(h, &bom, 2, &w, nullptr);
    WriteFile(h, content.c_str(), (DWORD)(content.size() * sizeof(wchar_t)), &w, nullptr);
    CloseHandle(h);
}

std::wstring ReadWideFile(const std::wstring& path) {
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return L"";
    DWORD sz = GetFileSize(h, nullptr);
    if (sz < 4) { CloseHandle(h); return L""; }
    std::vector<wchar_t> buf(sz / 2);
    DWORD r = 0;
    if (!ReadFile(h, buf.data(), sz, &r, nullptr)) { CloseHandle(h); return L""; }
    CloseHandle(h);
    size_t start = (buf[0] == 0xFEFF) ? 1 : 0;
    return std::wstring(buf.begin() + start, buf.begin() + (r / 2));
}

void SaveLibrary() {
    std::wstring c;
    for (auto& g : g_library) {
        c += std::to_wstring(g.id);
        c += L"\t" + g.name;
        c += L"\t" + g.version;
        c += L"\t" + g.description;
        c += L"\t" + g.installPath;
        c += L"\t" + g.installedAt;
        c += L"\t" + g.lastLaunched;
        c += L"\t" + std::to_wstring(g.playTimeSec);
        c += L"\t" + std::to_wstring(g.sizeMB);
        c += L"\t" + g.coverPath;
        c += L"\t";
        c += (g.mine ? L"1" : L"0");
        c += L"\n";
    }
    WriteWideFile(GetLibraryFile(), c);
}

void LoadLibrary() {
    std::wstring content = ReadWideFile(GetLibraryFile());
    if (content.empty()) return;
    g_library.clear();
    int maxId = 0;
    std::wstringstream ss(content);
    std::wstring line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::wstringstream ls(line);
        GameItem g; std::wstring t;
        std::getline(ls, t, L'\t'); g.id = _wtoi(t.c_str());
        std::getline(ls, g.name, L'\t');
        std::getline(ls, g.version, L'\t');
        std::getline(ls, g.description, L'\t');
        std::getline(ls, g.installPath, L'\t');
        std::getline(ls, g.installedAt, L'\t');
        std::getline(ls, g.lastLaunched, L'\t');
        std::getline(ls, t, L'\t'); g.playTimeSec = _wtoi(t.c_str());
        std::getline(ls, t, L'\t'); g.sizeMB = _wtoi(t.c_str());
        std::getline(ls, g.coverPath, L'\t');
        std::getline(ls, t, L'\t'); g.mine = (t == L"1");
        if (!g.name.empty()) {
            g_library.push_back(g);
            if (g.id > maxId) maxId = g.id;
        }
    }
    g_nextId = maxId + 1;
}

void SaveCatalog() {
    std::wstring c;
    for (auto& g : g_catalog) {
        c += g.name + L"\t" + g.version + L"\t" + g.description + L"\t";
        c += std::to_wstring(g.sizeMB) + L"\n";
    }
    WriteWideFile(GetCatalogFile(), c);
}

void LoadCatalog() {
    std::wstring content = ReadWideFile(GetCatalogFile());
    if (content.empty()) return;
    g_catalog.clear();
    std::wstringstream ss(content);
    std::wstring line;
    while (std::getline(ss, line)) {
        if (line.empty()) continue;
        std::wstringstream ls(line);
        GameItem g; std::wstring t;
        std::getline(ls, g.name, L'\t');
        std::getline(ls, g.version, L'\t');
        std::getline(ls, g.description, L'\t');
        std::getline(ls, t, L'\t'); g.sizeMB = _wtoi(t.c_str());
        if (!g.name.empty()) g_catalog.push_back(g);
    }
}

void SaveProfile() { WriteWideFile(GetProfileFile(), g_profileName); }
void LoadProfile() {
    std::wstring c = ReadWideFile(GetProfileFile());
    if (!c.empty()) g_profileName = c;
}

// ============================================================
// Заглушка
// ============================================================
void CreateGameStub(const GameItem& g) {
    EnsureGameFolder(g.installPath);
    std::wstring p = g.installPath + L"\\game.bat";
    std::wstring c;
    c += L"@echo off\r\nchcp 65001 >nul\r\n";
    c += L"title " + g.name + L" " + g.version + L"\r\n";
    c += L"color 0A\r\ncls\r\n";
    c += L"echo.\r\n";
    c += L"echo ============================================\r\n";
    c += L"echo    " + g.name + L"  " + g.version + L"\r\n";
    c += L"echo ============================================\r\n";
    c += L"echo.\r\n" + g.description + L"\r\n";
    c += L"echo.\r\npause\r\n";
    int n = WideCharToMultiByte(CP_UTF8, 0, c.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string u(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, c.c_str(), -1, u.data(), n, nullptr, nullptr);
    HANDLE h = CreateFileW(p.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD w; BYTE bom[3] = { 0xEF, 0xBB, 0xBF };
    WriteFile(h, bom, 3, &w, nullptr);
    WriteFile(h, u.data(), (DWORD)u.size() - 1, &w, nullptr);
    CloseHandle(h);
}

// ============================================================
// Тема
// ============================================================
void ApplyTheme(bool dark) {
    g_darkTheme = dark;
    g_p = dark ? DARK : LIGHT;
    if (g_brContent) DeleteObject(g_brContent);
    if (g_brSidebar) DeleteObject(g_brSidebar);
    if (g_brPanel) DeleteObject(g_brPanel);
    g_brContent = CreateSolidBrush(g_p.content);
    g_brSidebar = CreateSolidBrush(g_p.sidebar);
    g_brPanel = CreateSolidBrush(g_p.panel);

    HWND lsts[] = { g_hCatalogList, g_hLibraryList, g_hProfileMyList };
    for (HWND l : lsts) {
        if (!l) continue;
        ListView_SetBkColor(l, g_p.panel);
        ListView_SetTextBkColor(l, g_p.panel);
        ListView_SetTextColor(l, g_p.text);
    }
    if (g_hProgressBar) {
        SendMessageW(g_hProgressBar, PBM_SETBARCOLOR, 0, (LPARAM)g_p.accent);
        SendMessageW(g_hProgressBar, PBM_SETBKCOLOR, 0, (LPARAM)g_p.neutral);
    }
    if (g_hMainWnd)
        RedrawWindow(g_hMainWnd, nullptr, nullptr,
            RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE);
}

// ============================================================
// Списки
// ============================================================
void AddGameRow(HWND hList, const GameItem& g) {
    int coverIdx = -1;
    if (!g.installPath.empty())
        coverIdx = AddCoverIcon(g.installPath);

    LVITEMW lvi = {};
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem = ListView_GetItemCount(hList);
    lvi.pszText = const_cast<LPWSTR>(g.name.c_str());
    lvi.iImage = coverIdx;
    int idx = ListView_InsertItem(hList, &lvi);
    ListView_SetItemText(hList, idx, 1, const_cast<LPWSTR>(g.version.c_str()));
}

void StyleList(HWND hList) {
    SendMessageW(hList, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    ListView_SetBkColor(hList, g_p.panel);
    ListView_SetTextBkColor(hList, g_p.panel);
    ListView_SetTextColor(hList, g_p.text);
    LVCOLUMNW lvc = {};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.pszText = const_cast<LPWSTR>(L"Игра"); lvc.cx = 230;
    ListView_InsertColumn(hList, 0, &lvc);
    lvc.pszText = const_cast<LPWSTR>(L"Версия"); lvc.cx = 90;
    ListView_InsertColumn(hList, 1, &lvc);
}

HWND MakeButton(HWND p, int id, const wchar_t* t, HFONT f = nullptr) {
    HWND h = CreateWindowExW(0, L"BUTTON", t,
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 0, 0, p, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)(f ? f : g_fontBody), TRUE);
    return h;
}

HWND MakeLabel(HWND p, const wchar_t* t, HFONT f) {
    HWND h = CreateWindowExW(0, L"STATIC", t, WS_CHILD | SS_LEFT,
        0, 0, 0, 0, p, nullptr, GetModuleHandle(nullptr), nullptr);
    SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE);
    return h;
}

void RefreshCatalog(const std::wstring& filter = L"") {
    InitCoverList();
    ListView_DeleteAllItems(g_hCatalogList);
    std::wstring f = Lower(filter);
    for (auto& g : g_catalog) {
        if (!f.empty() && Lower(g.name).find(f) == std::wstring::npos) continue;
        AddGameRow(g_hCatalogList, g);
    }
    if (g_catalog.empty())
        SetWindowTextW(g_hCatalogStatus, L"Каталог пуст");
    else {
        wchar_t b[128];
        swprintf_s(b, L"Игр в каталоге: %d", (int)g_catalog.size());
        SetWindowTextW(g_hCatalogStatus, b);
    }
}

void RefreshLibrary() {
    InitCoverList();
    ListView_DeleteAllItems(g_hLibraryList);
    for (auto& g : g_library) AddGameRow(g_hLibraryList, g);
    wchar_t b[128];
    swprintf_s(b, L"Установлено игр: %d", (int)g_library.size());
    SetWindowTextW(g_hLibraryStatus, b);
}

void RefreshMyGames() {
    if (!g_hProfileMyList) return;
    InitCoverList();
    ListView_DeleteAllItems(g_hProfileMyList);
    int cnt = 0;
    for (auto& g : g_library)
        if (g.mine) { AddGameRow(g_hProfileMyList, g); cnt++; }
    wchar_t b[128];
    swprintf_s(b, L"Мои игры: %d", cnt);
    if (g_hProfileMyLbl) SetWindowTextW(g_hProfileMyLbl, b);
}

void RefreshStats() {
    int s = 0, mb = 0;
    for (auto& g : g_library) { s += g.playTimeSec; mb += g.sizeMB; }
    std::wstring most = L"—"; int best = 0;
    for (auto& g : g_library) if (g.playTimeSec > best) { best = g.playTimeSec; most = g.name; }
    std::wstring last = L"—";
    for (auto& g : g_library) if (!g.lastLaunched.empty()) last = g.lastLaunched;
    std::wstring body;
    body += L"Всего игр установлено:      " + std::to_wstring(g_library.size()) + L"\r\n";
    body += L"Общее время в играх:          " + HumanTime(s) + L"\r\n";
    body += L"Самая играемая:                 " + most + L"\r\n";
    body += L"Последний запуск:              " + last + L"\r\n";
    body += L"Занято на диске:               " + std::to_wstring(mb) + L" МБ";
    SetWindowTextW(g_hStatsBody, body.c_str());
}

// ============================================================
// Детальная панель
// ============================================================
void ClearDetails() {
    if (!g_hDetTitle) return;
    SetWindowTextW(g_hDetTitle, L"");
    SetWindowTextW(g_hDetInfo, L"");
    SetWindowTextW(g_hDetDesc, L"");
    ShowWindow(g_hDetBtn1, SW_HIDE);
    ShowWindow(g_hDetBtn2, SW_HIDE);
    ShowWindow(g_hDetBtn3, SW_HIDE);
    ShowWindow(g_hDetBtn4, SW_HIDE);
}

void SetDetButton(HWND btn, const wchar_t* text, bool show) {
    if (!btn) return;
    if (text) SetWindowTextW(btn, text);
    ShowWindow(btn, show ? SW_SHOW : SW_HIDE);
}

void ShowGameDetails(const GameItem* g, int source) {
    if (!g_hDetTitle) return;
    if (!g) { ClearDetails(); return; }

    SetWindowTextW(g_hDetTitle, g->name.c_str());

    std::wstring info;
    info += L"Версия: " + g->version;
    if (!g->installedAt.empty()) info += L"    •    Установлено: " + g->installedAt;
    if (g->playTimeSec > 0) info += L"    •    В игре: " + HumanTime(g->playTimeSec);
    if (source != 0) info += L"    •    Размер: " + std::to_wstring(g->sizeMB) + L" МБ";
    SetWindowTextW(g_hDetInfo, info.c_str());

    std::wstring desc = g->description.empty() ? L"Описание не указано." : g->description;
    SetWindowTextW(g_hDetDesc, desc.c_str());

    if (source == 0) {
        bool installed = false;
        for (auto& x : g_library) if (x.name == g->name) { installed = true; break; }
        SetDetButton(g_hDetBtn1, installed ? L"Уже установлена" : L"Установить", true);
        EnableWindow(g_hDetBtn1, installed ? FALSE : TRUE);
        SetDetButton(g_hDetBtn2, nullptr, false);
        SetDetButton(g_hDetBtn3, nullptr, false);
        SetDetButton(g_hDetBtn4, nullptr, false);
    }
    else {
        SetDetButton(g_hDetBtn1, L"Играть", true);
        EnableWindow(g_hDetBtn1, TRUE);
        SetDetButton(g_hDetBtn2, L"Обновить", true);
        SetDetButton(g_hDetBtn3, L"Открыть папку", true);
        SetDetButton(g_hDetBtn4, L"Удалить", true);
    }

    if (g_hMainWnd) InvalidateRect(g_hMainWnd, nullptr, TRUE);
}

// ============================================================
// Показ страниц
// ============================================================
void ShowPage(int page) {
    g_currentPage = page;
    auto v = [](HWND h, bool s) { if (h) ShowWindow(h, s ? SW_SHOW : SW_HIDE); };

    bool p0 = (page == 0);
    v(g_hSourceCombo, p0); v(g_hSearchEdit, p0); v(g_hSearchBtn, p0);
    v(g_hCatalogList, p0); v(g_hCatalogStatus, p0); v(g_hProgressBar, p0);

    bool p1 = (page == 1);
    v(g_hLibraryList, p1); v(g_hLibraryStatus, p1);

    bool p2 = (page == 2);
    v(g_hStatsTitle, p2); v(g_hStatsBody, p2);
    if (p2) RefreshStats();

    bool p3 = (page == 3);
    v(g_hProfileTitle, p3); v(g_hProfileNameLbl, p3); v(g_hProfileNameEdit, p3);
    v(g_hProfileSaveBtn, p3); v(g_hProfileUploadBtn, p3); v(g_hProfileMyLbl, p3);
    v(g_hProfileMyList, p3); v(g_hProfileStatus, p3);
    if (p3) RefreshMyGames();

    bool p4 = (page == 4);
    v(g_hSettingsTitle, p4); v(g_hSettingsBody, p4);
    v(g_hThemeDarkBtn, p4); v(g_hThemeLightBtn, p4);
    v(g_hOpenInstallBtn, p4); v(g_hClearLibBtn, p4); v(g_hSettingsStatus, p4);

    bool showDet = (p0 || p1 || p3);
    v(g_hDetTitle, showDet);
    v(g_hDetInfo, showDet);
    v(g_hDetDesc, showDet);
    if (!showDet) {
        v(g_hDetBtn1, false); v(g_hDetBtn2, false);
        v(g_hDetBtn3, false); v(g_hDetBtn4, false);
    }
    else {
        ClearDetails();
    }

    for (int i = 0; i < 5; ++i)
        if (g_hNav[i]) { ShowWindow(g_hNav[i], SW_SHOW); InvalidateRect(g_hNav[i], nullptr, TRUE); }
}

// ============================================================
// Проводник
// ============================================================
void OpenInExplorer(const std::wstring& path) {
    if (!DirExists(path)) {
        std::wstring q = L"Папка не создана:\r\n" + path + L"\r\n\r\nСоздать?";
        if (MessageBoxW(g_hMainWnd, q.c_str(), L"Папка", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
        EnsureGameFolder(path);
        if (!DirExists(path)) return;
    }
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = L"open"; sei.lpFile = path.c_str(); sei.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&sei);
}

void RemoveFolderRecursive(const std::wstring& path) {
    if (!DirExists(path)) return;
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((path + L"\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            std::wstring full = path + L"\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveFolderRecursive(full);
            else { SetFileAttributesW(full.c_str(), FILE_ATTRIBUTE_NORMAL); DeleteFileW(full.c_str()); }
        } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    RemoveDirectoryW(path.c_str());
}

// ============================================================
// Действия
// ============================================================
int SelectedCatalog() { return ListView_GetNextItem(g_hCatalogList, -1, LVNI_SELECTED); }
int SelectedLibrary() { return ListView_GetNextItem(g_hLibraryList, -1, LVNI_SELECTED); }
int SelectedMy() { return ListView_GetNextItem(g_hProfileMyList, -1, LVNI_SELECTED); }

GameItem* CatalogByIndex(int i) {
    if (i < 0) return nullptr;
    wchar_t b[256] = {};
    ListView_GetItemText(g_hCatalogList, i, 0, b, 256);
    for (auto& g : g_catalog) if (g.name == b) return &g;
    return nullptr;
}

GameItem* LibraryByIndex(int i) {
    if (i < 0) return nullptr;
    wchar_t b[256] = {};
    ListView_GetItemText(g_hLibraryList, i, 0, b, 256);
    for (auto& g : g_library) if (g.name == b) return &g;
    return nullptr;
}

GameItem* MyByIndex(int i) {
    if (i < 0) return nullptr;
    wchar_t b[256] = {};
    ListView_GetItemText(g_hProfileMyList, i, 0, b, 256);
    for (auto& g : g_library) if (g.name == b && g.mine) return &g;
    return nullptr;
}

void StartDownload(HWND h) {
    int i = SelectedCatalog();
    if (i < 0) return;
    GameItem* s = CatalogByIndex(i);
    if (!s) return;
    for (auto& g : g_library)
        if (g.name == s->name) { MessageBoxW(h, L"Уже в библиотеке.", L"", MB_OK); return; }
    g_downloadingCatalogIdx = i;
    g_downloadTick = 0;
    ShowWindow(g_hProgressBar, SW_SHOW);
    SendMessageW(g_hProgressBar, PBM_SETPOS, 0, 0);
    SetWindowTextW(g_hCatalogStatus, L"Скачивание...");
    SetTimer(h, ID_TIMER_DOWNLOAD, 60, nullptr);
}

void FinishDownload(HWND h) {
    GameItem* s = CatalogByIndex(g_downloadingCatalogIdx);
    if (s) {
        GameItem c = *s;
        c.id = g_nextId++;
        c.installPath = g_installRoot + L"\\" + c.name;
        c.installedAt = NowStr();
        c.mine = false;
        CreateGameStub(c);
        g_library.push_back(c);
        SaveLibrary();
        RefreshLibrary();
    }
    SendMessageW(g_hProgressBar, PBM_SETPOS, 100, 0);
    SetWindowTextW(g_hCatalogStatus, L"Установлено");
    KillTimer(h, ID_TIMER_DOWNLOAD);
    g_downloadingCatalogIdx = -1;
}

void RunGame(HWND h, GameItem* g) {
    if (!g) return;
    if (!DirExists(g->installPath)) {
        if (MessageBoxW(h, L"Папка не найдена. Создать заново?", L"", MB_YESNO) != IDYES) return;
        CreateGameStub(*g);
    }
    std::wstring exe;
    if (!FindExeRecursive(g->installPath, exe)) {
        if (MessageBoxW(h, L"Файл запуска не найден. Создать заново?", L"", MB_YESNO) != IDYES) return;
        CreateGameStub(*g);
        if (!FindExeRecursive(g->installPath, exe)) { MessageBoxW(h, L"Не удалось", L"", MB_OK); return; }
    }
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = L"open"; sei.lpFile = exe.c_str();
    sei.lpDirectory = g->installPath.c_str(); sei.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExW(&sei)) {
        g->lastLaunched = NowStr();
        g->playTimeSec += 300;
        SaveLibrary();
        RefreshStats();
    }
}

void DeleteLibraryItem(HWND h, int idx, bool mine) {
    GameItem* g = mine ? MyByIndex(idx) : LibraryByIndex(idx);
    if (!g) return;
    std::wstring name = g->name, path = g->installPath;
    std::wstring q = L"Удалить «" + name + L"»?\r\n\r\nДа — с файлами. Нет — только запись.";
    int r = MessageBoxW(h, q.c_str(), L"Удаление", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (r == IDCANCEL) return;
    if (r == IDYES) RemoveFolderRecursive(path);
    g_library.erase(std::remove_if(g_library.begin(), g_library.end(),
        [&](const GameItem& x) { return x.name == name; }), g_library.end());
    SaveLibrary(); RefreshLibrary(); RefreshStats(); RefreshMyGames();
    ClearDetails();
}

void ClearLibrary(HWND h) {
    if (g_library.empty()) return;
    if (MessageBoxW(h, L"Очистить всю библиотеку?", L"", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    g_library.clear();
    SaveLibrary(); RefreshLibrary(); RefreshStats(); RefreshMyGames();
    ClearDetails();
}

void UpdateGameFromCatalog(HWND h, GameItem* g) {
    if (!g) return;
    GameItem* cat = nullptr;
    for (auto& c : g_catalog) if (c.name == g->name) { cat = &c; break; }
    if (!cat) { MessageBoxW(h, L"Игра не найдена в каталоге.", L"", MB_OK); return; }
    bool newer = IsNewerVersion(g->version, cat->version);
    std::wstring msg = newer
        ? (L"Доступно обновление!\r\n\r\n" + g->version + L" → " + cat->version + L"\r\n\r\nОбновить?")
        : (L"Установлена последняя версия: " + g->version + L"\r\n\r\nПереустановить?");
    if (MessageBoxW(h, msg.c_str(), L"Обновление", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    RemoveFolderRecursive(g->installPath);
    g->version = cat->version;
    g->description = cat->description;
    g->sizeMB = cat->sizeMB;
    g->installedAt = NowStr();
    CreateGameStub(*g);
    SaveLibrary(); RefreshLibrary(); RefreshStats(); RefreshMyGames();
    MessageBoxW(h, L"Игра обновлена.", L"", MB_OK);
}

// ============================================================
// Диалоги
// ============================================================
LRESULT CALLBACK UpdGameDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp);
static HWND updVerEdit = nullptr, updFileName = nullptr, updInfoBox = nullptr;

void ShowUpdateGameDialog(HWND parent, GameItem* g) {
    if (!g) return;
    g_updatingGameId = g->id;
    g_updNewFile.clear();

    const wchar_t* CLS = L"GameHubUpdGameDlg";
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = UpdGameDlgProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(g_p.content);
        RegisterClassW(&wc);
        reg = true;
    }
    EnableWindow(parent, FALSE);
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, CLS,
        L"Обновить игру",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 500,
        parent, nullptr, GetModuleHandle(nullptr), nullptr);
    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
}

LRESULT CALLBACK UpdGameDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE: {
        GameItem* g = nullptr;
        for (auto& x : g_library) if (x.id == g_updatingGameId) { g = &x; break; }
        std::wstring title = g ? g->name : L"Игра";

        HWND lblT = CreateWindowExW(0, L"STATIC",
            (L"Обновление: " + title).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 20, 580, 30, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(lblT, WM_SETFONT, (WPARAM)g_fontHead, TRUE);

        HWND lblV = CreateWindowExW(0, L"STATIC", L"Новая версия:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 70, 150, 24, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(lblV, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

        updVerEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT",
            g ? g->version.c_str() : L"v1.0.0",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            180, 70, 420, 30, h, (HMENU)ID_UPD_VER_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(updVerEdit, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

        HWND lblF = CreateWindowExW(0, L"STATIC", L"Новый файл:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 115, 150, 24, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(lblF, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

        HWND pb = MakeButton(h, ID_UPD_PICK_FILE, L"Выбрать файл");
        SetWindowPos(pb, nullptr, 180, 115, 180, 32, SWP_NOZORDER);

        updFileName = CreateWindowExW(0, L"STATIC", L"Файл не выбран",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            370, 120, 230, 24, h, (HMENU)ID_UPD_FILE_NAME, GetModuleHandle(nullptr), nullptr);
        SendMessageW(updFileName, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);

        HWND lblI = CreateWindowExW(0, L"STATIC", L"Информация:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 160, 200, 24, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(lblI, WM_SETFONT, (WPARAM)g_fontBody, TRUE);

        updInfoBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            20, 190, 590, 200, h, (HMENU)ID_UPD_INFO, GetModuleHandle(nullptr), nullptr);
        SendMessageW(updInfoBox, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);

        HWND ok = MakeButton(h, ID_UPD_APPLY, L"Обновить");
        SetWindowPos(ok, nullptr, 20, 410, 220, 44, SWP_NOZORDER);
        HWND cn = MakeButton(h, ID_UPD_CANCEL, L"Отмена");
        SetWindowPos(cn, nullptr, 260, 410, 150, 44, SWP_NOZORDER);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_UPD_PICK_FILE) {
            wchar_t buf[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = h;
            ofn.lpstrFilter = L"Игра или архив\0*.exe;*.zip;*.msi\0Все\0*.*\0\0";
            ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Выберите новый файл";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn)) {
                g_updNewFile = buf;
                SetWindowTextW(updFileName, GetFileNameFromPath(g_updNewFile).c_str());
                WIN32_FILE_ATTRIBUTE_DATA fad;
                if (GetFileAttributesExW(g_updNewFile.c_str(), GetFileExInfoStandard, &fad)) {
                    long long sz = ((long long)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
                    std::wstring info = L"Размер: " + HumanSize(sz);
                    SetWindowTextW(updInfoBox, info.c_str());
                }
            }
        }
        else if (id == ID_UPD_APPLY) {
            if (g_updNewFile.empty()) { MessageBoxW(h, L"Выберите файл.", L"", MB_OK); break; }
            wchar_t ver[128] = {};
            GetWindowTextW(updVerEdit, ver, 128);
            std::wstring newVer = ver;
            if (newVer.empty()) newVer = L"v1.0.0";

            GameItem* g = nullptr;
            for (auto& x : g_library) if (x.id == g_updatingGameId) { g = &x; break; }
            if (!g) { DestroyWindow(h); break; }

            RemoveFolderRecursive(g->installPath);
            std::wstring ext = GetFileExt(g_updNewFile);
            EnsureGameFolder(g->installPath);
            if (ext == L".zip") {
                if (!ExtractZipTo(g_updNewFile, g->installPath)) {
                    MessageBoxW(h, L"Не удалось распаковать.", L"", MB_OK | MB_ICONERROR);
                    break;
                }
            }
            else {
                std::wstring dst = g->installPath + L"\\" + GetFileNameFromPath(g_updNewFile);
                CopyFileW(g_updNewFile.c_str(), dst.c_str(), FALSE);
            }
            g->version = newVer;
            g->installedAt = NowStr();
            SaveLibrary(); RefreshLibrary(); RefreshMyGames(); RefreshStats();
            MessageBoxW(h, L"Игра обновлена.", L"", MB_OK);
            DestroyWindow(h);
        }
        else if (id == ID_UPD_CANCEL) DestroyWindow(h);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_p.text);
        SetBkColor(dc, g_p.content);
        return (LRESULT)g_brContent;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lp;
        if (dis->CtlType == ODT_BUTTON) {
            int id = (int)dis->CtlID;
            COLORREF bg = g_p.accent, fg = RGB(255, 255, 255);
            if (id == ID_UPD_APPLY) bg = RGB(230, 165, 50);
            if (id == ID_UPD_CANCEL) { bg = g_p.neutral; fg = g_p.text; }
            HBRUSH br = CreateSolidBrush(bg);
            FillRect(dis->hDC, &dis->rcItem, br);
            DeleteObject(br);
            wchar_t t[128] = {};
            GetWindowTextW(dis->hwndItem, t, 128);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, fg);
            HFONT of = (HFONT)SelectObject(dis->hDC, g_fontBody);
            DrawTextW(dis->hDC, t, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dis->hDC, of);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE: DestroyWindow(h); break;
    case WM_DESTROY:
        EnableWindow(GetParent(h), TRUE);
        SetForegroundWindow(GetParent(h));
        break;
    }
    return DefWindowProcW(h, m, wp, lp);
}

LRESULT CALLBACK UploadDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp);
static HWND dlgName = nullptr, dlgVer = nullptr, dlgDesc = nullptr;
static HWND dlgExeInfo = nullptr, dlgCoverInfo = nullptr;

void ShowUploadDialog(HWND parent) {
    const wchar_t* CLS = L"GameHubUploadDlg";
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = UploadDlgProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(g_p.content);
        RegisterClassW(&wc);
        reg = true;
    }
    g_dlgExePath.clear();
    g_dlgCoverPath.clear();
    EnableWindow(parent, FALSE);
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, CLS,
        L"Опубликовать игру",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 680, 720,
        parent, nullptr, GetModuleHandle(nullptr), nullptr);
    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
}

LRESULT CALLBACK UploadDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE: {
        int y = 20;
        auto lbl = [&](const wchar_t* t, int yy) {
            HWND l = CreateWindowExW(0, L"STATIC", t,
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, yy, 130, 24, h, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessageW(l, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
            };
        lbl(L"Название:", y);
        dlgName = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            160, y, 480, 28, h, (HMENU)ID_DLG_NAME_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(dlgName, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
        y += 40;
        lbl(L"Версия:", y);
        dlgVer = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"v1.0.0",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            160, y, 480, 28, h, (HMENU)ID_DLG_VER_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(dlgVer, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
        y += 40;
        lbl(L"Описание:", y);
        dlgDesc = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            160, y, 480, 80, h, (HMENU)ID_DLG_DESC_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(dlgDesc, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
        y += 90;
        lbl(L"Файл игры:", y);
        HWND b1 = MakeButton(h, ID_DLG_PICK_EXE, L"Выбрать .exe или .zip");
        SetWindowPos(b1, nullptr, 160, y, 230, 36, SWP_NOZORDER);
        dlgExeInfo = CreateWindowExW(0, L"STATIC", L"Файл не выбран",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 400, y + 8, 240, 24, h,
            (HMENU)ID_DLG_EXE_INFO, GetModuleHandle(nullptr), nullptr);
        SendMessageW(dlgExeInfo, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);
        y += 46;
        HWND info1 = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            160, y, 480, 160, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(info1, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)info1);
        y += 172;
        lbl(L"Обложка:", y);
        HWND b2 = MakeButton(h, ID_DLG_PICK_COVER, L"Выбрать картинку");
        SetWindowPos(b2, nullptr, 160, y, 200, 34, SWP_NOZORDER);
        dlgCoverInfo = CreateWindowExW(0, L"STATIC", L"(необязательно)",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 370, y + 8, 270, 24, h,
            (HMENU)ID_DLG_COVER_INFO, GetModuleHandle(nullptr), nullptr);
        SendMessageW(dlgCoverInfo, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);
        y += 55;
        HWND pb = MakeButton(h, ID_DLG_PUBLISH, L"Опубликовать");
        SetWindowPos(pb, nullptr, 20, y, 240, 44, SWP_NOZORDER);
        HWND cb = MakeButton(h, ID_DLG_CANCEL_BTN, L"Отмена");
        SetWindowPos(cb, nullptr, 280, y, 160, 44, SWP_NOZORDER);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_DLG_PICK_EXE) {
            wchar_t buf[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = h;
            ofn.lpstrFilter = L"Игра или архив\0*.exe;*.zip;*.msi\0Все\0*.*\0\0";
            ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Файл игры";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn)) {
                g_dlgExePath = buf;
                SetWindowTextW(dlgExeInfo, GetFileNameFromPath(g_dlgExePath).c_str());
                WIN32_FILE_ATTRIBUTE_DATA fad;
                if (GetFileAttributesExW(g_dlgExePath.c_str(), GetFileExInfoStandard, &fad)) {
                    long long sz = ((long long)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
                    std::wstring info = L"Размер: " + HumanSize(sz) + L"\r\nТип: " + GetFileExt(g_dlgExePath);
                    if (GetFileExt(g_dlgExePath) == L".zip") info += L"\r\n(ZIP будет распакован)";
                    HWND in = (HWND)GetWindowLongPtrW(h, GWLP_USERDATA);
                    if (in) SetWindowTextW(in, info.c_str());
                }
            }
        }
        else if (id == ID_DLG_PICK_COVER) {
            wchar_t buf[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = h;
            ofn.lpstrFilter = L"Картинки\0*.png;*.jpg;*.jpeg;*.bmp\0Все\0*.*\0\0";
            ofn.lpstrFile = buf; ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = L"Обложка";
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn)) {
                g_dlgCoverPath = buf;
                SetWindowTextW(dlgCoverInfo, GetFileNameFromPath(g_dlgCoverPath).c_str());
            }
        }
        else if (id == ID_DLG_PUBLISH) {
            wchar_t name[256] = {}, ver[64] = {}, desc[1024] = {};
            GetWindowTextW(dlgName, name, 256);
            GetWindowTextW(dlgVer, ver, 64);
            GetWindowTextW(dlgDesc, desc, 1024);
            if (wcslen(name) == 0 || g_dlgExePath.empty()) {
                MessageBoxW(h, L"Введите название и выберите файл.", L"", MB_OK);
                break;
            }
            std::wstring ext = GetFileExt(g_dlgExePath);
            std::wstring folder = g_installRoot + L"\\" + name;
            if (ext == L".zip") {
                if (!ExtractZipTo(g_dlgExePath, folder)) {
                    MessageBoxW(h, L"Не удалось распаковать.", L"", MB_OK | MB_ICONERROR);
                    break;
                }
            }
            else {
                EnsureGameFolder(folder);
                std::wstring dst = folder + L"\\" + GetFileNameFromPath(g_dlgExePath);
                CopyFileW(g_dlgExePath.c_str(), dst.c_str(), FALSE);
            }
            if (!g_dlgCoverPath.empty()) {
                std::wstring cdst = folder + L"\\cover" + GetFileExt(g_dlgCoverPath);
                CopyFileW(g_dlgCoverPath.c_str(), cdst.c_str(), FALSE);
            }
            GameItem g;
            g.id = g_nextId++;
            g.name = name;
            g.version = (wcslen(ver) ? ver : L"v1.0.0");
            g.description = desc;
            g.installPath = folder;
            g.installedAt = NowStr();
            g.coverPath = g_dlgCoverPath;
            g.mine = true;
            g.sizeMB = 10;
            g_library.push_back(g);
            SaveLibrary(); RefreshLibrary(); RefreshMyGames(); RefreshStats();
            MessageBoxW(h, L"Игра опубликована.", L"", MB_OK);
            DestroyWindow(h);
        }
        else if (id == ID_DLG_CANCEL_BTN) DestroyWindow(h);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_p.text);
        SetBkColor(dc, g_p.content);
        return (LRESULT)g_brContent;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lp;
        if (dis->CtlType == ODT_BUTTON) {
            int id = (int)dis->CtlID;
            COLORREF bg = g_p.accent, fg = RGB(255, 255, 255);
            if (id == ID_DLG_PUBLISH) bg = g_p.green;
            if (id == ID_DLG_CANCEL_BTN) { bg = g_p.neutral; fg = g_p.text; }
            HBRUSH br = CreateSolidBrush(bg);
            FillRect(dis->hDC, &dis->rcItem, br);
            DeleteObject(br);
            wchar_t t[128] = {};
            GetWindowTextW(dis->hwndItem, t, 128);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, fg);
            HFONT of = (HFONT)SelectObject(dis->hDC, g_fontBody);
            DrawTextW(dis->hDC, t, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dis->hDC, of);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE: DestroyWindow(h); break;
    case WM_DESTROY:
        EnableWindow(GetParent(h), TRUE);
        SetForegroundWindow(GetParent(h));
        break;
    }
    return DefWindowProcW(h, m, wp, lp);
}

LRESULT CALLBACK UrlDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp);
static HWND urlEdit = nullptr, urlNameEdit = nullptr, urlStatus = nullptr;

void ShowUrlDialog(HWND parent) {
    const wchar_t* CLS = L"GameHubUrlDlg";
    static bool reg = false;
    if (!reg) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = UrlDlgProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLS;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(g_p.content);
        RegisterClassW(&wc);
        reg = true;
    }
    EnableWindow(parent, FALSE);
    HWND dlg = CreateWindowExW(WS_EX_DLGMODALFRAME, CLS,
        L"Установить по ссылке",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 660, 420,
        parent, nullptr, GetModuleHandle(nullptr), nullptr);
    ShowWindow(dlg, SW_SHOW);
    UpdateWindow(dlg);
}

LRESULT CALLBACK UrlDlgProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE: {
        auto lbl = [&](const wchar_t* t, int y) {
            HWND l = CreateWindowExW(0, L"STATIC", t, WS_CHILD | WS_VISIBLE | SS_LEFT,
                20, y, 130, 24, h, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessageW(l, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
            };
        lbl(L"Ссылка (URL):", 20);
        urlEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"https://",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            160, 20, 470, 28, h, (HMENU)ID_URL_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(urlEdit, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
        lbl(L"Название:", 60);
        urlNameEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            160, 60, 470, 28, h, (HMENU)ID_URL_NAME_EDIT, GetModuleHandle(nullptr), nullptr);
        SendMessageW(urlNameEdit, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
        urlStatus = CreateWindowExW(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            20, 150, 610, 100, h, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessageW(urlStatus, WM_SETFONT, (WPARAM)g_fontSmall, TRUE);
        HWND ib = MakeButton(h, ID_URL_INSTALL, L"Скачать и установить");
        SetWindowPos(ib, nullptr, 20, 310, 260, 44, SWP_NOZORDER);
        HWND cb = MakeButton(h, ID_URL_CANCEL, L"Отмена");
        SetWindowPos(cb, nullptr, 300, 310, 150, 44, SWP_NOZORDER);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        if (id == ID_URL_INSTALL) {
            wchar_t url[1024] = {}, nm[256] = {};
            GetWindowTextW(urlEdit, url, 1024);
            GetWindowTextW(urlNameEdit, nm, 256);
            if (wcslen(url) < 10 || wcslen(nm) == 0) {
                SetWindowTextW(urlStatus, L"Введите ссылку и название.");
                break;
            }
            std::wstring fname = GetFileNameFromPath(url);
            if (fname.empty() || fname.find(L'.') == std::wstring::npos) fname = L"download.exe";
            std::wstring folder = g_installRoot + L"\\" + nm;
            EnsureGameFolder(folder);
            std::wstring dest = folder + L"\\" + fname;
            SetWindowTextW(urlStatus, L"Скачивание...");
            UpdateWindow(h);
            HRESULT hr = URLDownloadToFileW(nullptr, url, dest.c_str(), 0, nullptr);
            if (SUCCEEDED(hr)) {
                if (GetFileExt(dest) == L".zip") {
                    SetWindowTextW(urlStatus, L"Распаковка...");
                    UpdateWindow(h);
                    ExtractZipTo(dest, folder);
                    DeleteFileW(dest.c_str());
                }
                GameItem g;
                g.id = g_nextId++;
                g.name = nm; g.version = L"v1.0.0";
                g.description = L"Установлено по ссылке";
                g.installPath = folder;
                g.installedAt = NowStr();
                g.mine = true;
                g_library.push_back(g);
                SaveLibrary(); RefreshLibrary(); RefreshMyGames(); RefreshStats();
                MessageBoxW(h, L"Установлено.", L"", MB_OK);
                DestroyWindow(h);
            }
            else {
                SetWindowTextW(urlStatus, L"Ошибка скачивания.");
            }
        }
        else if (id == ID_URL_CANCEL) DestroyWindow(h);
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_p.text);
        SetBkColor(dc, g_p.content);
        return (LRESULT)g_brContent;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lp;
        if (dis->CtlType == ODT_BUTTON) {
            int id = (int)dis->CtlID;
            COLORREF bg = (id == ID_URL_INSTALL) ? g_p.green : g_p.neutral;
            HBRUSH br = CreateSolidBrush(bg);
            FillRect(dis->hDC, &dis->rcItem, br);
            DeleteObject(br);
            wchar_t t[128] = {};
            GetWindowTextW(dis->hwndItem, t, 128);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(255, 255, 255));
            HFONT of = (HFONT)SelectObject(dis->hDC, g_fontBody);
            DrawTextW(dis->hDC, t, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dis->hDC, of);
            return TRUE;
        }
        break;
    }
    case WM_CLOSE: DestroyWindow(h); break;
    case WM_DESTROY:
        EnableWindow(GetParent(h), TRUE);
        SetForegroundWindow(GetParent(h));
        break;
    }
    return DefWindowProcW(h, m, wp, lp);
}

// ============================================================
// Создание контролов
// ============================================================
void CreateControls(HWND hWnd) {
    const wchar_t* nav[] = { L"Магазин", L"Библиотека", L"Статистика", L"Мои игры", L"Настройки" };
    for (int i = 0; i < 5; ++i)
        g_hNav[i] = MakeButton(hWnd, ID_NAV_CATALOG + i, nav[i], g_fontNav);

    g_hSourceCombo = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST,
        0, 0, 0, 0, hWnd, (HMENU)ID_SOURCE_COMBO, GetModuleHandle(nullptr), nullptr);
    SendMessageW(g_hSourceCombo, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
    SendMessageW(g_hSourceCombo, CB_ADDSTRING, 0, (LPARAM)L"Локальный каталог");
    SendMessageW(g_hSourceCombo, CB_SETCURSEL, 0, 0);

    g_hSearchEdit = CreateWindowExW(0, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
        0, 0, 0, 0, hWnd, (HMENU)ID_SEARCH_EDIT, GetModuleHandle(nullptr), nullptr);
    SendMessageW(g_hSearchEdit, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
    SendMessageW(g_hSearchEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Поиск...");

    g_hSearchBtn = MakeButton(hWnd, ID_SEARCH_BTN, L"Найти");

    g_hCatalogList = CreateWindowExW(0, WC_LISTVIEWW, L"",
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    StyleList(g_hCatalogList);

    g_hCatalogStatus = MakeLabel(hWnd, L"Каталог пуст", g_fontSmall);

    g_hProgressBar = CreateWindowExW(0, PROGRESS_CLASSW, L"",
        WS_CHILD | PBS_SMOOTH, 0, 0, 0, 0, hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    SendMessageW(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    g_hLibraryList = CreateWindowExW(0, WC_LISTVIEWW, L"",
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    StyleList(g_hLibraryList);
    g_hLibraryStatus = MakeLabel(hWnd, L"Установлено игр: 0", g_fontSmall);

    g_hStatsTitle = MakeLabel(hWnd, L"Статистика", g_fontHead);
    g_hStatsBody = MakeLabel(hWnd, L"", g_fontBody);

    g_hProfileTitle = MakeLabel(hWnd, L"Профиль", g_fontHead);
    g_hProfileNameLbl = MakeLabel(hWnd, L"Ваше имя:", g_fontBody);
    g_hProfileNameEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd,
        (HMENU)ID_PROFILE_NAME, GetModuleHandle(nullptr), nullptr);
    SendMessageW(g_hProfileNameEdit, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
    SetWindowTextW(g_hProfileNameEdit, g_profileName.c_str());
    g_hProfileSaveBtn = MakeButton(hWnd, ID_PROFILE_SAVE, L"Сохранить имя");
    g_hProfileUploadBtn = MakeButton(hWnd, ID_PROFILE_UPLOAD, L"Опубликовать игру");
    g_hProfileMyLbl = MakeLabel(hWnd, L"Мои игры: 0", g_fontBody);
    g_hProfileMyList = CreateWindowExW(0, WC_LISTVIEWW, L"",
        WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        0, 0, 0, 0, hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
    StyleList(g_hProfileMyList);
    g_hProfileStatus = MakeLabel(hWnd, L"", g_fontSmall);

    g_hSettingsTitle = MakeLabel(hWnd, L"Настройки", g_fontHead);
    g_hSettingsBody = MakeLabel(hWnd,
        L"Тема:              переключается кнопками ниже\r\n"
        L"Папка установки:   C:\\Games\\GameHub\r\n"
        L"Библиотека:        %APPDATA%\\GameHubLauncher\\library.dat\r\n"
        L"Обложки:           cover.png/jpg рядом с .exe\r\n"
        L"Версия:            " APP_VERSION,
        g_fontBody);
    g_hThemeDarkBtn = MakeButton(hWnd, ID_THEME_DARK, L"Тёмная тема");
    g_hThemeLightBtn = MakeButton(hWnd, ID_THEME_LIGHT, L"Светлая тема");
    g_hOpenInstallBtn = MakeButton(hWnd, ID_OPEN_INSTALL, L"Открыть папку установки");
    g_hClearLibBtn = MakeButton(hWnd, ID_CLEAR_LIBRARY, L"Очистить библиотеку");
    g_hSettingsStatus = MakeLabel(hWnd, L"", g_fontSmall);

    // Детальная панель — только текст
    g_hDetTitle = MakeLabel(hWnd, L"", g_fontHead);
    g_hDetInfo = MakeLabel(hWnd, L"", g_fontSmall);

    g_hDetDesc = CreateWindowExW(0, L"EDIT", L"",
        WS_CHILD | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL,
        0, 0, 0, 0, hWnd, (HMENU)ID_DET_DESC, GetModuleHandle(nullptr), nullptr);
    SendMessageW(g_hDetDesc, WM_SETFONT, (WPARAM)g_fontBody, TRUE);
    SendMessageW(g_hDetDesc, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
        MAKELPARAM(0, 0));

    g_hDetBtn1 = MakeButton(hWnd, ID_DET_PLAY, L"Играть");
    g_hDetBtn2 = MakeButton(hWnd, ID_DET_UPDATE, L"Обновить");
    g_hDetBtn3 = MakeButton(hWnd, ID_DET_FOLDER, L"Открыть папку");
    g_hDetBtn4 = MakeButton(hWnd, ID_DET_DELETE, L"Удалить");

    InitCoverList();

    RefreshCatalog();
    RefreshLibrary();
    ShowPage(0);
}

// ============================================================
// Раскладка
// ============================================================
void OnSize(int w, int h) {
    int navY = 120, navH = 46, gap = 8;
    for (int i = 0; i < 5; ++i)
        SetWindowPos(g_hNav[i], nullptr, 12, navY + i * (navH + gap),
            SIDEBAR_W - 24, navH, SWP_NOZORDER);

    int cx = SIDEBAR_W;
    int listX = cx + PAD;
    int listW = LIST_W;
    int detX = cx + LIST_W + PAD;
    int detW = w - cx - LIST_W - PAD * 2;

    int topY = PAD;
    SetWindowPos(g_hSourceCombo, nullptr, listX, topY, 160, 32, SWP_NOZORDER);
    SetWindowPos(g_hSearchBtn, nullptr, listX + listW - 90, topY, 90, 32, SWP_NOZORDER);
    SetWindowPos(g_hSearchEdit, nullptr, listX + 170, topY,
        listW - 170 - 90 - 10, 32, SWP_NOZORDER);

    int listY = topY + 50;
    int statusY = h - PAD - 22;
    int listH = statusY - listY - 30;

    SetWindowPos(g_hCatalogList, nullptr, listX, listY, listW, listH, SWP_NOZORDER);
    SetWindowPos(g_hProgressBar, nullptr, listX, statusY - 16, listW, 8, SWP_NOZORDER);
    SetWindowPos(g_hCatalogStatus, nullptr, listX, statusY, listW, 20, SWP_NOZORDER);

    SetWindowPos(g_hLibraryList, nullptr, listX, PAD, listW, h - PAD * 2 - 30, SWP_NOZORDER);
    SetWindowPos(g_hLibraryStatus, nullptr, listX, h - PAD - 22, listW, 20, SWP_NOZORDER);

    int py = PAD;
    SetWindowPos(g_hProfileTitle, nullptr, cx + PAD, py, 200, 30, SWP_NOZORDER);
    SetWindowPos(g_hProfileNameLbl, nullptr, cx + PAD, py + 40, 110, 28, SWP_NOZORDER);
    SetWindowPos(g_hProfileNameEdit, nullptr, cx + PAD + 120, py + 40, 260, 30, SWP_NOZORDER);
    SetWindowPos(g_hProfileSaveBtn, nullptr, cx + PAD + 390, py + 40, 150, 30, SWP_NOZORDER);
    SetWindowPos(g_hProfileUploadBtn, nullptr, cx + PAD, py + 85, 220, 40, SWP_NOZORDER);
    SetWindowPos(g_hProfileMyLbl, nullptr, listX, py + 140, listW, 26, SWP_NOZORDER);
    SetWindowPos(g_hProfileMyList, nullptr, listX, py + 170,
        listW, h - py - 170 - PAD - 30, SWP_NOZORDER);
    SetWindowPos(g_hProfileStatus, nullptr, listX, h - PAD - 22, listW, 20, SWP_NOZORDER);

    SetWindowPos(g_hStatsTitle, nullptr, cx + PAD, PAD, 300, 30, SWP_NOZORDER);
    SetWindowPos(g_hStatsBody, nullptr, cx + PAD, PAD + 50,
        w - cx - PAD * 2, h - PAD * 2 - 50, SWP_NOZORDER);

    SetWindowPos(g_hSettingsTitle, nullptr, cx + PAD, PAD, 300, 30, SWP_NOZORDER);
    SetWindowPos(g_hSettingsBody, nullptr, cx + PAD, PAD + 50,
        w - cx - PAD * 2, 160, SWP_NOZORDER);
    int sby = PAD + 220;
    SetWindowPos(g_hThemeDarkBtn, nullptr, cx + PAD, sby, 190, 42, SWP_NOZORDER);
    SetWindowPos(g_hThemeLightBtn, nullptr, cx + PAD + 200, sby, 190, 42, SWP_NOZORDER);
    SetWindowPos(g_hOpenInstallBtn, nullptr, cx + PAD, sby + 55, 280, 42, SWP_NOZORDER);
    SetWindowPos(g_hClearLibBtn, nullptr, cx + PAD, sby + 105, 280, 42, SWP_NOZORDER);
    SetWindowPos(g_hSettingsStatus, nullptr, cx + PAD, sby + 165, 400, 20, SWP_NOZORDER);

    // Детальная панель — вертикальный стек без картинки
    int detY = PAD;
    SetWindowPos(g_hDetTitle, nullptr, detX, detY, detW, 34, SWP_NOZORDER);
    SetWindowPos(g_hDetInfo, nullptr, detX, detY + 38, detW, 24, SWP_NOZORDER);

    int buttonsY = h - PAD - 46;
    int descY = detY + 72;
    int descH = buttonsY - descY - 16;
    if (descH < 80) descH = 80;

    SetWindowPos(g_hDetDesc, nullptr, detX, descY, detW, descH, SWP_NOZORDER);

    int bw = 150, bgap = 10;
    SetWindowPos(g_hDetBtn1, nullptr, detX, buttonsY, bw, 46, SWP_NOZORDER);
    SetWindowPos(g_hDetBtn2, nullptr, detX + bw + bgap, buttonsY, bw, 46, SWP_NOZORDER);
    SetWindowPos(g_hDetBtn3, nullptr, detX + (bw + bgap) * 2, buttonsY, bw + 30, 46, SWP_NOZORDER);
    SetWindowPos(g_hDetBtn4, nullptr, detX + (bw + bgap) * 3 + 30, buttonsY, bw, 46, SWP_NOZORDER);
}

// ============================================================
// Отрисовка
// ============================================================
void OnPaint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    RECT rc; GetClientRect(hWnd, &rc);
    SetBkMode(hdc, TRANSPARENT);

    HFONT of = (HFONT)SelectObject(hdc, g_fontLogo);
    SetTextColor(hdc, g_p.text);
    RECT r1 = { 20, 30, SIDEBAR_W - 10, 65 };
    DrawTextW(hdc, L"GameHub", -1, &r1, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, g_fontSmall);
    SetTextColor(hdc, g_p.accent);
    RECT r2 = { 20, 65, SIDEBAR_W - 10, 85 };
    DrawTextW(hdc, L"LAUNCHER", -1, &r2, DT_LEFT | DT_SINGLELINE);

    SetTextColor(hdc, g_p.textDim);
    RECT r3 = { 20, rc.bottom - 55, SIDEBAR_W - 10, rc.bottom - 35 };
    DrawTextW(hdc, L"Версия " APP_VERSION, -1, &r3, DT_LEFT | DT_SINGLELINE);

    std::wstring prof = g_profileName;
    RECT r4 = { 20, rc.bottom - 30, SIDEBAR_W - 10, rc.bottom - 10 };
    DrawTextW(hdc, prof.c_str(), -1, &r4, DT_LEFT | DT_SINGLELINE);

    SelectObject(hdc, of);

    HPEN pen = CreatePen(PS_SOLID, 1, g_p.border);
    HPEN op = (HPEN)SelectObject(hdc, pen);
    MoveToEx(hdc, SIDEBAR_W, 0, nullptr);
    LineTo(hdc, SIDEBAR_W, rc.bottom);

    if (g_currentPage == 0 || g_currentPage == 1 || g_currentPage == 3) {
        int sepX = SIDEBAR_W + LIST_W;
        MoveToEx(hdc, sepX, PAD, nullptr);
        LineTo(hdc, sepX, rc.bottom - PAD);
    }

    SelectObject(hdc, op);
    DeleteObject(pen);
    EndPaint(hWnd, &ps);
}

BOOL OnDrawItem(LPDRAWITEMSTRUCT dis) {
    if (dis->CtlType != ODT_BUTTON) return FALSE;
    int id = (int)dis->CtlID;
    bool isNav = (id >= ID_NAV_CATALOG && id <= ID_NAV_SETTINGS);
    bool isDis = (dis->itemState & ODS_DISABLED) != 0;
    bool isPre = (dis->itemState & ODS_SELECTED) != 0;

    COLORREF bg; COLORREF fg = RGB(255, 255, 255);
    if (isNav) {
        int i = id - ID_NAV_CATALOG;
        bg = (i == g_currentPage) ? g_p.accent : g_p.navInactive;
        fg = g_p.text;
    }
    else if (isDis) { bg = g_p.disabled; fg = g_p.disabledText; }
    else if (id == ID_DET_PLAY || id == ID_PROFILE_UPLOAD)
        bg = isPre ? g_p.greenP : g_p.green;
    else if (id == ID_DET_UPDATE || id == ID_UPD_APPLY)
        bg = isPre ? RGB(200, 145, 40) : RGB(230, 165, 50);
    else if (id == ID_DET_DELETE || id == ID_CLEAR_LIBRARY)
        bg = isPre ? g_p.redP : g_p.red;
    else if (id == ID_DET_FOLDER || id == ID_OPEN_INSTALL ||
        id == ID_PROFILE_SAVE || id == ID_UPD_CANCEL)
    {
        bg = isPre ? g_p.neutralP : g_p.neutral; fg = g_p.text;
    }
    else if (id == ID_THEME_DARK)
    {
        bg = g_darkTheme ? (isPre ? g_p.accentP : g_p.accent) : g_p.neutral;
        fg = g_darkTheme ? RGB(255, 255, 255) : g_p.text;
    }
    else if (id == ID_THEME_LIGHT)
    {
        bg = !g_darkTheme ? (isPre ? g_p.accentP : g_p.accent) : g_p.neutral;
        fg = !g_darkTheme ? RGB(255, 255, 255) : g_p.text;
    }
    else bg = isPre ? g_p.accentP : g_p.accent;

    HBRUSH br = CreateSolidBrush(bg);
    FillRect(dis->hDC, &dis->rcItem, br);
    DeleteObject(br);

    wchar_t t[256] = {};
    GetWindowTextW(dis->hwndItem, t, 256);
    SetBkMode(dis->hDC, TRANSPARENT);
    SetTextColor(dis->hDC, fg);
    HFONT f = isNav ? g_fontNav : g_fontBody;
    HFONT of = (HFONT)SelectObject(dis->hDC, f);
    RECT rc = dis->rcItem;
    UINT fl = DT_SINGLELINE | DT_VCENTER;
    if (isNav) { rc.left += 26; fl |= DT_LEFT; }
    else fl |= DT_CENTER;
    DrawTextW(dis->hDC, t, -1, &rc, fl);
    SelectObject(dis->hDC, of);
    return TRUE;
}

void UpdateDetailsFromSelection() {
    if (g_currentPage == 0) {
        GameItem* g = CatalogByIndex(SelectedCatalog());
        ShowGameDetails(g, 0);
    }
    else if (g_currentPage == 1) {
        GameItem* g = LibraryByIndex(SelectedLibrary());
        ShowGameDetails(g, 1);
    }
    else if (g_currentPage == 3) {
        GameItem* g = MyByIndex(SelectedMy());
        ShowGameDetails(g, 2);
    }
}

// ============================================================
// Команды
// ============================================================
void OnCommand(HWND hWnd, int id) {
    switch (id) {
    case ID_NAV_CATALOG:  ShowPage(0); break;
    case ID_NAV_LIBRARY:  ShowPage(1); break;
    case ID_NAV_STATS:    ShowPage(2); break;
    case ID_NAV_PROFILE:  ShowPage(3); break;
    case ID_NAV_SETTINGS: ShowPage(4); break;

    case ID_SEARCH_BTN: {
        wchar_t b[256] = {};
        GetWindowTextW(g_hSearchEdit, b, 256);
        RefreshCatalog(b);
        break;
    }

    case ID_DET_PLAY: {
        if (g_currentPage == 0) {
            StartDownload(hWnd);
        }
        else {
            GameItem* g = (g_currentPage == 1) ? LibraryByIndex(SelectedLibrary())
                : MyByIndex(SelectedMy());
            RunGame(hWnd, g);
        }
        break;
    }
    case ID_DET_UPDATE: {
        GameItem* g = (g_currentPage == 1) ? LibraryByIndex(SelectedLibrary())
            : MyByIndex(SelectedMy());
        if (!g) break;
        if (g->mine) ShowUpdateGameDialog(hWnd, g);
        else UpdateGameFromCatalog(hWnd, g);
        UpdateDetailsFromSelection();
        break;
    }
    case ID_DET_FOLDER: {
        GameItem* g = (g_currentPage == 1) ? LibraryByIndex(SelectedLibrary())
            : MyByIndex(SelectedMy());
        if (g) OpenInExplorer(g->installPath);
        break;
    }
    case ID_DET_DELETE: {
        int i = (g_currentPage == 1) ? SelectedLibrary() : SelectedMy();
        bool mine = (g_currentPage == 3);
        DeleteLibraryItem(hWnd, i, mine);
        UpdateDetailsFromSelection();
        break;
    }

    case ID_PROFILE_SAVE: {
        wchar_t b[256] = {};
        GetWindowTextW(g_hProfileNameEdit, b, 256);
        if (wcslen(b) > 0) {
            g_profileName = b;
            SaveProfile();
            SetWindowTextW(g_hProfileStatus, L"Имя сохранено");
            InvalidateRect(hWnd, nullptr, TRUE);
        }
        break;
    }
    case ID_PROFILE_UPLOAD:
        ShowUploadDialog(hWnd);
        break;

    case ID_THEME_DARK: ApplyTheme(true); SetWindowTextW(g_hSettingsStatus, L"Тема: тёмная"); break;
    case ID_THEME_LIGHT: ApplyTheme(false); SetWindowTextW(g_hSettingsStatus, L"Тема: светлая"); break;
    case ID_OPEN_INSTALL: EnsureGameFolder(g_installRoot); OpenInExplorer(g_installRoot); break;
    case ID_CLEAR_LIBRARY: ClearLibrary(hWnd); break;
    }
}

// ============================================================
// WndProc
// ============================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_CREATE:
        g_hMainWnd = hWnd;
        CreateControls(hWnd);
        break;

    case WM_SIZE:
        OnSize(LOWORD(lp), HIWORD(lp));
        InvalidateRect(hWnd, nullptr, TRUE);
        break;

    case WM_TIMER:
        if (wp == ID_TIMER_DOWNLOAD) {
            g_downloadTick += 4;
            SendMessageW(g_hProgressBar, PBM_SETPOS, g_downloadTick, 0);
            if (g_downloadTick >= 100) FinishDownload(hWnd);
        }
        break;

    case WM_ERASEBKGND: {
        HDC dc = (HDC)wp;
        RECT rc; GetClientRect(hWnd, &rc);
        RECT a = { 0, 0, SIDEBAR_W, rc.bottom };
        FillRect(dc, &a, g_brSidebar);
        RECT b = { SIDEBAR_W, 0, rc.right, rc.bottom };
        FillRect(dc, &b, g_brContent);
        if (g_currentPage == 0 || g_currentPage == 1 || g_currentPage == 3) {
            int topY = (g_currentPage == 3) ? PAD + 140 : PAD;
            RECT c = { SIDEBAR_W + PAD - 4, topY - 4,
                      SIDEBAR_W + LIST_W + 4, rc.bottom - PAD + 4 };
            FillRect(dc, &c, g_brPanel);
        }
        return 1;
    }

    case WM_PAINT: OnPaint(hWnd); break;
    case WM_DRAWITEM: return OnDrawItem((LPDRAWITEMSTRUCT)lp);

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g_p.text);
        SetBkColor(dc, g_p.content);
        return (LRESULT)g_brContent;
    }

    case WM_COMMAND: {
        int id = LOWORD(wp), code = HIWORD(wp);
        if (id == ID_SEARCH_EDIT && code == EN_CHANGE) {
            wchar_t b[256] = {};
            GetWindowTextW(g_hSearchEdit, b, 256);
            RefreshCatalog(b);
            break;
        }
        OnCommand(hWnd, id);
        break;
    }

    case WM_NOTIFY: {
        LPNMHDR nh = (LPNMHDR)lp;
        if (nh->hwndFrom == g_hCatalogList && nh->code == NM_DBLCLK) {
            StartDownload(hWnd); return 0;
        }
        if (nh->hwndFrom == g_hLibraryList && nh->code == NM_DBLCLK) {
            RunGame(hWnd, LibraryByIndex(SelectedLibrary())); return 0;
        }
        if (nh->hwndFrom == g_hProfileMyList && nh->code == NM_DBLCLK) {
            RunGame(hWnd, MyByIndex(SelectedMy())); return 0;
        }
        if ((nh->hwndFrom == g_hCatalogList ||
            nh->hwndFrom == g_hLibraryList ||
            nh->hwndFrom == g_hProfileMyList) && nh->code == LVN_ITEMCHANGED) {
            UpdateDetailsFromSelection();
        }
        if (nh->code == NM_CUSTOMDRAW) {
            LPNMLVCUSTOMDRAW cd = (LPNMLVCUSTOMDRAW)lp;
            if (cd->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
            if (cd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
                cd->clrText = g_p.text;
                cd->clrTextBk = g_p.panel;
                return CDRF_NEWFONT;
            }
        }
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, ID_TIMER_DOWNLOAD);
        if (g_hCoverList) { ImageList_Destroy(g_hCoverList); g_hCoverList = nullptr; }
        PostQuitMessage(0);
        break;

    default: return DefWindowProcW(hWnd, m, wp, lp);
    }
    return 0;
}

// ============================================================
// Точка входа
// ============================================================
int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int nShow) {
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    GdiplusStartupInput gdiIn;
    GdiplusStartup(&g_gdiToken, &gdiIn, nullptr);

    g_fontLogo = MakeFont(24, FW_BOLD);
    g_fontBody = MakeFont(14, FW_NORMAL);
    g_fontNav = MakeFont(15, FW_NORMAL);
    g_fontHead = MakeFont(20, 600);
    g_fontSmall = MakeFont(12, FW_NORMAL);

    g_brContent = CreateSolidBrush(g_p.content);
    g_brSidebar = CreateSolidBrush(g_p.sidebar);
    g_brPanel = CreateSolidBrush(g_p.panel);

    LoadCatalog();
    LoadLibrary();
    LoadProfile();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"GameHubLauncherMain";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hWnd = CreateWindowExW(0, L"GameHubLauncherMain", L"GameHub Launcher",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1250, 800,
        nullptr, nullptr, hInst, nullptr);

    if (!hWnd) {
        GdiplusShutdown(g_gdiToken);
        return 0;
    }
    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(g_gdiToken);
    return 0;
}