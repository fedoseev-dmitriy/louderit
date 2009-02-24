#pragma once

#include "volume.h"

#define	MAX_VOL   100

//-----------------------------------------------------------
// Client implementation of IAudioEndpointVolumeCallback
// interface. When a method in the IAudioEndpointVolume
// interface changes the volume level or muting state of the
// endpoint device, the change initiates a call to the
// client's IAudioEndpointVolumeCallback::OnNotify method.
//-----------------------------------------------------------
class CVolumeNotification : public IAudioEndpointVolumeCallback
{
	LONG  _cRef;

public:
	CVolumeNotification() :
	  _cRef(1)
	  {
	  }

	  ~CVolumeNotification()
	  {
	  }

	  // IUnknown methods -- AddRef, Release, and QueryInterface

	  ULONG STDMETHODCALLTYPE AddRef()
	  {
		  return InterlockedIncrement(&_cRef);
	  }

	  ULONG STDMETHODCALLTYPE Release()
	  {
		  ULONG ulRef = InterlockedDecrement(&_cRef);
		  if (0 == ulRef)
		  {
			  delete this;
		  }
		  return ulRef;

	  }

	  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
	  {
		  if (IID_IUnknown == riid)
		  {
			  AddRef();
			  *ppvInterface = (IUnknown*)this;
		  }
		  else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
		  {
			  AddRef();
			  *ppvInterface = (IAudioEndpointVolumeCallback*)this;
		  }
		  else
		  {
			  *ppvInterface = NULL;
			  return E_NOINTERFACE;
		  }
		  return S_OK;
	  }

	  // Callback method for endpoint-volume-change notifications.

	  HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA  pNotify)
	  {
		  if (pNotify == NULL)
		  {
			  return E_INVALIDARG;
		  }
		  PostMessage(HWND_BROADCAST, MM_MIXM_CONTROL_CHANGE, 0, 0);
		  return S_OK;
	  }
};
//////////////////////////////////////////////////////////////////////////
class CVolumeImpl : public IVolume
{
public:
						CVolumeImpl();
	virtual				~CVolumeImpl();

	bool				Init(int deviceNumber, HWND hwnd);
	void				Shutdown();

	void				SetVolume(int percent);

	int					GetVolume();

	void				SetMute(bool mute);
	bool				GetMute();

	void				SetVolumeChannel(int leftChannelVol, int rightChannelVol);
	int					GetNumDevice();
	std::string			GetDevName(const int index);

	//bool				CheckIdDevice(int idDevice);
private:


	HRESULT						m_hr;

	IAudioEndpointVolume		*m_pEndpointVolume;
	IMMDeviceEnumerator			*m_pDeviceEnumerator;
	IMMDevice					*m_pDefaultDevice;
	IMMDeviceCollection			*m_pCollection;

	CVolumeNotification			m_EPVolEvents;
	GUID						m_guidContext;

	enum Channel
	{
		LEFT = 0,
		RIGHT
	};
};

template <typename _anystr> std::string WideToAnsi(const _anystr src, DWORD codepage=CP_ACP)
{
	std::wstring s = src;
	std::string res;
	PCHAR buf;
	size_t l, j;

	l = s.length();
	if (!l) return "";

	j = (size_t)WideCharToMultiByte(codepage, 0, &s[0], (int)l, NULL, 0, NULL, NULL)+1;
	buf = new CHAR[j];
	WideCharToMultiByte(codepage, 0, &s[0], (int)l, buf, (int)j, NULL, NULL);	
	buf[j-1] = '\0';
	res = buf;
	delete[] buf;

	return res;
}