#pragma once
/*  iface_ctrl.hpp – common abstract base for controllers
 *  -----------------------------------------------------
 *  Both ClosedLoopCtrl and OpenLoopCtrl inherit from this so
 *  MainCtrl can own a polymorphic pointer (std::unique_ptr<IController>).
 */
struct IController
{
    virtual void setup() = 0;   // called once at initialisation / mode-switch
    virtual void loop () = 0;   // called every scheduler tick
    virtual ~IController() = default;
};
