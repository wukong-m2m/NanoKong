location_tree = None
wukong_status = ""

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

def get_wukong_status():
  global wukong_status
  return wukong_status

def set_wukong_status(status):
  global wukong_status
  print '-----set_wukong_status %s' % (status)
  wukong_status = status
