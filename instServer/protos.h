/* cfgnear.c */
long fcopy(char *dest, char *source);
int input_config(char *filename, struct Config_Tag configs[], char *header);
int update_config(char *filename, struct Config_Tag configs[], char *header);
/* logerror.c */
void LogError(char *Message);
/* main.c */
int DoMotor(struct ClientMessageType *ptrClientMessage);
int DoValve(struct ClientMessageType *ptrClientMessage);
int FindFkt(struct InterprFktType *ptr1, struct InterprFktType *ptr2);
int ParseMsg(char *ptrLine, unsigned nbytes, struct ActiveClientStructType *ptrActiveClientStruct);
int InitTCPListener(int portnum);
void InitClientAccessList(struct ClientRightAscType *ptrClientRightAscList, struct ClientListType **pptrClientRightList);
void ReleaseClientAccessList(struct ClientListType *ptrClientRightList);
struct ClientListType *ValidateAddress(struct sockaddr_in *ptrRemoteAddr, struct ClientListType *ptrClientRightList);
int InitActiveClientStruct(struct ActiveClientStructType *ActiveClientStruct);
void ReleaseActiveClientStruct(struct ActiveClientStructType *ActiveClientStruct);
int AddClientToList(struct ActiveClientStructType *ptrActiveClientStruct, struct ClientListType *ptrNewClient, int newfd);
int RemoveClientFromList(struct ActiveClientStructType *ptrActiveClientStruct, int fd);
int main(int argc, char *argv[]);
