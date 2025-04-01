#ifndef MLAYERSHELL_H
#define MLAYERSHELL_H

#include <Marco/roles/MSurface.h>
#include <AK/AKEdge.h>

class AK::MLayerSurface : public MSurface
{
public:
    enum Layer
    {
        Background,
        Bottom,
        Top,
        Overlay
    };

    enum KeyboardInteractivity
    {
        None,
        Exclusive,
        OnDemand
    };

    MLayerSurface(Layer layer, AKBitset<AKEdge> anchor, Int32 exclusiveZone = -1, MScreen *screen = nullptr, const std::string &scope = "") noexcept;

    AKCLASS_NO_COPY(MLayerSurface)

    /**
     * @brief Destructor for `MLayerSurface`.
     */
    ~MLayerSurface() noexcept;

    bool setScreen(MScreen *screen) noexcept;
    MScreen *screen() const noexcept;

    bool setAnchor(AKBitset<AKEdge> edges) noexcept;
    AKBitset<AKEdge> anchor() const noexcept;

    bool setExclusiveZone(Int32 size) noexcept;
    Int32 exclusiveZone() const noexcept;

    bool setMargin(const SkIRect &margin) noexcept;
    const SkIRect &margin() const noexcept;

    bool setKeyboardInteractivity(KeyboardInteractivity mode) noexcept;
    KeyboardInteractivity keyboardInteractivity() const noexcept;

    bool setLayer(Layer layer) noexcept;
    Layer layer() const noexcept;

    bool setExclusiveEdge(AKEdge edge) noexcept;
    AKEdge exclusiveEdge() const noexcept;

    bool setScope(const std::string &scope) noexcept;
    const std::string &scope() const noexcept;

    const SkISize &suggestedSize() const noexcept;

    class Imp;
    Imp *imp() const noexcept;

protected:
    virtual void suggestedSizeChanged();
    virtual void render() noexcept;
    void onUpdate() noexcept override;

private:
    std::unique_ptr<Imp> m_imp;
};

#endif // MLAYERSHELL_H
