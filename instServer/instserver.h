/***************************************************************************
                          instserver.h  -  description
                             -------------------
    begin                : Wed Oct 22 2003 by H.Harder
    email                : harder@mpch-mainz.mpg.de
 ***************************************************************************/

#define TCP_CLIENT_PORT               1111                      // port on which we accept our clients
#define TCP_CLIENT_MAX                10
#define TCP_ASC_ADR_LEN               16
#define TCP_CLIENT_WDG                10               
#define TCP_CLIENT_BUFSIZE            512               
#define CLIENT_ASC_RIGHT_LEN          10
#define MAX_CLIENT_NAME               128
#define MAX_CLIENT_NUMBER             5
#define MAX_FN_NAME_LEN               64          // maximum length of single command name
#define MAX_MSG_LEN                   512         // maximum number of bytes in single message

#define MSG_DELIM                     " ,;"       // delimiter for message parsing


enum CmdDeviceType {        // enum to list all commands to id the device type
   TCP_CMD_DEV_MOTOR,
   TCP_CMD_DEV_VALVE,

   TCP_CMD_COUNT           // last item in list to count items
};

enum CmdMotorIDType {        // enum to list all motor ID's
   MOTOR_ID_ETALON,          // stepper for etalon in laser
   MOTOR_ID_GREEN1,          // first turning mirror of green laser
   MOTOR_ID_GREEN2,            // second turning mirror of green laser
   MOTOR_ID_UV_FIBER,           // turning mirror of UV beam into fiber

   MOTOR_ID_COUNT             // last item in list to count items
};

enum CmdMotorAttribType {   // enum to list all motor attributes
   MOTOR_ATTRIB_SPD,        // speed attribute
   MOTOR_ATTRIB_POS,         // position attribute

   MOTOR_ATTRIB_COUNT        // last item in list to count items
};
   
enum TCPErrorType {
    TCP_ERR_CLIENT_LIST_FULL=-2,   // can't add client because list is full
    TCP_ERR_CLIENT_UNKNOWN=-1      // can't find specified client in list
};

typedef int (*func)();

struct InterprFktType {    
    char FName[MAX_FN_NAME_LEN];
    int (*func)();
    unsigned FktNum;
};


struct ClientMessageType {
    unsigned FktNum;                // identified function
    unsigned MessageNum;            // MessageNumber
    unsigned DeviceNum;             // identified Device
    char Arg[MAX_MSG_LEN];          // args to device
    unsigned Answer;                // answer of device
};


struct ClientRightsType {    // struct to define access rights of clients
   unsigned SetAll:1;
   unsigned SetValve:1;
   unsigned SetLaser:1;
};
  
struct ClientListType {      // struct to keep the info of an connected client
    int ClientID;
    char ClientName[MAX_CLIENT_NAME];
    struct sockaddr_in ClientAddr;
    struct ClientRightsType ClientRights;
};

struct ClientRightAscType {  // struct which builds the readable master Client IP->rights list
    char *ClientIP;
    char *ClientRight;
    char *ClientName;
};

struct ActiveClientStructType {                 // struct to keep all active clients
    unsigned NumActiveClient;                 // number of ActiveClients
    char  *ptrComBuf;                                // Buffer in which we can collect communication
    struct ClientListType *ptrActiveClientArray;   // Array to keep clients
    struct ClientListType *ptrActiveClientList;    // pointer list on active clients
    
};

// end of file  
