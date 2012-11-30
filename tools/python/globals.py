active_ind = 0
applications = []

MASTER_BUSY = False

def active_application():
  global applications
  global active_ind
  return applications[active_ind]

