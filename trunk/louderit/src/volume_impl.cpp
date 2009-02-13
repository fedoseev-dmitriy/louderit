#include "precompiled.h"
#include "volume_impl.h"
#include "platform.h"

//-----------------------------------------------------------------------------
CVolumeImpl::CVolumeImpl() :
			m_pEndpointVolume( NULL ),
			m_pDeviceEnumerator( NULL ),
			m_pDefaultDevice( NULL ),
			m_pCollection( NULL )
{
}

//-----------------------------------------------------------------------------
CVolumeImpl::~CVolumeImpl()
{
}

//-----------------------------------------------------------------------------
bool CVolumeImpl::Init( int deviceNumber, HWND hwnd )
{
	CoInitialize( NULL );

	m_hr = CoCreateGuid( &m_guidContext );
	EXIT_ON_ERROR(m_hr);
	
	m_hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&m_pDeviceEnumerator);
	EXIT_ON_ERROR(m_hr);
	
	//hr = m_pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDefaultDevice);
	//EXIT_ON_ERROR(hr);

	m_hr = m_pCollection->Item(deviceNumber, &m_pDefaultDevice);
	EXIT_ON_ERROR(m_hr);
	
	m_hr = m_pDefaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&m_pEndpointVolume);
	EXIT_ON_ERROR(m_hr);

	SAFE_RELEASE(m_pDefaultDevice);

	m_hr = m_pEndpointVolume->RegisterControlChangeNotify(
		(IAudioEndpointVolumeCallback*)&m_EPVolEvents);

	return true;
}

//-----------------------------------------------------------------------------
void CVolumeImpl::Shutdown()
{
	SAFE_RELEASE(m_pDeviceEnumerator);

	m_pEndpointVolume->UnregisterControlChangeNotify(
		(IAudioEndpointVolumeCallback*)&m_EPVolEvents);

	m_pEndpointVolume->Release();

	CoUninitialize();
}

//-----------------------------------------------------------------------------
void CVolumeImpl::SetVolume( int percent )
{
	float  volume;
	volume = (float)percent / MAX_VOL;

	m_hr = m_pEndpointVolume->SetMasterVolumeLevelScalar( volume, &m_guidContext );
	EXIT_ON_ERROR(m_hr);
}
//-----------------------------------------------------------------------------
void CVolumeImpl::SetMute( bool mute )
{
	m_pEndpointVolume->SetMute( mute, &m_guidContext);
}

//-----------------------------------------------------------------------------
int CVolumeImpl::GetVolume()
{
	float	volume;
	int		percent;

	m_hr = m_pEndpointVolume->GetMasterVolumeLevelScalar( &volume );
	EXIT_ON_ERROR(m_hr);
	percent = (int)( MAX_VOL * volume + 0.5f );

	return 	percent;
}

//-----------------------------------------------------------------------------
bool CVolumeImpl::GetMute()
{
	BOOL mute;

	m_hr = m_pEndpointVolume->GetMute( &mute );
	EXIT_ON_ERROR(m_hr);

	return mute != 0 ? true : false;
}

//-----------------------------------------------------------------------------
void CVolumeImpl::SetVolumeChannel( int leftChannelVol, int rightChannelVol )
{
	float  volume;
	volume = (float)leftChannelVol / MAX_VOL;
	m_hr = m_pEndpointVolume->SetChannelVolumeLevelScalar( LEFT, volume, &m_guidContext);
	EXIT_ON_ERROR(m_hr);

	volume = (float)rightChannelVol / MAX_VOL;
	m_hr = m_pEndpointVolume->SetChannelVolumeLevelScalar( RIGHT, volume, &m_guidContext);
	EXIT_ON_ERROR(m_hr);
}

//-----------------------------------------------------------------------------
int CVolumeImpl::GetNumDevice()
{
	CoInitialize( NULL );

	m_hr = CoCreateGuid( &m_guidContext );
	EXIT_ON_ERROR(m_hr);

	m_hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&m_pDeviceEnumerator);
	EXIT_ON_ERROR(m_hr);

	m_hr = m_pDeviceEnumerator->EnumAudioEndpoints(
		eAll, DEVICE_STATE_ACTIVE,
		&m_pCollection);
	EXIT_ON_ERROR(m_hr)

	UINT  count;
	m_hr = m_pCollection->GetCount(&count);
	EXIT_ON_ERROR(m_hr)

	return count;
}

//-----------------------------------------------------------------------------
std::string CVolumeImpl::GetDevName( const int index )
{
	std::string		s;
	IMMDevice		*pEndpoint = NULL;
	IPropertyStore	*pProps = NULL;

	// Get pointer to endpoint number i.
	m_hr = m_pCollection->Item( index, &pEndpoint );
	EXIT_ON_ERROR(m_hr)

	m_hr = pEndpoint->OpenPropertyStore( STGM_READ, &pProps );
	EXIT_ON_ERROR(m_hr)

	PROPVARIANT varName;
	// Initialize container for property value.
	PropVariantInit( &varName );

	// Get the endpoint's friendly-name property.
	m_hr = pProps->GetValue( PKEY_Device_FriendlyName, &varName );
	EXIT_ON_ERROR(m_hr)

	s = WideToAnsi( varName.pwszVal );

	PropVariantClear( &varName );
	SAFE_RELEASE( pProps )
	SAFE_RELEASE( pEndpoint )
	return s;
}

