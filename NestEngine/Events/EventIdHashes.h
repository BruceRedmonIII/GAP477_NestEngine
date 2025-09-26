#pragma once
#include "../Math/Hash.h"
namespace nest
{
    using HashedId = uint64_t;
    static constexpr HashedId kInputEventId = nest::Hash("InputEvent");
    static constexpr HashedId kMouseEventId = nest::Hash("MouseEvent");
    static constexpr HashedId kTextInputEventId = nest::Hash("TextInputEvent");
    static constexpr HashedId kLocalizationEventId = nest::Hash("LocalizationEvent");
    static constexpr HashedId kFocusChangedEvent = nest::Hash("FocusChangedEvent");
    static constexpr HashedId kMouseClickedId = nest::Hash("MouseClicked");
    static constexpr HashedId kMouseReleasedId = nest::Hash("MouseReleased");
    static constexpr HashedId kQuitGame = nest::Hash("QuitGame");
    static constexpr HashedId kNone = nest::Hash("None");
    static constexpr HashedId kOnClick = nest::Hash("OnClick");
    static constexpr HashedId kOnRelease = nest::Hash("OnRelease");
    static constexpr HashedId kMinFocusLayerChange = nest::Hash("MinFocusLayerChange");

    // horizontal
    static constexpr HashedId kLeftAlign = Hash("Left");
    static constexpr HashedId kMiddleAlign = Hash("Middle");
    static constexpr HashedId kRightAlign = Hash("Right");

    // vertical
    static constexpr HashedId kTopAlign = Hash("Top");
    static constexpr HashedId kCenterAlign = Hash("Center");
    static constexpr HashedId kBottomAlign = Hash("Bottom");

    // view type
    static constexpr HashedId kStackView = Hash("StackView");
    static constexpr HashedId kGridView = Hash("GridView"); 
    static constexpr HashedId kWrapView = Hash("WrapView");
    static constexpr HashedId kTableView = Hash("TableView");
    //...
    static constexpr HashedId kDoorId = nest::Hash("Door");
    static constexpr HashedId kHeroId = nest::Hash("Hero");
    static constexpr HashedId kDefaultTileId = nest::Hash("default tile");
}