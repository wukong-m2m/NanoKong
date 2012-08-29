# Wukong Master server

This repository contains the source code for Wukong Master server, a python tornado server for Python 2.7+

# Prerequisites

To run the server the following must be present in your system:

Python modules:

* lxml               - a fast xml parser
* tornado            - a scalable, non-blocking web server

Install using [easy_install][] run:

    sudo easy_install lxml tornado

Or using [pip][](You can install pip by `sudo easy_install pip`) run:

    sudo pip install lxml tornado

# Running

The server is always on port 5000, but it can be changed easily in the source code.

To run the server:

	python master_server.py [IP]
	
IP is the ip the server will bind to, so if you want to host a public server, you can bind it to a public IP, therefore it can be seen and access from the outside, otherwise omitting the parameter will default to "localhost"

# Structure

To get more information about internal workings of tornado server, please refer to [Tornado][]

As the time of writing, we are planning for Tornado to be a user interface to Wukong master node to manage, modify, deploy applications. Including, not limited to, the ability to create applications in flow based programming style by dragging components and link them together.

## Routes

Tornado server look a lot like Google's webapp structure. All the routes are specified in application constructor in tuple (r"/route/for/something", function_name) inside an array.

There are 13 routes so far:

* (r"/", main)
* (r"/main", main)
* (r"/testrtt/exclude", ex_testrtt)
* (r"/testrtt/include", in_testrtt)¬
* (r"/testrtt", testrtt)
* (r"/application/json", list_applications)
* (r"/application/new", new_application)
* (r"/application/([a-fA-F\d]{32})", application)
* (r"/application/([a-fA-F\d]{32})/deploy", deploy_application)
* (r"/application/([a-fA-F\d]{32})/deploy/poll", poll_deployer)
* (r"/application/([a-fA-F\d]{32})/fbp/save", save_fbp)¬
* (r"/application/([a-fA-F\d]{32})/fbp/load", load_fbp)¬
* (r"/application/([a-fA-F\d]{32})/fbp/poll", poll_fbp)¬

## Main window

![main](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/main.png)

Operations corresponding to the routes:

* Add application: /application/new [POST]
* Show application: /application/[1-fA-F\d]{32} [POST]
* Delete application: /application/[1-fA-F\d]{32} [DELETE]

## Application editor

![fbp](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/editor.png)

* Update application: /application/[1-fA-F\d]{32} [PUT]

## FBP.js (Flow Based Programming)

![fbp-logic]()

## Deploy window

![deploy](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/deploy.png)

![deploy-logic](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/deploy-logic.png)

## Monitor window

### Monitor logging

![monitor-logging](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-logging.png)

### Monitor mapping

![monitor-mapping](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-mapping.png)

### Monitor properties

![monitor-properties](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-properties.png)

# Contributing

## GitHub Workflow

[Tornado]:				http://www.tornadoweb.org/
[easy_install]:			http://pypi.python.org/pypi/setuptools
[pip]:						http://pypi.python.org/pypi/pip/