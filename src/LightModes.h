
enum LightMode {
    SIMPLE, //just on
    BLINK_FAST,
    BLINK_SLOW,
    CYCLE_FAST,
    CYCLE_SLOW,
    CYCLE_RAINBOW_FAST,
    CYCLE_RAINBOW_SLOW,
    PULSE_ON_OFF_SHORT, //short pulse then turns off
    PULSE_ON_OFF_LONG, //long pulse then turns off
    LOADING_BAR, //only some of the LEDs are on, like a progress bar
    UNLOADING_BAR, //only some of the LEDs are on, like a progress bar, but in reverse
};
