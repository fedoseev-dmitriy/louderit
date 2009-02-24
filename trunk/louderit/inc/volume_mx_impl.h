#pragma once

#include "volume.h"

#define	MAX_VOL_XP   65535

// Volume control Mixer edition for windows xp
class CVolumeMxImpl : public IVolume
{
public:
						CVolumeMxImpl();
	virtual				~CVolumeMxImpl();

	bool				Init(int deviceNumber, HWND hwnd );
	void				Shutdown();

	void				SetVolume(int percent );
	int					GetVolume();

	void				SetMute(bool mute );
	bool				GetMute();

	void				SetVolumeChannel(int leftChannelVol, int rightChannelVol );
	int					GetNumDevice();
	std::string			GetDevName(const int index );

	//bool				CheckIdDevice(int idDevice );

private:
	enum Channel
	{
		LEFT = 1,
		RIGHT = 2
	};

	MMRESULT						m_Res;
	int								m_channelCount;

	HMIXER							m_hmx;
	MIXERLINE						m_mxLine;		// Описатель линии
	MIXERLINECONTROLS				m_mxCtls;		// Описатель группы элементов управления 
	MIXERCONTROL					m_mxVolCtrl;	// Описатель регулятора громкости
	MIXERCONTROL					m_mxMuteCtrl;	// Описатель переключателя глушения
	MIXERCONTROLDETAILS				m_mxVolDetails;	// Описатель состояния регулятора громкости
	MIXERCONTROLDETAILS				m_mxMuteDetails;// Описатель состояния переключателя глушения
	MIXERCONTROLDETAILS_UNSIGNED	m_mxVolVal;		// Значение уровня громкости
	MIXERCONTROLDETAILS_BOOLEAN		m_mxMuteVal;	// Значение переключателя глушения
};