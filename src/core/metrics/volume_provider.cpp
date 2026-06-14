#include "volume_provider.h"

#include <QTimer>
#include <QVariantMap>

// INITGUID doit précéder les includes pour matérialiser les GUID Core Audio
// (CLSID_MMDeviceEnumerator, IID_*, PKEY_Device_FriendlyName) dans cette unité.
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
#include <audioclient.h>                      // WASAPI : capture du micro (IAudioClient)
#include <functiondiscoverykeys_devpkey.h>   // PKEY_Device_FriendlyName
#include <propvarutil.h>                      // PROPVARIANT helpers
#include <cmath>

// Les en-têtes MinGW ne font que forward-déclarer IAudioMeterInformation (jamais
// sa définition). On redéclare l'interface minimale nécessaire + son IID officiel
// (passé tel quel à Activate ; pas de __uuidof, capricieux côté MinGW).
struct IAudioMeterInformationMin : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetPeakValue(float *pfPeak) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMeteringChannelCount(UINT *pnChannelCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetChannelsPeakValues(UINT32 u32ChannelCount, float *afPeakValues) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryHardwareSupport(DWORD *pdwHardwareSupportMask) = 0;
};
static const GUID IID_IAudioMeterInformationMin =
    { 0xC02216F6, 0x8C67, 0x4B5B, { 0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64 } };

struct VolumeProvider::Com {
    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioEndpointVolume *volume = nullptr;
    IAudioMeterInformationMin *meter = nullptr;    // VU-mètre (niveau de sortie réel)
    IMMDevice *micDevice = nullptr;                // endpoint capture (micro)
    IAudioClient *micClient = nullptr;             // flux de capture du micro
    IAudioCaptureClient *micCapture = nullptr;     // lecture des frames → pic (→ sphère)
    int micChannels = 0;
    bool micFloat = false;                         // format mix : 32 bits float (vs PCM16)
    QString micName;                               // nom du micro capté (diag)
    QString friendlyName;                      // nom du périphérique de sortie
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

        readFriendlyName();

        // Meter : optionnel (on ne fait pas échouer init() s'il manque).
        device->Activate(IID_IAudioMeterInformationMin, CLSCTX_ALL, nullptr,
                         reinterpret_cast<void **>(&meter));

        // Le micro (flux de capture WASAPI) est ouvert plus tard via useMicDevice()
        // depuis QML (applyConfig), avec l'éventuel micro choisi dans les Réglages.

        hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void **>(&volume));
        return SUCCEEDED(hr) && volume != nullptr;
    }

    void readFriendlyName()
    {
        if (!device)
            return;
        IPropertyStore *props = nullptr;
        if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &props)) && props) {
            PROPVARIANT pv;
            PropVariantInit(&pv);
            if (SUCCEEDED(props->GetValue(PKEY_Device_FriendlyName, &pv))
                && pv.vt == VT_LPWSTR && pv.pwszVal)
                friendlyName = QString::fromWCharArray(pv.pwszVal);
            PropVariantClear(&pv);
            props->Release();
        }
    }

    // Ouvre un flux de capture partagé sur le micro par défaut (silencieux : on ne
    // fait que lire le pic). Format = mix du moteur (généralement 32 bits float).
    void initMicCapture()
    {
        // Nom du micro (diag) — pour confirmer quel périphérique Windows nous donne.
        IPropertyStore *p = nullptr;
        if (SUCCEEDED(micDevice->OpenPropertyStore(STGM_READ, &p)) && p) {
            PROPVARIANT pv; PropVariantInit(&pv);
            if (SUCCEEDED(p->GetValue(PKEY_Device_FriendlyName, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal)
                micName = QString::fromWCharArray(pv.pwszVal);
            PropVariantClear(&pv); p->Release();
        }
        if (FAILED(micDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                       reinterpret_cast<void **>(&micClient))) || !micClient)
            return;
        WAVEFORMATEX *fmt = nullptr;
        if (FAILED(micClient->GetMixFormat(&fmt)) || !fmt)
            return;
        micChannels = fmt->nChannels;
        micFloat = (fmt->wBitsPerSample == 32);   // mix partagé = float 32 bits en pratique
        const REFERENCE_TIME dur = 2000000;        // tampon ~200 ms
        if (SUCCEEDED(micClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, dur, 0, fmt, nullptr))
            && SUCCEEDED(micClient->GetService(__uuidof(IAudioCaptureClient),
                                               reinterpret_cast<void **>(&micCapture)))
            && micCapture)
            micClient->Start();
        CoTaskMemFree(fmt);
    }

    // Ferme proprement le flux de capture micro (sans toucher au reste du COM).
    void stopMic()
    {
        if (micClient) micClient->Stop();
        if (micCapture) { micCapture->Release(); micCapture = nullptr; }
        if (micClient) { micClient->Release(); micClient = nullptr; }
        if (micDevice) { micDevice->Release(); micDevice = nullptr; }
        micName.clear();
        micChannels = 0;
    }

    void shutdown()
    {
        stopMic();
        if (meter) { meter->Release(); meter = nullptr; }
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
    m_deviceName = m_com->friendlyName;
    // Le micro est ouvert par useMicDevice() (appelé depuis QML au démarrage).

    // VU-mètre : timer interne ~16 Hz (la barre suit le son en temps réel,
    // indépendamment de l'intervalle de polling de 2 s). Coût CPU négligeable.
    auto *vu = new QTimer(this);
    vu->setInterval(60);
    connect(vu, &QTimer::timeout, this, &VolumeProvider::samplePeak);
    vu->start();
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

void VolumeProvider::samplePeak()
{
    // Niveau de SORTIE (VU-mètre affiché).
    if (m_com->meter) {
        float peak = 0.0f;
        if (SUCCEEDED(m_com->meter->GetPeakValue(&peak))) {
            const double p = m_muted ? 0.0 : static_cast<double>(peak);
            if (p != m_peak) { m_peak = p; emit peakLevelChanged(); }
        }
    }
    // Niveau du MICRO : on draine le flux de capture et on prend le pic des frames
    // accumulées depuis le dernier tick (silence → pic 0).
    if (m_com->micCapture) {
        UINT32 packet = 0;
        float maxPeak = 0.0f;
        while (SUCCEEDED(m_com->micCapture->GetNextPacketSize(&packet)) && packet > 0) {
            BYTE *data = nullptr; UINT32 frames = 0; DWORD flags = 0;
            if (FAILED(m_com->micCapture->GetBuffer(&data, &frames, &flags, nullptr, nullptr)))
                break;
            if (data && frames > 0 && !(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                const int ch = m_com->micChannels > 0 ? m_com->micChannels : 1;
                const UINT32 n = frames * static_cast<UINT32>(ch);
                if (m_com->micFloat) {
                    const float *s = reinterpret_cast<const float *>(data);
                    for (UINT32 i = 0; i < n; ++i) { float a = std::fabs(s[i]); if (a > maxPeak) maxPeak = a; }
                } else {
                    const short *s = reinterpret_cast<const short *>(data);
                    for (UINT32 i = 0; i < n; ++i) { float a = std::fabs(s[i] / 32768.0f); if (a > maxPeak) maxPeak = a; }
                }
            }
            m_com->micCapture->ReleaseBuffer(frames);
        }
        const double m = static_cast<double>(maxPeak);
        if (m != m_mic) { m_mic = m; emit micLevelChanged(); }
    }
}

QVariantList VolumeProvider::inputDevices() const
{
    QVariantList list;
    if (!m_com->enumerator)
        return list;
    IMMDeviceCollection *coll = nullptr;
    if (FAILED(m_com->enumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &coll)) || !coll)
        return list;
    UINT count = 0;
    coll->GetCount(&count);
    for (UINT i = 0; i < count; ++i) {
        IMMDevice *d = nullptr;
        if (FAILED(coll->Item(i, &d)) || !d)
            continue;
        QString id, name;
        LPWSTR wid = nullptr;
        if (SUCCEEDED(d->GetId(&wid)) && wid) { id = QString::fromWCharArray(wid); CoTaskMemFree(wid); }
        IPropertyStore *p = nullptr;
        if (SUCCEEDED(d->OpenPropertyStore(STGM_READ, &p)) && p) {
            PROPVARIANT pv; PropVariantInit(&pv);
            if (SUCCEEDED(p->GetValue(PKEY_Device_FriendlyName, &pv)) && pv.vt == VT_LPWSTR && pv.pwszVal)
                name = QString::fromWCharArray(pv.pwszVal);
            PropVariantClear(&pv); p->Release();
        }
        QVariantMap m;
        m.insert(QStringLiteral("id"), id);
        m.insert(QStringLiteral("name"), name);
        list.append(m);
        d->Release();
    }
    coll->Release();
    return list;
}

void VolumeProvider::useMicDevice(const QString &id)
{
    if (!m_com->enumerator)
        return;
    m_com->stopMic();
    m_micId = id;

    IMMDevice *dev = nullptr;
    if (!id.isEmpty())
        m_com->enumerator->GetDevice(reinterpret_cast<LPCWSTR>(id.utf16()), &dev);
    if (!dev)   // id vide ou introuvable → micro par défaut Windows
        m_com->enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &dev);

    m_com->micDevice = dev;
    if (dev)
        m_com->initMicCapture();

    m_mic = 0.0;
    emit micLevelChanged();
    emit micDeviceChanged();
}
