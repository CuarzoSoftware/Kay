#ifndef MSCREEN_H
#define MSCREEN_H

#include <AK/AKObject.h>
#include <AK/AKSignal.h>
#include <AK/AKTransform.h>
#include <AK/AKBitset.h>
#include <Marco/Marco.h>
#include <Marco/MProxy.h>
#include <Marco/MApplication.h>
#include <skia/core/SkSize.h>
#include <skia/core/SkSurface.h>

class AK::MScreen : public AKObject
{
public:
    enum Changes
    {
        Position        = 1 << 0,
        PhysicalSize    = 1 << 1,
        PixelGeometry   = 1 << 2,
        Scale           = 1 << 3,
        Name            = 1 << 4,
        Description     = 1 << 5,
        Make            = 1 << 6,
        Model           = 1 << 7,
        Transform       = 1 << 8,
        Modes           = 1 << 9
    };

    struct Mode
    {
        SkISize size;
        Int32 refresh;
        bool current;
        bool preferred;
    };

    struct Props
    {
        std::string name;
        std::string description;
        std::string make;
        std::string model;
        SkIPoint pos;
        SkISize physicalSize;
        Int32 scale { 1 };
        SkPixelGeometry pixelGeometry;
        AKTransform transform;
        std::vector<Mode> modes;
    };

    const Props &props() const noexcept
    {
        return m_current;
    }

    struct
    {
        AKSignal<MScreen&, AKBitset<Changes>> propsChanged;
    } on;

    constexpr static SkPixelGeometry wl2SkPixelGeometry(Int32 wlPixelGeometry) noexcept
    {
        switch (wlPixelGeometry)
        {
        case WL_OUTPUT_SUBPIXEL_UNKNOWN:
        case WL_OUTPUT_SUBPIXEL_NONE:
            return kUnknown_SkPixelGeometry;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
            return kRGB_H_SkPixelGeometry;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
            return kBGR_H_SkPixelGeometry;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
            return kRGB_V_SkPixelGeometry;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
            return kBGR_V_SkPixelGeometry;
        }
        return kUnknown_SkPixelGeometry;
    }

    wl_output *wlOutput() const noexcept
    {
        return m_proxy.get();
    }

private:
    friend class MApplication;
    MScreen(void *proxy, UInt32 name) noexcept : m_proxy(proxy, name) {}

    ~MScreen()
    {
        wl_output_destroy(m_proxy);
    }

    Props m_current, m_pending;
    AKBitset<Changes> m_changes;
    bool m_pendingFirstDone { true };
    MProxy<wl_output> m_proxy;
};

#endif // MSCREEN_H
