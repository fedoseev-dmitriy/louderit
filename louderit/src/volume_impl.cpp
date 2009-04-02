#include "precompiled.h"
#include "volume_impl.h"
#include "platform.h"

//-----------------------------------------------------------------------------
VolumeImpl::VolumeImpl() :
			end_point_volume_(NULL),
			device_enumerator_(NULL),
			default_device_(NULL),
			collection_(NULL)
{
}

//-----------------------------------------------------------------------------
VolumeImpl::~VolumeImpl()
{
}

//-----------------------------------------------------------------------------
bool VolumeImpl::Init(int device_number, HWND hwnd)
{
	CoInitialize(NULL);

	hr_ = CoCreateGuid(&guid_context_);
	EXIT_ON_ERROR(hr_);
	
	hr_ = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&device_enumerator_);
	EXIT_ON_ERROR(hr_);
	
	hr_ = device_enumerator_->GetDefaultAudioEndpoint(eRender, eConsole, &default_device_);
	EXIT_ON_ERROR(hr_);

	//hr_ = collection_->Item(deviceNumber, &default_device_);
	//EXIT_ON_ERROR(hr_);
	
	hr_ = default_device_->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&end_point_volume_);
	EXIT_ON_ERROR(hr_);

	SAFE_RELEASE(default_device_);

	hr_ = end_point_volume_->RegisterControlChangeNotify(
		(IAudioEndpointVolumeCallback*)&volume_events_);

	return true;
}

//-----------------------------------------------------------------------------
void VolumeImpl::Shutdown()
{
	SAFE_RELEASE(device_enumerator_);

	end_point_volume_->UnregisterControlChangeNotify(
		(IAudioEndpointVolumeCallback*)&volume_events_);

	end_point_volume_->Release();

	CoUninitialize();
}

//-----------------------------------------------------------------------------
void VolumeImpl::SetVolume(int percent)
{
	float volume = (float)percent / MAX_VOL;

	hr_ = end_point_volume_->SetMasterVolumeLevelScalar(volume, &guid_context_);
	EXIT_ON_ERROR(hr_);
}
//-----------------------------------------------------------------------------
void VolumeImpl::SetMute(bool mute)
{
	end_point_volume_->SetMute(mute, &guid_context_);
}

//-----------------------------------------------------------------------------
int VolumeImpl::GetVolume() const
{
	float	volume;

	hr_ = end_point_volume_->GetMasterVolumeLevelScalar(&volume);
	EXIT_ON_ERROR(hr_);

	int percent = (int)(MAX_VOL * volume + 0.5f);
	return 	percent;
}

//-----------------------------------------------------------------------------
bool VolumeImpl::GetMute() const
{
	BOOL mute;

	hr_ = end_point_volume_->GetMute(&mute);
	EXIT_ON_ERROR(hr_);

	return mute != 0 ? true : false;
}

//-----------------------------------------------------------------------------
void VolumeImpl::SetVolumeChannel(int l_channel_volume, int r_channel_volume)
{
	float volume = (float)l_channel_volume / MAX_VOL;
	hr_ = end_point_volume_->SetChannelVolumeLevelScalar(LEFT, volume, &guid_context_);
	EXIT_ON_ERROR(hr_);

	volume = (float)r_channel_volume / MAX_VOL;
	hr_ = end_point_volume_->SetChannelVolumeLevelScalar(RIGHT, volume, &guid_context_);
	EXIT_ON_ERROR(hr_);
}

//-----------------------------------------------------------------------------
int VolumeImpl::GetNumDevice() const
{
	CoInitialize(NULL);

	hr_ = CoCreateGuid(const_cast<GUID *>(&guid_context_));
	EXIT_ON_ERROR(hr_);

	hr_ = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&device_enumerator_);
	EXIT_ON_ERROR(hr_);

	hr_ = device_enumerator_->EnumAudioEndpoints(
		eAll, DEVICE_STATE_ACTIVE,
		const_cast<IMMDeviceCollection **>(&collection_));
	EXIT_ON_ERROR(hr_)

	UINT  count;
	hr_ = collection_->GetCount(&count);
	EXIT_ON_ERROR(hr_)

	return count;
}

//-----------------------------------------------------------------------------
wstring VolumeImpl::GetDeviceName(const int index) const
{
	IMMDevice		*pEndpoint = NULL;
	IPropertyStore	*pProps = NULL;

	// Get pointer to endpoint number i.
	hr_ = collection_->Item(index, &pEndpoint);
	EXIT_ON_ERROR(hr_)

	hr_ = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
	EXIT_ON_ERROR(hr_)

	PROPVARIANT varName;
	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the endpoint's friendly-name property.
	hr_ = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	EXIT_ON_ERROR(hr_)

	//s = WideToAnsi(varName.pwszVal);
	wstring s = varName.pwszVal;

	PropVariantClear(&varName);
	SAFE_RELEASE(pProps)
	SAFE_RELEASE(pEndpoint)
	return s;
}

