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
void log(String logType, String logMessage);

Task taskSendMessage(TASK_SECOND * 1 , TASK_FOREVER, &sendMessage); // Debug: To be removed.

void sendMessage() {
  /**
    * This is a debug-function which is going to be removed.
  */

  /*
  sendMessageBroadcast("message", "Message A", 10, 10, true, 10, 5, NULL, NULL);
  */
  int random_num = random(0, 10);
  if (random_num == 0) { log("error", "Division by zero"); }
  if (random_num == 1) { log("error", "Out of bounds"); }
  if (random_num == 2) { log("error", "Stack overflow"); }
  if (random_num == 3) { log("info", "i am bored"); }
  if (random_num == 4) { log("info", "who let the dogs out"); }
  if (random_num == 5) { log("info", "i refuse to send messages"); }
  if (random_num == 6) { sendMessageBroadcast("message", "Message A", 0, 4, true, 10, 5, NULL, NULL); }
  if (random_num == 7) { sendMessageBroadcast("message", "Message B", 4, 3, true, 10, 5, NULL, NULL); }
  if (random_num == 8) { sendMessageBroadcast("message", "Message C", 9, 9, true, 10, 5, NULL, NULL); }
  if (random_num == 9) { sendMessageBroadcast("message", "Message D", 5, 6, true, 10, 5, NULL, NULL); }
  
}

void sendMessageBroadcast(String messageType, String message, int grid_position_x, int grid_position_y, bool need_charge, double priority, int queue_position, uint32_t ante, uint32_t post) {
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
  double priority        = doc["priority"];
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

void log(String logType, String logMessage, int x, int y, bool needs_charge, double priority, int queue_position, uint32_t ante, uint32_t post)
{
  /*
   * This function sends the logType and logMessage to the logging-server. 
   * The other variables such as x, y, needs_charge, priority, queue_position, ante and post 
   * is needed to update all the variables in the logging-server. These can be set to NULL
   * if the variable is not set or should be ignored by the logging-server.
   * logType and logMessage can not be NONE.
  */
  String logTypeConfig =  "LOG:" + logType;

  sendMessageBroadcast(
    logTypeConfig, 
    logMessage,
    
    // These can be set to NULL if they should be ignored by the logging-server
    x, y, needs_charge, priority, queue_position, ante, post
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
