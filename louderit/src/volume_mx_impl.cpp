#include "precompiled.h"
#include "volume_mx_impl.h"

//-----------------------------------------------------------------------------
CVolumeMxImpl::CVolumeMxImpl() :
				m_channelCount(0)
{

}

//-----------------------------------------------------------------------------
CVolumeMxImpl::~CVolumeMxImpl()
{

}

//-----------------------------------------------------------------------------
bool CVolumeMxImpl::Init(int deviceNumber, HWND hwnd)
{
	m_hmx = NULL;

	m_Res = mixerOpen(&m_hmx, deviceNumber, DWORD(hwnd), 0, CALLBACK_WINDOW);

	if (m_Res != MMSYSERR_NOERROR)
	{
		TraceA("%s", "Невозможно открыть микшер");
		return FALSE;
	}
	do 
	{
		//
		// Запрашиваем параметры линии по типу
		//
		m_mxLine.cbStruct = sizeof(m_mxLine);

		m_mxLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;

		m_Res = mixerGetLineInfo(HMIXEROBJ(m_hmx),&m_mxLine,MIXER_GETLINEINFOF_COMPONENTTYPE);

		if (m_Res != MMSYSERR_NOERROR) 
		{
			TraceA("%s", "Невозможно получить параметры линии");
			break;
		}
		m_channelCount = m_mxLine.cChannels;
		TraceA("m_channelCount = %i", m_channelCount);
		//
		// Запрашиваем параметры регулятора громкости (тип элемента - VOLUME)
		//
		m_mxCtls.cbStruct = sizeof(m_mxCtls);
		m_mxCtls.dwLineID = m_mxLine.dwLineID;
		m_mxCtls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
		m_mxCtls.cControls = 1;
		m_mxCtls.cbmxctrl = sizeof(m_mxVolCtrl);
		m_mxCtls.pamxctrl = &m_mxVolCtrl;

		m_Res = mixerGetLineControls(HMIXEROBJ(m_hmx),&m_mxCtls,MIXER_GETLINECONTROLSF_ONEBYTYPE);

		if (m_Res != MMSYSERR_NOERROR) 
		{
			TraceA("%s", "Невозможно получить параметры регулятора громкости");
			break;
		}  
		//
		// Запрашиваем параметры переключателя глушения (тип элемента - MUTE)
		//
		m_mxCtls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
		m_mxCtls.cControls = 1;
		m_mxCtls.cbmxctrl = sizeof(m_mxMuteCtrl);
		m_mxCtls.pamxctrl = &m_mxMuteCtrl;

		m_Res = mixerGetLineControls(HMIXEROBJ(m_hmx),&m_mxCtls,MIXER_GETLINECONTROLSF_ONEBYTYPE);

		if (m_Res != MMSYSERR_NOERROR) 
		{
			TraceA("%s", "Невозможно получить параметры переключателя глушения");
			break;
		}  
		//
		// Инициализируем описатели состояния элементов
		//
		m_mxVolDetails.cbStruct = sizeof(m_mxVolDetails);
		m_mxVolDetails.dwControlID = m_mxVolCtrl.dwControlID;
		m_mxVolDetails.cChannels = 1;
		m_mxVolDetails.cMultipleItems = 0;
		m_mxVolDetails.cbDetails = sizeof(m_mxVolVal);
		m_mxVolDetails.paDetails = &m_mxVolVal;

		m_mxMuteDetails.cbStruct = sizeof(m_mxMuteDetails);
		m_mxMuteDetails.dwControlID = m_mxMuteCtrl.dwControlID;
		m_mxMuteDetails.cChannels = 1;
		m_mxMuteDetails.cMultipleItems = 0;
		m_mxMuteDetails.cbDetails = sizeof(m_mxMuteVal);
		m_mxMuteDetails.paDetails = &m_mxMuteVal;

		return true;

	} 	while (0);
	
	// при неудаче производим закрытие =(
	Shutdown();
	return false;

}

//-----------------------------------------------------------------------------
void CVolumeMxImpl::Shutdown()
{
	if (m_hmx)
	{
		mixerClose(m_hmx);
		m_hmx = 0;
	}
}

//-----------------------------------------------------------------------------
void CVolumeMxImpl::SetVolume(int percent)
{
	m_mxVolVal.dwValue = (MAX_VOL_XP / 100) * percent;
	mixerSetControlDetails(HMIXEROBJ(m_hmx), &m_mxVolDetails, MIXER_GETCONTROLDETAILSF_VALUE);
}

//-----------------------------------------------------------------------------
int CVolumeMxImpl::GetVolume()
{
	int percent;
	mixerGetControlDetails(HMIXEROBJ(m_hmx), &m_mxVolDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	percent = m_mxVolVal.dwValue / (MAX_VOL_XP / 100);
	return percent;
}

//-----------------------------------------------------------------------------
void CVolumeMxImpl::SetMute(bool mute)
{
	m_mxMuteVal.fValue = mute;
	mixerSetControlDetails(HMIXEROBJ(m_hmx), &m_mxMuteDetails, MIXER_GETCONTROLDETAILSF_VALUE);
}

//-----------------------------------------------------------------------------
bool CVolumeMxImpl::GetMute()
{
	mixerGetControlDetails(HMIXEROBJ(m_hmx), &m_mxMuteDetails, MIXER_GETCONTROLDETAILSF_VALUE);
	return m_mxMuteVal.fValue ? true : false;
}

//-----------------------------------------------------------------------------
void CVolumeMxImpl::SetVolumeChannel(int LVolume, int RVolume)
{

	MessageBox(0, "Данная функция пока не реализована для Windows XP, установите баланс 50",
		"Error!", MB_ICONERROR);
}

//-----------------------------------------------------------------------------
int CVolumeMxImpl::GetNumDevice()
{
	return mixerGetNumDevs();
}

//-----------------------------------------------------------------------------
string CVolumeMxImpl::GetDevName(const int index)
{
	MIXERCAPS Caps;

	mixerGetDevCaps(index, &Caps, sizeof(Caps));
	return (const char *)Caps.szPname;
}

//-----------------------------------------------------------------------------
//bool CVolumeMxImpl::CheckIdDevice(int idDevice)
//{
//	if (nIdDevice == mxVolCtrl.dwControlID  || nIdDevice == mxMuteCtrl.dwControlID)
//	{
//		return true;
//	}
//	return false;
//}