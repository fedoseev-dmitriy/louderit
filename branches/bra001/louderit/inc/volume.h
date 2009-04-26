#pragma once

class IVolume
{
public:
	virtual	bool	Init(int device_number, HWND hwnd) = 0;
	virtual	void	Shutdown() = 0;

	virtual	void	SetVolume(int percent) = 0;
	virtual int		GetVolume() const = 0;

	virtual void	SetMute(bool mute) = 0;
	virtual bool	GetMute() const = 0;

	virtual int		GetNumDevice() const = 0;
	virtual wstring	GetDeviceName(const int index) const = 0;

	virtual void	SetVolumeChannel(int l_channel_volume, int r_channel_volume) = 0;
	//virtual bool	CheckIdDevice(int idDevice) = 0;

};



