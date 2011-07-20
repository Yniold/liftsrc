/*
* $RCSfile: elekIOPorts.h,v $ last changed on $Date: 2007-08-07 12:35:22 $ by $Author: rudolf $
*
* $Log: elekIOPorts.h,v $
* Revision 1.24  2007-08-07 12:35:22  rudolf
* preparations for recording spectral data as well
*
* Revision 1.23  2007-07-15 11:07:34  martinez
* elekIOPorts updated for receiveUDPData
*
* Revision 1.22  2007-06-12 12:08:01  martinez
* included eCmd commands set realignminutes and stoprealigning
*
* Revision 1.21  2007/03/08 13:09:07  rudolf
* added new port for elekIOaux->elekIOServ(M)
*
* Revision 1.20  2007-03-07 20:38:27  harder
* Changed RealingCommand name to CMD_Realign
*
* Revision 1.19  2007-03-07 16:06:46  rudolf
* more work on elekIOaux
*
* Revision 1.18  2007-03-06 11:34:31  harder
* added MsgType Calib Start & Stop
*
* Revision 1.17  2007-03-05 20:51:32  martinez
* debugging mirrors
*
* Revision 1.16  2007-03-05 16:06:58  martinez
* mirror moving implemented
*
* Revision 1.15  2007-02-21 19:33:24  harder
* changed IP address of status client to avsolut LIFT address, 10.111.111.196
*
* Revision 1.14  2007-02-21 19:30:06  harder
* added status port for calib, calib->statusIO
*
* Revision 1.13  2006-11-03 12:56:04  rudolf
* added new messagetypes for calibrator
*
* Revision 1.12  2006/10/15 08:55:19  harder
* ref channel can be now assigned to any counter channel
* eCmd: new command 'refchannel'
* elekIOServ : used etalon Status info to store channel info
* elekIO.h modified etalon structure in status
*
* Revision 1.11  2006/10/06 13:40:59  rudolf
* added new MSG_TYPE for sending new target position to butterfly valve
*
* Revision 1.10  2006/09/04 09:33:38  rudolf
* added ports and IP for Calibrator
*
* Revision 1.9  2005/06/25 18:32:58  rudolf
* changed IP to lift
*
* Revision 1.8  2005/06/22 18:09:43  rudolf
* added seperate IN port for receiving the data from Slave when in Master Mode
*
* Revision 1.7  2005/06/22 13:15:25  rudolf
* changed IP to my VMware for further investigations...
*
* Revision 1.6  2005/06/21 15:55:46  rudolf
* for yet unknow reason the whole system seems to freeze if we use the IP_ELEKIO_MASTER 10.111.111.196. Changed to Localhost 127.0.0.1 for further investigations :/
*
* Revision 1.5  2005/06/08 22:44:53  rudolf
* added preliminary master slave support, HH
*
* Revision 1.4  2005/06/08 17:38:27  rudolf
* update
*
* Revision 1.3  2005/05/23 17:05:02  rudolf
* added new command for requesting data from slave elekIOServ
*
*
*
*/
#ifndef ELEKIOPORTS_H
#define ELEKIOPORTS_H

#define IP_ELEK_SERVER              "127.0.0.1"      // local ELEKIO Server
#define IP_ELEKIO_MASTER            "10.111.111.196" // ELEKIO which is Master server (LIFT)
#define IP_DEBUG_CLIENT             "127.0.0.1"
#define IP_STATUS_CLIENT            "10.111.111.196"
#define IP_ETALON_CLIENT            "127.0.0.1"
#define IP_MIRROR_CLIENT            "127.0.0.1"
#define IP_SCRIPT_CLIENT            "127.0.0.1"
#define IP_LOCALHOST                "127.0.0.1"
#define IP_ARMAXIS                  "10.111.111.10" // wingpod IP 
#define IP_ARMCALIBRATOR            "10.111.111.13" // calibrator IP
#define IP_GSB_MPI					"10.111.111.15" // GSB
#define LEN_IP_ADDR                 16

// listening ports on lift side
#define UDP_ELEK_STATUS_REQ_INPORT     1110    // port for status
#define UDP_ELEK_MANUAL_INPORT         1120    // port for incoming manual commands to ElekServer 
#define UDP_ELEK_ETALON_INPORT         1130    // port for Etalon Task
#define UDP_ELEK_SCRIPT_INPORT         1140    // port for Script Task
#define UDP_ELEK_SLAVE_DATA_INPORT     1150    // port for reception of slave data in MasterMode
#define UDP_ELEK_CALIB_DATA_INPORT     1160    // port for reception of calibrator data in MasterMode
#define UDP_ELEK_MIRROR_INPORT         1170    // port for Mirror Task
#define UDP_ELEK_AUX_INPORT            1180    // port for Aux Data (weather, ship etc.)
#define UDP_ELEK_GSB_DATA_INPORT       1190	   // port for GSB Data in MasterMode
#define UDP_ELEK_SPECTRA_INPORT        4711    // port for spectral data from spectrometerServer

