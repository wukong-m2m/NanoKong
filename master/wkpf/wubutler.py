import wkpf.util
from wkpf.wuapplication import WuApplication
import wkpf.globals
from wkpf.configuration import *

class WuButler:

  __instance = None

  class __impl():
    def __init__(self):
      self.applications = []
      self.current_application = None

  def __init__(self):
    if WuButler.__instance is None:
      WuButler.__instance = WuButler.__impl()

    self.__dict__['_Singleton__instance'] = WuButler.__instance

  def __getattr__(self, attr):
    return getattr(self.__instance, attr)

  def __setattr__(self, attr, value):
    return setattr(self.__instance, attr, value)

  def load_applications(self, base=APP_DIR):
    application_basenames = [os.path.basename(app.dir) for app_id, app in self.applications]

    for dirname in os.listdir(base):
      app_dir = os.path.join(base, dirname)
      if dirname.lower() == 'base': continue
      if not os.path.isdir(app_dir): continue

      if dirname not in application_basenames:
        app = WuApplication(dir=app_dir)
        self.applications.append(app)
        application_basenames.append(os.path.basename(app.dir))

  def create_application(self, name=""):
    if name == "":
      name = 'application' + str(len(self.applications))
    id = hashlib.md5(name).hexdigest()
    dir = os.path.join(APP_DIR, id)

    util.copyAnything(BASE_DIR, dir)
    application = WuApplication(id=id, name=name, dir=dir)
    self.applications.append(application)
    return application

  def application_by_id(self, app_id):
    for app in self.applications:
      if app.id == app_id:
        return app
    return None

