/* header file for elekIO 
* created by HH Oct 2003 
* changelog 
*
*
*/

#define IP_ELEK_SERVER              "127.0.0.1"
#define IP_DEBUG_CLIENT             "127.0.0.1"
#define IP_STATUS_CLIENT            "127.0.0.1"
#define IP_ETALON_CLIENT            "127.0.0.1"
#define IP_SCRIPT_CLIENT            "127.0.0.1"
#define IP_LOCALHOST                "127.0.0.1"

#define LEN_IP_ADDR                 16


#define UDP_ELEK_STATUS_REQ_INPORT     1110    // port for status
#define UDP_ELEK_MANUAL_INPORT         1120    // port for incoming manual commands to ElekServer 
#define UDP_ELEK_ETALON_INPORT         1130    // port for Etalon Task
#define UDP_ELEK_SCRIPT_INPORT         1140    // port for Script Task

#define UDP_ELEK_STATUS_REQ_OUTPORT    1111    // port for status messages
#define UDP_ELEK_MANUAL_OUTPORT        1121    // port for answers to manual
#define UDP_ELEK_ETALON_OUTPORT        1131    // port for answer to Etalon Task
#define UDP_ELEK_SCRIPT_OUTPORT        1141    // port for answer to Script Task

#define UDP_ELEK_STATUS_STATUS_OUTPORT 1200    // port for status (elekIO->Status)
#define UDP_ELEK_ETALON_STATUS_OUTPORT 1210    // port for status (elekIO->etalon)
#define UDP_ELEK_DEBUG_OUTPORT         1300    // port for debug messages


#define UDP_SERVER_TIMEOUT               5     // timeout until elek server makes its round 


#define UDP_IN_PORT                    0       // defines UDP incoming port
#define UDP_OUT_PORT                   1       // defines UDP outgoing port

#define MAX_PORT_NAME_LEN     30


enum MsgTypeListEnum {                   // the list of available Message Types
    MSG_TYPE_READ_DATA,
    MSG_TYPE_WRITE_DATA,
    MSG_TYPE_ACK,                          // ACKNOLEDGE of execution
    MSG_TYPE_SIGNAL,                       // signals action
    MSG_TYPE_CHANGE_FLAG_STATUS_QUERY,     // change StatusQueryFlag
    MSG_TYPE_CHANGE_FLAG_STATUS_SAVE,      // change StatusSave
    MSG_TYPE_CHANGE_FLAG_ETALON_ACTION,    // change EtalonAction
    MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER, // change system parameter
    MSG_TYPE_CHANGE_MASK,				   // change counter mask

    MAX_MSG_TYPE }; 

enum SysParameterListEnum {                 // list of System Parameters
  
  SYS_PARAMETER_ETALON_ONLINE,
  SYS_PARAMETER_ETALON_OFFLINE_LEFT,
  SYS_PARAMETER_ETALON_OFFLINE_RIGHT,

  SYS_PARAMETER_ETALON_DITHER,

  SYS_PARAMETER_ETALON_SCAN_START,
  SYS_PARAMETER_ETALON_SCAN_STOP,
  SYS_PARAMETER_ETALON_SCAN_STEP,
  
  MAX_SYS_PARAMETER };

struct MessagePortType {                // struct to keep all udp information
    char PortName[MAX_PORT_NAME_LEN];   // Port Name
    int  PortNumber;                    // Portnumber
    int  RevMessagePort;                // PortlistNumber of reverse direction
    char IPAddr[LEN_IP_ADDR];           // IP Adress for the udp port
    int  fdSocket;                      // socket of Port
    int  MaxMsg;                        // Maximum Messages to poll
    int  Direction;                     // in or out port (UDP_IN_PORT UDP_OUT_PORT)
}; /* MessagePortType */

struct ElekMessageType {
    uint64_t MsgID;                     // ID of Message
    uint64_t MsgTime;                   // Time of Message
    uint16_t MsgType;                   // Message Type
    uint16_t Addr;                      // address of elektronik port
    uint64_t Value;                     // value to transmit or recieve depending on MsgType
    uint16_t Status;                    // status of message (error, success...)
}; /* ElekMessageType */



