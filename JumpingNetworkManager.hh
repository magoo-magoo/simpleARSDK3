#ifndef JUMPINGNETWORKMANAGER_H_
# define JUMPINGNETWORKMANAGER_H_

# include <memory>

extern "C" {
# include <libARSAL/ARSAL.h>
# include <libARSAL/ARSAL_Print.h>
# include <libARNetwork/ARNetwork.h>
# include <libARNetworkAL/ARNetworkAL.h>
# include <libARCommands/ARCommands.h>
# include <libARDiscovery/ARDiscovery.h>
}

#define TAG "JumpingSumoChangePosture"
#define JS_IP_ADDRESS "192.168.2.1"
#define JS_DISCOVERY_PORT 44444
#define JS_C2D_PORT 54321 // should be read from Json
#define JS_D2C_PORT 43210

#define JS_NET_CD_NONACK_ID 10
#define JS_NET_CD_ACK_ID 11
#define JS_NET_CD_VIDEO_ACK_ID 13
#define JS_NET_DC_NONACK_ID 127
#define JS_NET_DC_ACK_ID 126

typedef struct ARNETWORK_Manager_t ARNETWORK_Manager_t;

class JumpingNetworkManager
{
public:

JumpingNetworkManager();
virtual ~JumpingNetworkManager();

  int sendPilotingPosture(eARCOMMANDS_JUMPINGSUMO_PILOTING_POSTURE_TYPE type);

  int ardiscoveryConnect();
  int startNetwork();

  void stopNetwork();

private:
  JumpingNetworkManager(const JumpingNetworkManager &) = delete;
  JumpingNetworkManager &operator=(const JumpingNetworkManager &) = delete;
  JumpingNetworkManager(JumpingNetworkManager &&) = delete;
  JumpingNetworkManager &operator=(JumpingNetworkManager &&) = delete;

  // member functions

  static eARDISCOVERY_ERROR ARDISCOVERY_Connection_SendJsonCallback (uint8_t *dataTx, uint32_t *dataTxSize, void *customData);
  static eARDISCOVERY_ERROR ARDISCOVERY_Connection_ReceiveJsonCallback (uint8_t *dataRx, uint32_t dataRxSize, char *ip, void *customData);
  static void onDisconnectNetwork(ARNETWORK_Manager_t *manager, ARNETWORKAL_Manager_t *alManager, void *customData);

  static eARNETWORK_MANAGER_CALLBACK_RETURN arnetworkCmdCallback(int buffer_id, uint8_t *data, void *custom, eARNETWORK_MANAGER_CALLBACK_STATUS cause);

  // data members
  ARNETWORKAL_Manager_t* alManager_;
  ARNETWORK_Manager_t*   netManager_;

  ARSAL_Thread_t	rxThread_;
  ARSAL_Thread_t	txThread_;
  int			d2cPort_;
  int			c2dPort_;

  ARNETWORK_IOBufferParam_t c2dParams_[2];
  ARNETWORK_IOBufferParam_t d2cParams_[2];

  const unsigned int numC2dParams_ = sizeof(c2dParams_) / sizeof(ARNETWORK_IOBufferParam_t);
  const unsigned int numD2cParams_ = sizeof(d2cParams_) / sizeof(ARNETWORK_IOBufferParam_t);

};



#endif /* !JUMPINGNETWORKMANAGER_H_ */
