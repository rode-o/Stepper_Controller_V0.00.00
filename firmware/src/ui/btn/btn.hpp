#pragma once
#include <stdint.h>

namespace UI
{
    /** Logical buttons coming from ButtonsTwo (and used by the UI) */
    enum class Btn : uint8_t
    {
        UP,
        DOWN,
        OK,
        NEXT_DIGIT        // short dual-press during weight entry
    };
} // namespace UI
