#pragma once

class IVolume
{
public:
	virtual	bool	Init(int deviceNumber, HWND hwnd) = 0;
	virtual	void	Shutdown() = 0;

	// Set current volume in percent(0-100%)
	virtual	void	SetVolume(int percent) = 0;

	// Get current volume in percent(0-100%)
	virtual int		GetVolume() = 0;

	// Set current volume in percent(0-100%) left and right channel
	virtual	void	SetVolumeChannel(int leftChannelVol, int rightChannelVol) = 0;

	virtual void	SetMute(bool mute) = 0;
	virtual bool	GetMute() = 0;

	virtual int		GetNumDevice() = 0;
	virtual string	GetDevName(const int index) = 0;

	//virtual bool	CheckIdDevice(int idDevice) = 0;

};



