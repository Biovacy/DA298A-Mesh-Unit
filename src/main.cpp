#include <Arduino.h>
#include <painlessMesh.h> 
#include <ArduinoJson.h>

#define MESH_PREFIX   "mesh"
#define MESH_PASSWORD "meshmeshmesh"
#define MESH_PORT 5

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

void sendMessageBroadcast(String messageType, String message, int grid_position_x, int grid_position_y, bool need_charge, int priority, int queue_position, uint32_t ante, uint32_t post);
void sendMessage(); // Debug: to be removed

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage); // Debug: To be removed.

void sendMessage() {
  /**
    * This is a debug-function which is going to be removed.
  */
    sendMessageBroadcast(
      "Debug",
      "Hello, me name Galiano!",
      10,
      10,
      true,
      10,
      5,
      NULL,
      NULL
    );
}

void sendMessageBroadcast(String messageType, String message, int grid_position_x, int grid_position_y, bool need_charge, int priority, int queue_position, uint32_t ante, uint32_t post) {
  /**
    * This function broadcast a message to all other nodes in the peer-to-peer network. 
  */

  DynamicJsonDocument doc(1400);

  doc["from"]             = mesh.getNodeId();
  doc["to"]               = 0;    // A broadcast has the ID of 0
  doc["messageType"]      = messageType;
  doc["message"]          = message;
  doc["grid_position_x"]  = grid_position_x;
  doc["grid_position_y"]  = grid_position_y;
  doc["need_charge"]      = need_charge;
  doc["priority"]         = priority;
  doc["queue_position"]   = queue_position;
  doc["ante"]             = ante;
  doc["post"]             = post;

  String msg;
  serializeJson(doc, msg);

  mesh.sendBroadcast(msg);
}

// Needed for painless library
void receiveMessageCallback( uint32_t from, String &msg ) {

  DynamicJsonDocument doc(1400);
  deserializeJson(doc, msg);

  uint32_t to         = doc["to"]; // This will be a zero if the message is a broadcast!
  String messageType  = doc["messageType"];
  String message      = doc["message"];
  int grid_position_x = doc["grid_position_x"];
  int grid_position_y = doc["grid_position_y"];
  bool need_charge    = doc["need_charge"];
  int priority        = doc["priority"];
  int queue_position  = doc["queue_position"];
  uint32_t ante       = doc["ante"]; 
  uint32_t post       = doc["post"];

  // To print out.
  String fixed_message = messageType + ": " + message;
  Serial.println(fixed_message);

  // ADD CODE HERE BELOW
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  mesh.sendBroadcast( "announcingLogger" );
}

void log(String logType, String logMessage) {
  String logTypeConfig =  "LOG:" + logType;

  sendMessageBroadcast(
    logTypeConfig, logMessage,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL  // These values are not used in logging.
  );
}

void setup() {
  // Serialport init
  Serial.begin(115200);
  Serial.println("Serial initialized");

  // Painless Mesh Init
  mesh.setDebugMsgTypes( ERROR | STARTUP );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receiveMessageCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  // DEBUG-MEDDELANDE
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  Serial.println("INIT COMPLETED");
}

void loop() {
  // put your main code here, to run repeatedly:
  mesh.update();
}