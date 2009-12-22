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
	MIXERLINE						mx_line_;		// Описатель линии
	MIXERLINECONTROLS				mx_ctls_;		// Описатель группы элементов управления 
	MIXERCONTROL					mx_vol_ctrl_;	// Описатель регулятора громкости
	MIXERCONTROL					mx_mute_ctrl_;	// Описатель переключателя глушения
	MIXERCONTROLDETAILS				mx_vol_details_;	// Описатель состояния регулятора громкости
	MIXERCONTROLDETAILS				mx_mute_details_;// Описатель состояния переключателя глушения
	MIXERCONTROLDETAILS_UNSIGNED	mx_vol_value_;		// Значение уровня громкости
	MIXERCONTROLDETAILS_BOOLEAN		mx_mute_status_;	// Значение переключателя глушения
};