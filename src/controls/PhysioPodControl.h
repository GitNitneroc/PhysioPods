#ifndef PhysioPodControl_h
#define PhysioPodControl_h

class PhysioPodControl {
public:
    virtual void initialize() = 0;
    virtual void stop() = 0;
    virtual bool checkControl() = 0;

    virtual ~PhysioPodControl() {}
};

#endif