class Convert:
    @staticmethod
    def CamelCase_to_underscore(name):
        s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
        return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

    @staticmethod
    def underscore_to_CamelCase(name):
        return ''.join(x.capitalize() or '_' for x in name.split('_'))

    @staticmethod
    def to_c(raw):
        if re.search('.*_.*', raw) == None:
            return Convert.CamelCase_to_underscore(raw)
        else:
            return raw.lower()

    @staticmethod
    def to_java(raw):
        if re.search('.*_.*', raw):
            return Convert.underscore_to_CamelCase(raw)
        else:
          return raw

    @staticmethod
    def to_constant(raw):
        return Convert.to_c(raw).upper()


def get_immediate_subdirectories(dir):
  return [name for name in os.listdir(dir)
          if os.path.isdir(os.path.join(dir, name))]

def findInSubdirectory(filename, subdirectory=''):
  if subdirectory:
    path = subdirectory
  else:
    path = os.getcwd()

  for root, dirs, names in os.walk(path):
    if filename in names:
      return os.path.join(root, filename)

    for dirname in dirs:
      result = findInSubdirectory(filename, os.path.join(path, dirname))
      if result != None:
        return result

  return None

def get_all_subdirectories(dir):
  directories = []
  directories += map(lambda x: os.path.join(dir, x), get_immediate_subdirectories(dir))
  for directory in map(lambda x: os.path.join(dir, x), get_immediate_subdirectories(dir)):
    directories += get_all_subdirectories(directory)
  return directories
