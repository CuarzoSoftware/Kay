#ifndef CZ_AKSCENE_H
#define CZ_AKSCENE_H

#include <CZ/AK/Nodes/AKNode.h>
#include <CZ/AK/AKObject.h>
#include <CZ/AK/AKTarget.h>
#include <CZ/Core/CZWindowState.h>
#include <CZ/Core/CZTimer.h>
#include <memory.h>
#include <vector>

class CZ::AKScene : public AKObject
{
public:
    [[nodiscard]] static std::shared_ptr<AKScene> Make() noexcept;
    ~AKScene();
    bool isSubScene() const noexcept { return m_isSubScene; };

    [[nodiscard]] std::shared_ptr<AKTarget> makeTarget() noexcept;
    const std::vector<AKTarget*> &targets() const noexcept { return m_targets; }

    bool render(std::shared_ptr<AKTarget> target) noexcept;

    // Target passed to render(), set to nullptr after the call finishes
    std::shared_ptr<AKTarget> currentTarget() const noexcept { return ct; }

    /**
     * @brief Root node.
     *
     * Only the children of the root node are rendered.
     * The root node's bounds do not clip its children, but its layout properties may affect them.
     */
    void setRoot(AKNode *node) noexcept;
    AKNode *nodeAt(const SkPoint &pos) const noexcept;
    AKNode *root() const noexcept { return m_root; }

    void setPointerGrab(AKNode *node) noexcept;
    AKNode *pointerGrab() const noexcept;
    AKNode *pointerFocus() const noexcept;
    AKNode *keyboardFocus() const noexcept;
    AKNode *nextKeyboardFocusable() const noexcept;
    CZBitset<CZWindowState> windowState() const noexcept;
protected:
    bool event(const CZEvent &event) noexcept override;
private:
    friend class AKTarget;
    friend class AKNode;
    friend class AKSubScene;
    static std::shared_ptr<AKScene> MakeSubScene() noexcept;
    AKScene(bool isSubScene) noexcept;
    bool validateTarget(std::shared_ptr<AKTarget> target) noexcept;
    void layoutTree() noexcept;
    void setupInvisibleRegion() noexcept;
    void treeNotifyBegin() noexcept;
    void notifyBegin(AKNode *node);
    void calculateTreeDamage() noexcept;
    void renderBackground() noexcept;
    void renderTree() noexcept;
    void resetTarget() noexcept;
    std::weak_ptr<AKScene> m_self;
    std::shared_ptr<AKTarget> ct;
    std::shared_ptr<RPass> pass;
    std::vector<AKTarget*> m_targets;
    CZWeak<AKNode> m_root;

    struct Window
    {
        CZWeak<AKNode> pointerGrab;
        CZWeak<AKNode> pointerFocus;
        CZWeak<AKNode> keyboardFocus;
        CZTimer keyDelayTimer, keyRepeatTimer;
        Int64 repeatedKey { -1 };
        CZBitset<CZWindowState> windowState { CZWinActivated };
    };

    // Not available for sub scenes
    std::unique_ptr<Window> m_win;
    bool m_isSubScene { false };
    bool m_treeChanged { false };
    bool m_eventWithoutTarget { false };
    void addNodeDamage(AKNode &node, const SkRegion &damage) noexcept;
    void createOrAssignTargetDataForNode(AKNode *node) noexcept;
    void calculateNewDamage(AKNode *node);
    void updateDamageRing() noexcept;
    void updateDamageTrackers() noexcept;
    void backgroundPass(std::shared_ptr<RPass> pass, SkRegion &region) noexcept;
    void renderNodes(AKNode *node);
    void nodeTranslucentPass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept;
    void nodeOpaquePass(AKRenderable *node, std::shared_ptr<RPass> pass, SkRegion &region) noexcept;

    void pointerEnterEvent(const CZPointerEnterEvent &e) noexcept;
    void pointerMoveEvent(const CZPointerMoveEvent &e) noexcept;
    void pointerButtonEvent(const CZPointerButtonEvent &e) noexcept;
    void pointerScrollEvent(const CZPointerScrollEvent &e) noexcept;
    void pointerLeaveEvent(const CZPointerLeaveEvent &e) noexcept;

    void keyboardEnterEvent(const CZKeyboardEnterEvent &e) noexcept;
    void keyboardKeyEvent(const CZKeyboardKeyEvent &e) noexcept;
    void keyboardLeaveEvent(const CZKeyboardLeaveEvent &e) noexcept;

    void windowStateEvent(const CZWindowStateEvent &e) noexcept;

    static void SetPassParamsFromRenderable(std::shared_ptr<RPass> pass, AKRenderable *rend, bool opaque) noexcept;
};

#endif // CZ_AKSCENE_H
