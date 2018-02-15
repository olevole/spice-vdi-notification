#!/usr/bin/env python2.7
# a dirty test code for checking VDI-notification server
import json
from websocket_server import WebsocketServer

clients = {}

def update_status(id,status,name):
   here = 0
   for i in range(0,len(arr_all_profiles)):
     t=arr_all_profiles[i]
     cid=t["id"]
     print ( "C[%d]\n" % cid )
     print ( "I[%d]\n" % id )
     print ( "\n" )
     if ( cid == id ):
       here = 1
       t["status"] = status
       t["name"] = name
       print ( "UPDATED" )
       break
   if ( here == 0 ):
     print ( "NOT FOUND\n" )


def client_left(client, server):
    msg = "Client (%s) left" % client['id']
    print msg
    try:
        clients.pop(client['id'])
    except:
        print "Error in removing client %s" % client['id']

def new_client(client, server):
    msg = "New client (%s) connected" % client['id']
    print msg
    clients[client['id']] = client
    server.send_message(client,json.dumps(arr_all_profiles))

def is_json(myjson):
  try:
    json_object = json.loads(myjson)
  except ValueError, e:
    return False
  return True

def msg_received(client, server, msg):
    print msg
    print ( "\n" )
    if is_json(msg):
       print "JSON:\n"
       x= json.loads(msg)
       print(json.dumps(x))
       print(json.dumps(msg))
       try:
          command=x["command"]
          if ( command == "update" ):
            print ( "UPDATE here\n" )
            status = x["status"]
            id = int(x["id"])
            if 'name' not in x:
              name="NOOP"
            else:
              name=x["name"]
            update_status(id,status,name)
       except ValueError, e:
          print ( "no command here" )
    print ( "\n" )
    clientid = client['id']
    for cl in clients:
        if cl != clientid:
            cl = clients[cl]
            server.send_message(cl, msg)

arr_all_profiles=[]
arr_profile = []

for i in range(1,15):
 id=i
 status=0
 group=1
 name="test"

 arr_profile = {
  "id": id,
  "status": status,
  "group": group,
  "name": "slot%s"%i,
 }

 arr_all_profiles.append(arr_profile)

for i in range(16,30):
 id=i
 status=0
 group=2
 name="slot"

 if ( i == 22 ):
  status = 1
 if ( i == 28 ):
  status = 2


 arr_profile = {
  "id": id,
  "status": status,
  "group": group,
  "name": "temp%s"%i,
 }

 arr_all_profiles.append(arr_profile)

print(json.dumps(arr_all_profiles))


server = WebsocketServer(9001, "0.0.0.0")
server.set_fn_client_left(client_left)
server.set_fn_new_client(new_client)
server.set_fn_message_received(msg_received)
server.run_forever()