// reply ports from lift side
#define UDP_ELEK_STATUS_REQ_OUTPORT    1111    // port for status messages
#define UDP_ELEK_MANUAL_OUTPORT        1121    // port for answers to manual
#define UDP_ELEK_ETALON_OUTPORT        1131    // port for answer to Etalon Task
#define UDP_ELEK_SCRIPT_OUTPORT        1141    // port for answer to Script Task
#define UDP_ELEK_MIRROR_OUTPORT        1171    // port for answer to Etalon Task

// outgoing port numbers from clients
#define UDP_ELEK_STATUS_STATUS_OUTPORT 1200    // port for status (elekIO->Status)
#define UDP_ELEK_ETALON_STATUS_OUTPORT 1210    // port for status (elekIO->etalon)
#define UDP_ELEK_ELEKIO_STATUS_OUTPORT 1220    // port for status (elekIOSlave->elekIOMaster)
#define UDP_ELEK_DEBUG_OUTPORT         1300    // port for debug messages
#define UDP_CALIB_STATUS_STATUS_OUTPORT 1400    // port for status (Calib->Status)
#define UDP_ELEK_MIRROR_STATUS_OUTPORT 1270    // port for status (elekIO->etalon)
#define UDP_AUX_STATUS_STATUS_OUTPORT  1280    // port for status (elekAux->elekIOMaster)
#define UDP_GSB_STATUS_STATUS_OUTPORT  1290    // port for status (elekIOGSB->elekIOMaster)

#define UDP_SERVER_TIMEOUT               1     // timeout until elek server makes its round 


#define UDP_IN_PORT                    0       // defines UDP incoming port
#define UDP_OUT_PORT                   1       // defines UDP outgoing port

#define MAX_PORT_NAME_LEN     30


enum MsgTypeListEnum {                   // the list of available Message Types
    MSG_TYPE_READ_DATA,
    MSG_TYPE_WRITE_DATA,
    MSG_TYPE_ACK,                              // ACKNOWLEDGE of execution
    MSG_TYPE_SIGNAL,                           // signals action
    MSG_TYPE_CHANGE_FLAG_STATUS_QUERY,         // change StatusQueryFlag
    MSG_TYPE_CHANGE_FLAG_STATUS_SAVE,          // change StatusSave
    MSG_TYPE_CHANGE_FLAG_INSTRUMENT_ACTION,    // change InstrumentAction
    MSG_TYPE_CHANGE_FLAG_ETALON_ACTION,        // change EtalonAction
    MSG_TYPE_CHANGE_FLAG_SYSTEM_PARAMETER,     // change system parameter
    MSG_TYPE_CHANGE_MASK,                      // change counter mask
    MSG_TYPE_FETCH_DATA,                       // request data from slave elekIOServ
    MSG_TYPE_MOVE_BUTTERFLY,                   // goto a new target position
    MSG_TYPE_REF_CHANNEL,                      // set reference channel
    MSG_TYPE_MIRROR_MOVE,		       // move motorized mirror axis to new target position
    MSG_TYPE_MIRROR_CMD_REALIGN,		       // realign a motorized mirror to optimize signal
    MSG_TYPE_MIRROR_STOP,		       // stop all mirror movements
    MSG_TYPE_MIRROR_FLAG_REALIGN,		       // realignment in process
    
    // Calibrator
    MSG_TYPE_CALIB_START,                        // start calibration
    MSG_TYPE_CALIB_STOP,                        // stop calibration    
    MSG_TYPE_CALIB_SETTEMP,                    // set calibrator water temperature
    MSG_TYPE_CALIB_SETFLOW,                    // set calibrator cumulated flow
    MSG_TYPE_CALIB_SETHUMID,                   // set calibrator humidity mixing ratio

	// GSB
	MSG_TYPE_GSB_SETFLOW,					// set GSB flow
	MSG_TYPE_GSB_SETVALVE,					// set GSB set valves
	MSG_TYPE_GSB_SETLIGHT,					// set GSB interior light     
    MAX_MSG_TYPE }; 

enum SysParameterListEnum {                 // list of System Parameters
  
  SYS_PARAMETER_ETALON_ONLINE,
  SYS_PARAMETER_ETALON_OFFLINE_LEFT,
  SYS_PARAMETER_ETALON_OFFLINE_RIGHT,

  SYS_PARAMETER_ETALON_DITHER,

  SYS_PARAMETER_ETALON_SCAN_START,
  SYS_PARAMETER_ETALON_SCAN_STOP,
  SYS_PARAMETER_ETALON_SCAN_STEP,
  
  SYS_PARAMETER_MIRRORS_REALIGN_MINUTES,
  
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
    int64_t Value;                     // value to transmit or recieve depending on MsgType
    uint16_t Status;                    // status of message (error, success...)
}; /* ElekMessageType */

union Unsigned2SignedType {
int64_t i_signed;
uint64_t i_unsigned;
}; /* to treat unsigned variables as signed */
#endif
