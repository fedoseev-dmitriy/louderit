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
class VolumeNotification : public IAudioEndpointVolumeCallback
{
	LONG  cRef_;

public:
	VolumeNotification() :
	  cRef_(1)
	  {
	  }

	  ~VolumeNotification()
	  {
	  }

	  ULONG STDMETHODCALLTYPE AddRef()
	  {
		  return InterlockedIncrement(&cRef_);
	  }

	  ULONG STDMETHODCALLTYPE Release()
	  {
		  ULONG ulRef = InterlockedDecrement(&cRef_);
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
class VolumeImpl: public IVolume
{
public:
						VolumeImpl();
	virtual				~VolumeImpl();

	bool				Init(int device_number, HWND hwnd);
	void				Shutdown();

	void				SetVolume(int percent);
	int					GetVolume() const;

	void				SetMute(bool mute);
	bool				GetMute() const;

	int					GetNumDevice() const;
	wstring				GetDeviceName(const int index) const;

	void				SetVolumeChannel(int l_channel_volume, int r_channel_volume);
	//bool				CheckIdDevice(int idDevice);
private:
	mutable HRESULT				hr_;

	IAudioEndpointVolume		*end_point_volume_;
	IMMDeviceEnumerator			*device_enumerator_;
	IMMDevice					*default_device_;
	IMMDeviceCollection			*collection_;

	VolumeNotification			volume_events_;
	GUID						guid_context_;

	enum Channel
	{
		kLeft = 0,
		kRight
	};
};