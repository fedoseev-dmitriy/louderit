#include "precompiled.h"
#include "volume_mx_impl.h"

//-----------------------------------------------------------------------------
VolumeMxImpl::VolumeMxImpl() :
				channel_count_(0),
				hmx_(NULL)
{

}

//-----------------------------------------------------------------------------
VolumeMxImpl::~VolumeMxImpl()
{

}

//-----------------------------------------------------------------------------
bool VolumeMxImpl::Init(int deviceNumber, HWND hwnd)
{
	hmx_ = NULL;

	mm_result_ = mixerOpen(&hmx_, deviceNumber, DWORD(hwnd), 0, CALLBACK_WINDOW);

	if (mm_result_ != MMSYSERR_NOERROR)
	{
		//TraceA("%s", "Невозможно открыть микшер");
		return FALSE;
	}
	do 
	{
		//
		// Запрашиваем параметры линии по типу
		//
		mx_line_.cbStruct = sizeof(mx_line_);

		mx_line_.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		mm_result_ = mixerGetLineInfo(HMIXEROBJ(hmx_),&mx_line_,MIXER_GETLINEINFOF_COMPONENTTYPE);

		if (mm_result_ != MMSYSERR_NOERROR) 
		{
			//TraceA("%s", "Невозможно получить параметры линии");
			break;
		}
		channel_count_ = mx_line_.cChannels;
		//TraceA("channel_count_ = %i", channel_count_);
		//
		// Запрашиваем параметры регулятора громкости (тип элемента - VOLUME)
		//
		mx_ctls_.cbStruct = sizeof(mx_ctls_);
		mx_ctls_.dwLineID = mx_line_.dwLineID;
		mx_ctls_.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		mx_ctls_.cControls = 1;
		mx_ctls_.cbmxctrl = sizeof(mx_vol_ctrl_);
		mx_ctls_.pamxctrl = &mx_vol_ctrl_;

		mm_result_ = mixerGetLineControls(HMIXEROBJ(hmx_),&mx_ctls_,MIXER_GETLINECONTROLSF_ONEBYTYPE);

		if (mm_result_ != MMSYSERR_NOERROR) 
		{
			//TraceA("%s", "Невозможно получить параметры регулятора громкости");
			break;
		}  
		//
		// Запрашиваем параметры переключателя глушения (тип элемента - MUTE)
		//
		mx_ctls_.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		mx_ctls_.cControls = 1;
		mx_ctls_.cbmxctrl = sizeof(mx_mute_ctrl_);
		mx_ctls_.pamxctrl = &mx_mute_ctrl_;

		mm_result_ = mixerGetLineControls(HMIXEROBJ(hmx_),&mx_ctls_,MIXER_GETLINECONTROLSF_ONEBYTYPE);

		if (mm_result_ != MMSYSERR_NOERROR) 
		{
			//TraceA("%s", "Невозможно получить параметры переключателя глушения");
			break;
		}  
		//
		// Инициализируем описатели состояния элементов
		//
		mx_vol_details_.cbStruct = sizeof(mx_vol_details_);
		mx_vol_details_.dwControlID = mx_vol_ctrl_.dwControlID;
		mx_vol_details_.cChannels = 1;
		mx_vol_details_.cMultipleItems = 0;
		mx_vol_details_.cbDetails = sizeof(mx_vol_value_);
		mx_vol_details_.paDetails = &mx_vol_value_;

		mx_mute_details_.cbStruct = sizeof(mx_mute_details_);
		mx_mute_details_.dwControlID = mx_mute_ctrl_.dwControlID;
		mx_mute_details_.cChannels = 1;
		mx_mute_details_.cMultipleItems = 0;
		mx_mute_details_.cbDetails = sizeof(mx_mute_status_);
		mx_mute_details_.paDetails = &mx_mute_status_;

		return true;

	} 	while (0);
	
	// при неудаче производим закрытие =(
	Shutdown();
	return false;

}

//-----------------------------------------------------------------------------
void VolumeMxImpl::Shutdown()
{
	if (hmx_)
	{
		mixerClose(hmx_);
		hmx_ = 0;
	}
}

//-----------------------------------------------------------------------------
void VolumeMxImpl::SetVolume(int percent)
{
	mx_vol_value_.dwValue = (MAX_VOL_XP / 100) * percent;
	mixerSetControlDetails(HMIXEROBJ(hmx_), &mx_vol_details_, MIXER_GETCONTROLDETAILSF_VALUE);
}

//-----------------------------------------------------------------------------
int VolumeMxImpl::GetVolume() const
{
	mixerGetControlDetails(HMIXEROBJ(hmx_), 
		const_cast<LPMIXERCONTROLDETAILS>(&mx_vol_details_), MIXER_GETCONTROLDETAILSF_VALUE);
	int percent = mx_vol_value_.dwValue / (MAX_VOL_XP / 100);
	return percent;
}

//-----------------------------------------------------------------------------
void VolumeMxImpl::SetMute(bool mute)
{
	mx_mute_status_.fValue = mute;
	mixerSetControlDetails(HMIXEROBJ(hmx_), &mx_mute_details_, MIXER_GETCONTROLDETAILSF_VALUE);
}

//-----------------------------------------------------------------------------
bool VolumeMxImpl::GetMute() const
{
	mixerGetControlDetails(HMIXEROBJ(hmx_), 
		const_cast<LPMIXERCONTROLDETAILS>(&mx_mute_details_), MIXER_GETCONTROLDETAILSF_VALUE);
	return mx_mute_status_.fValue ? true : false;
}

//-----------------------------------------------------------------------------
void VolumeMxImpl::SetVolumeChannel(int l_channel_volume, int r_channel_volume)
{

	MessageBox(0, L"Данная функция пока не реализована для Windows XP, установите баланс 50",
		L"Error!", MB_ICONERROR);
}

//-----------------------------------------------------------------------------
int VolumeMxImpl::GetNumDevice() const
{
	return mixerGetNumDevs();
}

//-----------------------------------------------------------------------------
wstring VolumeMxImpl::GetDeviceName(const int index) const
{
	MIXERCAPS Caps;

	mixerGetDevCaps(index, &Caps, sizeof(Caps));
	return (const wchar_t *)Caps.szPname;
}

//-----------------------------------------------------------------------------
//bool VolumeMxImpl::CheckIdDevice(int idDevice)
//{
//	if (nIdDevice == mxVolCtrl.dwControlID  || nIdDevice == mxMuteCtrl.dwControlID)
//	{
//		return true;
//	}
//	return false;
//}