#pragma once
#include "resource.h"
#include "volume_impl.h"
#include "volume_mx_impl.h"
#include "lexical_cast.h"
#include "tray.h"


extern CAppModule	_Module;

class CMainFrame :	public CFrameWindowImpl<CMainFrame>, 
					public CUpdateUI<CMainFrame>,
					public CMessageFilter,
					public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	enum HotkeysIds
	{
		HK_UPKEY = 0,
		HK_DOWNKEY,
		HK_MUTEKEY
	};
	enum MenuIds
	{
		IDM_OPEN_VOLUME_CONTROL = 40001,
		IDM_SETTING_AUDIO_PARAMETR,
		IDM_SETTING_PROGRAM,	
		IDM_HELP,
		IDM_ABOUT,
		IDM_EXIT
	};

	CCommandBarCtrl	command_bar;
	IVolume*		volume_;
	TrayIcon*		trayicon_; // нет инициализции и т.д.

	HHOOK			hook_;
	bool			scroll_with_tray_; // Вкл./выкл. управление скроллом над треем
	bool			scroll_with_ctrl_;
	bool			scroll_with_alt_;
	bool			scroll_with_shift_;
	CPoint			last_tray_pos_;		// Координаты мыши над треем

	UINT			WM_LOADCONFIG;

	//------------------------------------------------------------------------------

	bool			is_winxp_;
	int				device_number_;

	vector<CIcon>	icons_;
	int				icon_index_;
	int				volume_level_;
	int 			num_icons_;

	bool					is_double_click_;

	int						hotkey_;
	int						keymod_;

	//------------------------------------------------------------------------------
	// Settings
	//------------------------------------------------------------------------------

	bool						balloon_hint_;
	int							steps_;
	int							tray_commands_[2];
	int							balance_;

	wchar_t						config_file_[MAX_PATH];
	
	wchar_t						skin_[256];

	CMainFrame() : volume_(NULL),
		device_number_(0),
		hook_(0),
		scroll_with_tray_(false),
		scroll_with_ctrl_(false),
		scroll_with_alt_(false),
		scroll_with_shift_(false),
		is_winxp_(false),
		icon_index_(0),
		volume_level_(0),
		num_icons_(0),
		is_double_click_(false),
		hotkey_(0),
		keymod_(0),
		balloon_hint_(false),
		steps_(0),
		balance_(0)
	{
	}

	virtual ~CMainFrame() 
	{
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_MSG_MAP(CMainFrame)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_HOTKEY(OnHotKey)

		MESSAGE_HANDLER(MM_MIXM_CONTROL_CHANGE, OnMixerCtrlChange)
		MESSAGE_HANDLER(WM_LOADCONFIG, OnLoadConfig)
		//MESSAGE_HANDLER_EX(WM_NOTIFYICONTRAY, OnWmNotifyicontray)

		COMMAND_ID_HANDLER_EX(IDM_OPEN_VOLUME_CONTROL, OnOpenVolumeControl)
		COMMAND_ID_HANDLER_EX(IDM_SETTING_AUDIO_PARAMETR, OnSettingAudioParametr)
		COMMAND_ID_HANDLER_EX(IDM_SETTING_PROGRAM, OnSettingProgram)
		//COMMAND_ID_HANDLER_EX(IDM_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER_EX(IDM_EXIT, OnExit)

		//COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		//UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		//create command bar window
		HWND hwnd_command_bar = command_bar.Create(m_hWnd, rcDefault, NULL, 
			ATL_SIMPLE_CMDBAR_PANE_STYLE);

		//attach menu
		command_bar.AttachMenu(GetMenu());

		//load command bar images
		command_bar.LoadImages(IDR_MAINFRAME);

		//remove old menu
		SetMenu(NULL);

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(hwnd_command_bar);


		// this code
		is_winxp_ = CheckXP();

		WM_LOADCONFIG = RegisterWindowMessage(L"WM_LOADCONFIG");

		if (is_winxp_)
		{
			volume_ = new VolumeMxImpl;
		}
		else // Vista or Win7
		{
			volume_ = new VolumeImpl;
		}

		SetHook(true); 
		LoadConfig();

		volume_->Init(device_number_, m_hWnd);

		LoadIcons();

		CMessageLoop* pLoop = _Module.GetMessageLoop();
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		CMenu	menu;
		menu.CreatePopupMenu();
		menu.AppendMenu(MF_STRING, IDM_OPEN_VOLUME_CONTROL, L"Открыть &регулятор громкости");
		menu.AppendMenu(MF_STRING, IDM_SETTING_AUDIO_PARAMETR, L"Настройка &аудиопараметров");
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, IDM_SETTING_PROGRAM, L"&Настройки программы");
		menu.AppendMenu(MF_SEPARATOR);
		//menu.AppendMenu(MF_STRING, IDM_ABOUT, L"&О программе...");
		menu.AppendMenu(MF_STRING, IDM_EXIT, L"&Выход"); 

		
		// Install the tray icon
		UpdateTrayIcon();

		return 0;
	}

	void OnClose() 
	{
		// Подготовка к закрытию программы
		volume_->Shutdown();
		UnregHotKeys();
		SetHook(false);
		delete volume_;

		SetMsgHandled(FALSE);
		PostQuitMessage(0);
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		if (nIDEvent == 1)
		{
			KillTimer(1);
			if (!is_double_click_)
			{
				TrayCommand(tray_commands_[0]);
			}
			else
			{
				is_double_click_ = false;
			}
		}
	}

	void OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey)
	{
		switch (nHotKeyID)
		{
		case HK_UPKEY:
			VolumeUp();
			break;
		case HK_DOWNKEY:
			VolumeDown();
			break;
		case HK_MUTEKEY:
			volume_->SetMute(!volume_->GetMute());
			break;
		}
		if (balloon_hint_)
		{
			wstring str_on = lexical_cast<wstring>(volume_->GetVolume()) + L"%";
			if (!volume_->GetMute())
			{
				//trayicon_->ShowBaloon((lexical_cast<wstring>(volume_->GetVolume()) + L"%").c_str(), L"Громкость:");
			}
			else
			{
				//trayicon_->ShowBaloon((lexical_cast<wstring>(volume_->GetVolume()) + L"% (Выкл.)").c_str(), L"Громкость:");
			}
		}
	}

	void OnOpenVolumeControl(UINT, int, HWND)
	{
		if (is_winxp_)
		{
			Launch(L"sndvol32.exe", GetMixerCmdLine().c_str());
		}
		else
		{
			Launch(L"sndvol.exe");
		}
	}
	void OnSettingAudioParametr(UINT, int, HWND)
	{
		Launch(L"rundll32.exe", L"shell32.dll,Control_RunDLL mmsys.cpl");
	}
	void OnSettingProgram(UINT, int, HWND)
	{
		Launch(L"LConfig.exe");
	}
	void OnExit(UINT, int, HWND)
	{
		OnClose();
	}
	//void OnAppAbout(UINT, int, HWND)
	//{
	//Launch(L"LConfig.exe -a");
	//}

	//LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//{
	//	//CAboutDlg dlg;
	//	//dlg.DoModal();
	//	return 0;
	//}
	LRESULT OnMixerCtrlChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		UpdateTrayIcon();
		return 0;
	}
	LRESULT OnLoadConfig(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LoadConfig();
		LoadIcons();
		//trayicon_->Update(icons_[icon_index_], L"LouderIt");
		//trayicon_.SetIcon(icons_.at(icon_index_));
		return 0;
	}

	bool Launch(const wchar_t* command_line, const wchar_t* parameters = NULL);
	bool CheckXP();
	void UnregHotKeys();
	bool SetHotKey(const wchar_t* skey, const wchar_t* smod, int numkey);
	bool GetAppPath(wchar_t* app_path);
	void LoadConfig();
	void LoadIcons();
	void UpdateTrayIcon();
	wstring GetMixerCmdLine();
	void TrayCommand(int flag);
	void VolumeUp();
	void VolumeDown();
	bool GetKeys();
	LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
	void SetHook(bool flag);

	
	LRESULT OnWmNotifyicontray(UINT uMsg, WPARAM wParam, LPARAM lParam);
};