#include <AK/AKApplication.h>
#include <AK/AKAnimation.h>
#include <AK/AKTime.h>

#include <algorithm>

using namespace AK;

AKAnimation::AKAnimation(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) noexcept :
    m_onUpdate(onUpdate),
    m_duration(durationMs),
    m_onFinish(onFinish)
{
    akApp()->m_animations.push_back(this);
}

AKAnimation::~AKAnimation()
{
    notifyDestruction();
    stop();

    auto it { std::find(akApp()->m_animations.begin(), akApp()->m_animations.end(), this) };
    if (it != akApp()->m_animations.end())
    {
        akApp()->m_animationsChanged = true;
        akApp()->m_animations.erase(it);
    }
}

void AKAnimation::OneShot(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) noexcept
{
    AKAnimation *anim { new AKAnimation(durationMs, onUpdate, onFinish) };
    anim->m_destroyOnFinish = true;
    anim->start();
}

void AKAnimation::setOnUpdateCallback(const Callback &onUpdate) noexcept
{
    if (m_running)
        return;

    m_onUpdate = onUpdate;
}

void AKAnimation::setOnFinishCallback(const Callback &onFinish) noexcept
{
    if (m_running)
        return;

    m_onFinish = onFinish;
}

void AKAnimation::start() noexcept
{
    if (m_running)
        return;

    m_value = 0.0;
    m_beginTime = std::chrono::steady_clock::now();
    m_running = true;

    if (m_onUpdate)
    {
        akApp()->m_animationsTimer->start(8);
        m_onUpdate(this);
    }
}

void AKAnimation::stop()
{
    if (!m_running)
        return;

    m_value = 1.0;
    m_running = false;

    if (m_onFinish)
        m_onFinish(this);

    if (m_destroyOnFinish)
        m_pendingDestroy = true;
}
