#pragma once
#include "../_ctrl.hpp"          // IController base

class OpenLoopCtrl : public IController {
public:
    void setup() override;
    void loop () override;
};
