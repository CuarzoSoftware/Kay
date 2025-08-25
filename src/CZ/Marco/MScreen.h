#ifndef MSCREEN_H
#define MSCREEN_H

#include <CZ/AK/AKObject.h>
#include <CZ/CZSignal.h>
#include <CZ/CZTransform.h>
#include <CZ/CZBitset.h>
#include <CZ/Marco/Marco.h>
#include <CZ/Marco/MProxy.h>
#include <CZ/Marco/MApplication.h>
#include <CZ/skia/core/SkSize.h>
#include <CZ/skia/core/SkSurface.h>

class CZ::MScreen : public AKObject
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
        CZTransform transform;
        std::vector<Mode> modes;
    };

    const Props &props() const noexcept
    {
        return m_current;
    }

    struct
    {
        CZSignal<MScreen&, CZBitset<Changes>> propsChanged;
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
    CZBitset<Changes> m_changes;
    bool m_pendingFirstDone { true };
    MProxy<wl_output> m_proxy;
};

#endif // MSCREEN_H
