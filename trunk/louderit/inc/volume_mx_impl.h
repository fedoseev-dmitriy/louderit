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
	MIXERLINE						m_mxLine;		// ��������� �����
	MIXERLINECONTROLS				m_mxCtls;		// ��������� ������ ��������� ���������� 
	MIXERCONTROL					m_mxVolCtrl;	// ��������� ���������� ���������
	MIXERCONTROL					m_mxMuteCtrl;	// ��������� ������������� ��������
	MIXERCONTROLDETAILS				m_mxVolDetails;	// ��������� ��������� ���������� ���������
	MIXERCONTROLDETAILS				m_mxMuteDetails;// ��������� ��������� ������������� ��������
	MIXERCONTROLDETAILS_UNSIGNED	m_mxVolVal;		// �������� ������ ���������
	MIXERCONTROLDETAILS_BOOLEAN		m_mxMuteVal;	// �������� ������������� ��������
};