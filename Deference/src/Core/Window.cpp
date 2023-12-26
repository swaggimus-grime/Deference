#include "Window.h"
#include <imgui_impl_win32.h>
#include <comdef.h>
#include <ShObjIdl.h>
#include <shlobj.h>
#include "Debug/Exception.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <filesystem>
#include <shlobj.h>

static std::wstring GetLatestWinPixGpuCapturerPath_Cpp17()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::filesystem::path pixInstallationPath = programFilesPath;
	pixInstallationPath /= "Microsoft PIX";

	std::wstring newestVersionFound;

	for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
	{
		if (directory_entry.is_directory())
		{
			if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
			{
				newestVersionFound = directory_entry.path().filename().c_str();
			}
		}
	}

	if (newestVersionFound.empty())
	{
		// TODO: Error, no PIX installation found
	}

	return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}

static std::wstring GetLatestNvidiaNsightPath()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::filesystem::path pixInstallationPath = programFilesPath;
	pixInstallationPath /= "Microsoft PIX";

	std::wstring newestVersionFound;

	for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
	{
		if (directory_entry.is_directory())
		{
			if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
			{
				newestVersionFound = directory_entry.path().filename().c_str();
			}
		}
	}

	if (newestVersionFound.empty())
	{
		// TODO: Error, no PIX installation found
	}

	return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}

Window::Window(const std::string& name, unsigned int width, unsigned int height)
	:m_Inst(GetModuleHandle(nullptr)), m_Name(name), m_Width(width), m_Height(height)
{
	//SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	HR CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	/*if (GetModuleHandleW(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibraryW(GetLatestWinPixGpuCapturerPath_Cpp17().c_str());
	}*/

	WNDCLASSEX wc{};
	HINSTANCE hInst = m_Inst;
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = SetupMessageProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = m_Name.c_str();
	RegisterClassEx(&wc);

	m_OnResize = [](UINT, UINT) {};

	RECT wr = { 0, 0, m_Width, m_Height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	m_Handle = CreateWindowEx(NULL, m_Name.c_str(), m_Name.c_str(),
		WS_OVERLAPPEDWINDOW,
		0, 0,
		wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr,
		hInst, this);
	BR m_Handle;

	ImGui_ImplWin32_Init(m_Handle);
	ShowWindow(m_Handle, SW_SHOW);
	ShowCursor(true);
	GetClipCursor(&m_OrigClipRect);
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
	UI::Shutdown();
	DestroyWindow(m_Handle);
	UnregisterClass(m_Name.c_str(), m_Inst);
}

 std::optional<INT> Window::Update()
{
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
		case WM_QUIT:
			return static_cast<INT>(msg.wParam);
		default:
			//translate any virtual key message into WM_CHAR message
			TranslateMessage(&msg);
			//Send the message to the window procedure
			DispatchMessage(&msg);
		}
	}

	return {};
}

std::wstring Window::OpenDialogBoxW()
 {
	 PWSTR pszFilePath;
	 HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		 COINIT_DISABLE_OLE1DDE);
	 if (SUCCEEDED(hr))
	 {
		 IFileOpenDialog* pFileOpen = nullptr;

		 // Create the FileOpenDialog object.
		 hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			 IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		 if (SUCCEEDED(hr))
		 {
			 // Show the Open dialog box.
			 hr = pFileOpen->Show(NULL);

			 // Get the file name from the dialog box.
			 if (SUCCEEDED(hr))
			 {
				 IShellItem* pItem;
				 hr = pFileOpen->GetResult(&pItem);
				 if (SUCCEEDED(hr))
				 {
					 hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					 // Display the file name to the user.
					 if (SUCCEEDED(hr))
					 {
						 CoTaskMemFree(pszFilePath);
					 }
					 pItem->Release();
				 }
			 }
			 pFileOpen->Release();
		 }
		 CoUninitialize();
	 }

	 return pszFilePath;
 }

static int CALLBACK BrowseFolderCallback(
	HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		LPCTSTR path = reinterpret_cast<LPCTSTR>(lpData);
		::SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)path);
	}
	return 0;
}

std::string Window::FolderDialogBox()
{
	TCHAR path[MAX_PATH];

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = ("Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseFolderCallback;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return "";
}

std::wstring Window::FolderDialogBoxW()
{
	const auto& folderPath = FolderDialogBox();
	return std::wstring(folderPath.begin(), folderPath.end());
}

std::string Window::OpenDialogBox()
{
	const auto& wPath = OpenDialogBoxW();
	return std::string(wPath.begin(), wPath.end());
}

void Window::ClipCursor(BOOL clip)
{
	if (!clip) {
		::ClipCursor(&m_OrigClipRect);
		return;
	}

	RECT rect;
	GetClientRect(m_Handle, &rect);
	MapWindowPoints(m_Handle, nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
	::ClipCursor(&rect);
}

LRESULT Window::SetupMessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg != WM_NCCREATE)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	const CREATESTRUCTW createInfo = *reinterpret_cast<CREATESTRUCTW*>(lParam);
	Window* window = static_cast<Window*>(createInfo.lpCreateParams);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
	SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::MessageProc));
	return window->HandleMessage(hWnd, msg, wParam, lParam);
}

LRESULT Window::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA))->HandleMessage(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		m_Width = LOWORD(lParam);
		m_Height = HIWORD(lParam);
		m_OnResize(m_Width, m_Height);
		break;
	case WM_KEYDOWN:
		m_Input.OnKeyPressed(static_cast<UINT>(wParam));
		break;
	case WM_KEYUP:
		m_Input.OnKeyReleased(static_cast<UINT>(wParam));
		break;
	case WM_MOUSEMOVE:
		POINTS mp = MAKEPOINTS(lParam);
		m_Input.OnMouseMoved(mp.x, mp.y);
		break;
	case WM_MOUSEWHEEL:
		POINTS wp = MAKEPOINTS(lParam);
		m_Input.OnWheelDelta(wp.x, wp.y);
		break;
	case WM_LBUTTONDOWN:
		m_Input.OnMouseLPressed();
		break;
	case WM_LBUTTONUP:
		m_Input.OnMouseLReleased();
		break;
	case WM_INPUT:
		if (!m_Input.m_RawDeltaEnabled)
			break;

		UINT buffSize = 0;
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &buffSize, sizeof(RAWINPUTHEADER)) < 0)
			break;
		std::string buff;
		buff.resize(buffSize);
		if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, buff.data(), &buffSize, sizeof(RAWINPUTHEADER)) != buffSize)
			break;
		auto ri = reinterpret_cast<const RAWINPUT*>(buff.data());
		if (ri->header.dwType == RIM_TYPEMOUSE &&
			(ri->data.mouse.lLastX != 0 || ri->data.mouse.lLastY != 0))
			m_Input.OnMouseDelta(ri->data.mouse.lLastX, ri->data.mouse.lLastY);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
