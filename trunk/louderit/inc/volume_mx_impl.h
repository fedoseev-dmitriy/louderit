#pragma once

#include "volume.h"

#define	MAX_VOL_XP   65535

// Volume control Mixer implementation for windows xp
class VolumeMxImpl: public IVolume
{
public:
						VolumeMxImpl();
	virtual				~VolumeMxImpl();

	bool				Init(int device_number, HWND hwnd );
	void				Shutdown();

	void				SetVolume(int percent );
	int					GetVolume() const;

	void				SetMute(bool mute );
	bool				GetMute() const;

	int					GetNumDevice() const;
	wstring				GetDeviceName(const int index ) const;

	void				SetVolumeChannel(int l_channel_volume, int r_channel_volume);
	//bool				CheckIdDevice(int idDevice );

private:
	enum Channel
	{
		kLeft = 1,
		kRight = 2
	};

	mutable MMRESULT				mm_result_;
	int								channel_count_;

	HMIXER							hmx_;
	MIXERLINE						mx_line_;		// ��������� �����
	MIXERLINECONTROLS				mx_ctls_;		// ��������� ������ ��������� ���������� 
	MIXERCONTROL					mx_vol_ctrl_;	// ��������� ���������� ���������
	MIXERCONTROL					mx_mute_ctrl_;	// ��������� ������������� ��������
	MIXERCONTROLDETAILS				mx_vol_details_;	// ��������� ��������� ���������� ���������
	MIXERCONTROLDETAILS				mx_mute_details_;// ��������� ��������� ������������� ��������
	MIXERCONTROLDETAILS_UNSIGNED	mx_vol_value_;		// �������� ������ ���������
	MIXERCONTROLDETAILS_BOOLEAN		mx_mute_status_;	// �������� ������������� ��������
};