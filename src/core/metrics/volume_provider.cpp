#include "volume_provider.h"

// INITGUID doit précéder les includes pour matérialiser les GUID Core Audio
// (CLSID_MMDeviceEnumerator, IID_*) dans cette unité de compilation.
#define INITGUID
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

struct VolumeProvider::Com {
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioEndpointVolume *volume = nullptr;
    bool comInit = false;

    bool init()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr) || hr == S_FALSE)
            comInit = true;

        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator),
                              reinterpret_cast<void **>(&enumerator));
        if (FAILED(hr) || !enumerator)
            return false;

        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (FAILED(hr) || !device)
            return false;

        hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void **>(&volume));
        return SUCCEEDED(hr) && volume != nullptr;
    }

    void shutdown()
    {
        if (volume) { volume->Release(); volume = nullptr; }
        if (device) { device->Release(); device = nullptr; }
        if (enumerator) { enumerator->Release(); enumerator = nullptr; }
        if (comInit) { CoUninitialize(); comInit = false; }
    }
};

VolumeProvider::VolumeProvider(QObject *parent)
    : MetricProvider(parent), m_com(std::make_unique<Com>())
{
    m_available = m_com->init();
}

VolumeProvider::~VolumeProvider()
{
    m_com->shutdown();
}

void VolumeProvider::poll()
{
    if (!m_com->volume) {
        if (m_available) { m_available = false; emit availableChanged(); }
        return;
    }

    float scalar = 0.0f;
    BOOL mute = FALSE;
    m_com->volume->GetMasterVolumeLevelScalar(&scalar);
    m_com->volume->GetMute(&mute);

    const double level = scalar * 100.0;
    const bool muted = (mute != FALSE);

    if (level != m_level) { m_level = level; emit levelChanged(); }
    if (muted != m_muted) { m_muted = muted; emit mutedChanged(); }
    if (!m_available) { m_available = true; emit availableChanged(); }
}
