active_ind = 0
applications = []

MASTER_BUSY = False

def is_master_busy():
    global MASTER_BUSY
    return MASTER_BUSY

def master_busy():
    global MASTER_BUSY
    MASTER_BUSY = True

def master_available():
    global MASTER_BUSY
    MASTER_BUSY = False

def active_application():
  global applications
  global active_ind
  return applications[active_ind]

