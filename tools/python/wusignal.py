reconfiguration_signal = False

def signal_handler(ioloop, active_application):
    if reconfiguration_signal:
        active_application().reconfiguration()
