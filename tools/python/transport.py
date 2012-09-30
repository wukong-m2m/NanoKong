import os, fcntl
import pickle
import tornado.ioloop
import hashlib
import logging
from collections import namedtuple
import gevent
from gevent.event import AsyncResult
from gevent.queue import Queue

import pynvc # for message constants
import pyzwave
import pyzigbee

Message = namedtuple('Message', 'destination command payload')
Defer = namedtuple('Defer', 'callback verify allowed_replies message')

tasks = Queue()
messages = Queue()

def new_defer(*args):
    logging.info('new_defer')
    return Defer(*args)

def new_message(*args):
    logging.info('new_message')
    return Message(*args)

class DeferredQueue:
    def __init__(self):
        self.queue = {}

    def find_defer(self, message):
        logging.info('find_defer')
        for defer_id, defer in self.queue.items():
            if defer.verify(message, defer):
                logging.info('found defer')
                return defer_id, defer
            else:
                log = "Either one of " + str(defer.allowed_replies) + " expected from defer " + str(defer) + " does not match or the sequence number got skewed: " + str(message)
                logging.warning(log)
        logging.info('Cannot find defer')
        return False, False

    def add_defer(self, defer):
        logging.info('add_defer')
        queue_id = str(len(self.queue)) + hashlib.md5(str(defer.message.destination) + str(defer.message.command)).hexdigest()
        log = "queue_id " + str(queue_id)
        logging.info(log)
        self.queue[queue_id] = defer
        return queue_id

    def remove_defer(self, defer_id):
        if defer_id in self.queue:
            del self.queue[defer_id]
            return defer_id
        else:
            return False

    def get_defer(self, defer_id):
        if defer_id in self.queue:
            return self.queue[defer_id]
        else:
            return False

class TransportAgent:
    def __init__(self):
        self._seq = 0
        gevent.spawn(self.handler)
        gevent.spawn(self.receive)

    def getNextSequenceNumberAsPrefixPayload(self):
      self._seq = (self._seq + 1) % (2**16)
      return [self._seq/256, self._seq%256]

    # to be overridden, non-blocking, send defer to greelet thread
    def deferSend(self, destination, command, payload, allowed_replies, cb):
        pass

    # to be overridden, blocking before it returns
    def send(self, destination, command, payload, allowed_replies):
        pass

    # could be overridden 
    def verify(self, allowed_replies):
        return lambda message, defer: (message.command in allowed_replies and message.command != pynvc.WKPF_ERROR_R) and (message.payload != None and message.payload[0:2]==defer.message.payload[0:2])


    # to be run in a greenlet thread, context switching with handler
    def receive(self, timeout_msec):
        pass

    # to be run in a thread, and others will use ioloop to monitor pipe of this thread
    def handler(self):
        pass

class ZwaveAgent(TransportAgent):
    agent = None

    @classmethod
    def init(cls):
        logging.info('ZwaveAgent init')
        if not cls.agent:
            cls.agent = ZwaveAgent()
        return cls.agent

    def __init__(self):
        logging.info('ZwaveAgent constructor')
        TransportAgent.__init__(self)
        self._mode = 'stop'

        # pyzwave
        try:
            pyzwave.init(ZWAVE_GATEWAY_IP)
        except IOError as e:
            return False

    # add a defer to queue
    def deferSend(self, destination, command, payload, allowed_replies, cb):
        def callback(reply):
            cb(reply)

        defer = new_defer(callback, 
                self.verify(allowed_replies), 
                allowed_replies, 
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload))
        tasks.put_nowait(defer)
        return defer

    def send(self, destination, command, payload, allowed_replies):
        logging.info('ZwaveAgent send')

        result = AsyncResult()

        def callback(reply):
            logging.info('callback')
            logging.info(reply)
            result.set(reply)

        defer = new_defer(callback, 
                self.verify(allowed_replies), 
                allowed_replies, 
                new_message(destination, command, self.getNextSequenceNumberAsPrefixPayload() + payload))
        logging.info(str(defer))
        tasks.put_nowait(defer)


        message = result.get() # blocking
        logging.info('get message from result.get()')
        logging.info(message)

        # received ack from Agent
        return message

    def discovery(self):
        nodes = pyzwave.discover()
        gateway_id = nodes[0]
        total_nodes = nodes[1]
         # remaining are the discovered nodes
        return nodes[2:]

    def add(self):
        if self._mode != 'stop':
            logging.warning('need to stop first')
            return False

        try:
            pyzwave.add()
            self._mode = 'add'
            return True
        except:
            return False

    def delete(self):
        if self._mode != 'stop':
            logging.warning('need to stop first')
            return False

        try:
            pyzwave.delete()
            self._mode = 'delete'
            return True
        except:
            return False

    def stop(self):
        try:
            pyzwave.stop()
            self._mode = 'stop'
            return True
        except:
            return False

    def poll(self):
        try:
            pyzwave.poll()
            return True
        except:
            return False

    def receive(self, timeout_msec=100):
        while 1:
            logging.info('receiving')
            try:
                src, reply = pyzwave.receive(timeout_msec)
                if src and reply:
                    # with seq number
                    message = new_message(src, reply[0], reply[1:])
                    messages.put_nowait(message)
            except:
                logging.exception('receive exception')
            logging.info('receive: going to sleep')
            gevent.sleep(0.01) # sleep for at least 10 msec


    # to be run in a thread, and others will use ioloop to monitor this thread
    def handler(self):
        while 1:
            logging.info('just before handler tasks.get')
            defer = tasks.get()

            destination = defer.message.destination
            command = defer.message.command
            payload = defer.message.payload

            retries = 3

            while retries > 0:
                try:
                    pyzwave.send(destination, [0x88, command] + payload)

                    if len(defer.allowed_replies) > 0:
                        BrokerAgent.init().append(defer)

                    break
                except Exception as e:
                    log = "===========IOError========== retries remaining: " + str(retries)
                    logging.exception(log)
                retries -= 1

            if retries == 0 or len(defer.allowed_replies) == 0:
                # returns immediately to handle failues, or no expected replies
                defer.callback(None)

            gevent.sleep(0)

class BrokerAgent:
    agent = None

    @classmethod
    def init(cls):
        if not cls.agent:
            cls.agent = BrokerAgent()
        return cls.agent

    def __init__(self):
        #tornado.ioloop.PeriodicCallback(self.run, 100) # in a new ioloop instance
        self._defer_queue = DeferredQueue()
        self._agents = []
        gevent.spawn(self.run)

    def append(self, defer):
        logging.info('defer appended')
        self._defer_queue.add_defer(defer)

    def add(self, agent):
        self._agents.append(agent)

    def run(self):
        while 1:
            # monitor pipes from receive
            logging.info('just before run messages.get')
            message = messages.get()
            log = 'got message in run(): ' + str(message)
            logging.info(log)

            # find out which defer it is for
            defer_id, defer = self._defer_queue.find_defer(message)

            if defer_id and defer:
                # call callback
                defer.callback(message)

                # remove it
                self._defer_queue.remove_defer(defer_id)
            else:
                #log = "Incorrect reply received. Message type correct, but didnt pass verification: " + str(message)
                #logging.info(log)
                log = "Dropped message: " + str(message)
                logging.info(log)
            gevent.sleep(0)

agent = BrokerAgent.init()
