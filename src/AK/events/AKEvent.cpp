#include <AK/events/AKEvent.h>
#include <AK/events/AKPointerEnterEvent.h>
#include <AK/events/AKPointerLeaveEvent.h>
#include <AK/events/AKPointerMoveEvent.h>
#include <AK/events/AKPointerButtonEvent.h>
#include <AK/events/AKPointerScrollEvent.h>
#include <AK/events/AKPointerSwipeBeginEvent.h>
#include <AK/events/AKPointerSwipeUpdateEvent.h>
#include <AK/events/AKPointerSwipeEndEvent.h>
#include <AK/events/AKPointerPinchBeginEvent.h>
#include <AK/events/AKPointerPinchUpdateEvent.h>
#include <AK/events/AKPointerPinchEndEvent.h>
#include <AK/events/AKPointerHoldBeginEvent.h>
#include <AK/events/AKPointerHoldEndEvent.h>
#include <AK/events/AKKeyboardEnterEvent.h>
#include <AK/events/AKKeyboardLeaveEvent.h>
#include <AK/events/AKKeyboardKeyEvent.h>
#include <AK/events/AKKeyboardModifiersEvent.h>
#include <AK/events/AKTouchDownEvent.h>
#include <AK/events/AKTouchMoveEvent.h>
#include <AK/events/AKTouchUpEvent.h>
#include <AK/events/AKTouchFrameEvent.h>
#include <AK/events/AKTouchCancelEvent.h>

using namespace AK;

AKEvent *AKEvent::copy() const noexcept
{
    switch (m_type)
    {
    case Type::Pointer:
    {
        switch (m_subtype)
        {
        case Subtype::Enter:
            return (AKEvent*)new AKPointerEnterEvent((const AKPointerEnterEvent&)*this);
        case Subtype::Leave:
            return (AKEvent*)new AKPointerLeaveEvent((const AKPointerLeaveEvent&)*this);
        case Subtype::Move:
            return (AKEvent*)new AKPointerMoveEvent((const AKPointerMoveEvent&)*this);
        case Subtype::Button:
            return (AKEvent*)new AKPointerButtonEvent((const AKPointerButtonEvent&)*this);
        case Subtype::Scroll:
            return (AKEvent*)new AKPointerScrollEvent((const AKPointerScrollEvent&)*this);
        case Subtype::SwipeBegin:
            return (AKEvent*)new AKPointerSwipeBeginEvent((const AKPointerSwipeBeginEvent&)*this);
        case Subtype::SwipeUpdate:
            return (AKEvent*)new AKPointerSwipeUpdateEvent((const AKPointerSwipeUpdateEvent&)*this);
        case Subtype::SwipeEnd:
            return (AKEvent*)new AKPointerSwipeEndEvent((const AKPointerSwipeEndEvent&)*this);
        case Subtype::PinchBegin:
            return (AKEvent*)new AKPointerPinchBeginEvent((const AKPointerPinchBeginEvent&)*this);
        case Subtype::PinchUpdate:
            return (AKEvent*)new AKPointerPinchUpdateEvent((const AKPointerPinchUpdateEvent&)*this);
        case Subtype::PinchEnd:
            return (AKEvent*)new AKPointerPinchEndEvent((const AKPointerPinchEndEvent&)*this);
        case Subtype::HoldBegin:
            return (AKEvent*)new AKPointerHoldBeginEvent((const AKPointerHoldBeginEvent&)*this);
        case Subtype::HoldEnd:
            return (AKEvent*)new AKPointerHoldEndEvent((const AKPointerHoldEndEvent&)*this);
        case Subtype::Cancel:
        case Subtype::Up:
        case Subtype::Down:
        case Subtype::Key:
        case Subtype::Modifiers:
        case Subtype::Frame:
            return nullptr;
        }
        break;
    }
    case Type::Keyboard:
    {
        switch (m_subtype)
        {
        case Subtype::Enter:
            return (AKEvent*)new AKKeyboardEnterEvent((const AKKeyboardEnterEvent&)*this);
        case Subtype::Leave:
            return (AKEvent*)new AKKeyboardLeaveEvent((const AKKeyboardLeaveEvent&)*this);
        case Subtype::Key:
            return (AKEvent*)new AKKeyboardKeyEvent((const AKKeyboardKeyEvent&)*this);
        case Subtype::Modifiers:
            return (AKEvent*)new AKKeyboardModifiersEvent((const AKKeyboardModifiersEvent&)*this);
        case Subtype::Up:
        case Subtype::Down:
        case Subtype::Move:
        case Subtype::Button:
        case Subtype::Scroll:
        case Subtype::Frame:
        case Subtype::Cancel:
        case Subtype::SwipeBegin:
        case Subtype::SwipeUpdate:
        case Subtype::SwipeEnd:
        case Subtype::PinchBegin:
        case Subtype::PinchUpdate:
        case Subtype::PinchEnd:
        case Subtype::HoldBegin:
        case Subtype::HoldEnd:
            return nullptr;
        }
        break;
    }
    case Type::Touch:
    {
        switch (m_subtype)
        {
        case Subtype::Down:
            return (AKEvent*)new AKTouchDownEvent((const AKTouchDownEvent&)*this);
        case Subtype::Move:
            return (AKEvent*)new AKTouchMoveEvent((const AKTouchMoveEvent&)*this);
        case Subtype::Up:
            return (AKEvent*)new AKTouchUpEvent((const AKTouchUpEvent&)*this);
        case Subtype::Frame:
            return (AKEvent*)new AKTouchFrameEvent((const AKTouchFrameEvent&)*this);
        case Subtype::Cancel:
            return (AKEvent*)new AKTouchCancelEvent((const AKTouchCancelEvent&)*this);
        case Subtype::Enter:
        case Subtype::Leave:
        case Subtype::Button:
        case Subtype::Key:
        case Subtype::Modifiers:
        case Subtype::Scroll:
        case Subtype::SwipeBegin:
        case Subtype::SwipeUpdate:
        case Subtype::SwipeEnd:
        case Subtype::PinchBegin:
        case Subtype::PinchUpdate:
        case Subtype::PinchEnd:
        case Subtype::HoldBegin:
        case Subtype::HoldEnd:
            return nullptr;
        }
        break;
    }
    }

    return nullptr;
}


