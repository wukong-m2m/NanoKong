# kong_client.py
import sys
from poster.encode import multipart_encode
from poster.streaminghttp import register_openers
import urllib2

IP = '127.0.0.1'
if len(sys.argv) > 2:
  IP = sys.argv[1]

# Register the streaming http handlers with urllib2
register_openers()

# Start the multipart/form-data encoding of the file "DSC0001.jpg"
# "image1" is the name of the parameter, which is normally set
# via the "name" parameter of the HTML <input> tag.

# headers contains the necessary Content-Type and Content-Length
# datagen is a generator object that yields the encoded parameters
datagen, headers = multipart_encode({'file': open(sys.argv[2])})

# Create the Request object
request = urllib2.Request("http://"+IP+":5000/", datagen, headers)
# Actually do the request, and get the response
print urllib2.urlopen(request).read()
