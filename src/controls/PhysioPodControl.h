#ifndef PhysioPodControl_h
#define PhysioPodControl_h

class PhysioPodControl {
protected:
    void (*onPressedCallback)(); //the callback given by the pod to call when the button is pressed

public:
    virtual void initialize(void (*callback)()) = 0;
    //virtual void stop() = 0;
    virtual bool checkControl() = 0;

    virtual ~PhysioPodControl() {}
};

#endif