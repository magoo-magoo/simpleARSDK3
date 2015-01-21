
#include "JumpingNetworkManager.hh"

const std::string JumpingNetworkManager::TAG = "SimpleSumo";
const std::string JumpingNetworkManager::JS_IP_ADDRESS = "192.168.2.1";


JumpingNetworkManager::JumpingNetworkManager()
{
  this->alManager_ = nullptr;
  this->netManager_ = nullptr;
  this->rxThread_ = nullptr;
  this->txThread_ = nullptr;
  this->d2cPort_ = JS_D2C_PORT;
  this->c2dPort_ = JS_C2D_PORT; //this->c2dPort = 0; // should be read from js

  this->c2dParams_[0].ID = 10;
  this->c2dParams_[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
  this->c2dParams_[0].sendingWaitTimeMs = 5;
  this->c2dParams_[0].ackTimeoutMs = -1;
  this->c2dParams_[0].numberOfRetry = -1;
  this->c2dParams_[0].numberOfCell = 10;
  this->c2dParams_[0].dataCopyMaxSize = 128;
  this->c2dParams_[0].isOverwriting = 0;

  this->c2dParams_[1].ID = 11;
  this->c2dParams_[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
  this->c2dParams_[1].sendingWaitTimeMs = 20;
  this->c2dParams_[1].ackTimeoutMs = 500;
  this->c2dParams_[1].numberOfRetry = 3;
  this->c2dParams_[1].numberOfCell = 20;
  this->c2dParams_[1].dataCopyMaxSize = 128;
  this->c2dParams_[1].isOverwriting = 0;

  this->d2cParams_[0].ID = ((ARNETWORKAL_MANAGER_WIFI_ID_MAX / 2) - 1);
  this->d2cParams_[0].dataType = ARNETWORKAL_FRAME_TYPE_DATA;
  this->d2cParams_[0].sendingWaitTimeMs = 20;
  this->d2cParams_[0].ackTimeoutMs = -1;
  this->d2cParams_[0].numberOfRetry = -1;
  this->d2cParams_[0].numberOfCell = 10;
  this->d2cParams_[0].dataCopyMaxSize = 128;
  this->d2cParams_[0].isOverwriting = 0;

  this->d2cParams_[1].ID = ((ARNETWORKAL_MANAGER_WIFI_ID_MAX / 2) - 2);
  this->d2cParams_[1].dataType = ARNETWORKAL_FRAME_TYPE_DATA_WITH_ACK;
  this->d2cParams_[1].sendingWaitTimeMs = 20;
  this->d2cParams_[1].ackTimeoutMs = 500;
  this->d2cParams_[1].numberOfRetry = 3;
  this->d2cParams_[1].numberOfCell = 20;
  this->d2cParams_[1].dataCopyMaxSize = 128;
  this->d2cParams_[1].isOverwriting = 0;
}

JumpingNetworkManager::~JumpingNetworkManager()
{
  this->stopNetwork();
}


int JumpingNetworkManager::ardiscoveryConnect()
{
    int failed = 0;

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG.c_str(), "- ARDiscovery Connection");

    eARDISCOVERY_ERROR err = ARDISCOVERY_OK;
    ARDISCOVERY_Connection_ConnectionData_t *discoveryData = ARDISCOVERY_Connection_New(ARDISCOVERY_Connection_SendJsonCallback,
													ARDISCOVERY_Connection_ReceiveJsonCallback,
													this,
													&err);
    if (discoveryData == nullptr || err != ARDISCOVERY_OK)
    {
      ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "Error while creating discoveryData : %s", ARDISCOVERY_Error_ToString(err));
        failed = 1;
    }

    if (!failed)
    {
      err = ARDISCOVERY_Connection_ControllerConnection(discoveryData, JS_DISCOVERY_PORT, JS_IP_ADDRESS.c_str());
        if (err != ARDISCOVERY_OK)
        {
	  ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "Error while opening discovery connection : %s", ARDISCOVERY_Error_ToString(err));
            failed = 1;
        }
    }

    ARDISCOVERY_Connection_Delete(&discoveryData);

    return failed;
}

eARDISCOVERY_ERROR JumpingNetworkManager::ARDISCOVERY_Connection_SendJsonCallback (uint8_t *dataTx, uint32_t *dataTxSize, void *customData)
{
    JumpingNetworkManager *jsManager = (JumpingNetworkManager*)customData;
    eARDISCOVERY_ERROR err = ARDISCOVERY_OK;

    if ((dataTx != nullptr) && (dataTxSize != nullptr) && (jsManager != nullptr))
    {
        *dataTxSize = static_cast<unsigned int>(sprintf((char *)dataTx, "{ \"%s\": %d,\n \"%s\": \"%s\",\n \"%s\": \"%s\" }",
                              ARDISCOVERY_CONNECTION_JSON_D2CPORT_KEY, jsManager->d2cPort_,
                              ARDISCOVERY_CONNECTION_JSON_CONTROLLER_NAME_KEY, "name",
							ARDISCOVERY_CONNECTION_JSON_CONTROLLER_TYPE_KEY, "type")) + 1;
    }
    else
    {
        err = ARDISCOVERY_ERROR;
    }

    return err;
}

eARDISCOVERY_ERROR JumpingNetworkManager::ARDISCOVERY_Connection_ReceiveJsonCallback (uint8_t *dataRx, uint32_t dataRxSize, char *, void *customData)
{
    JumpingNetworkManager *jsManager = (JumpingNetworkManager*)customData;
    eARDISCOVERY_ERROR err = ARDISCOVERY_OK;

    if ((dataRx != nullptr) && (dataRxSize != 0) && (jsManager != nullptr))
    {
      char *json = new char[dataRxSize + 1];

      ::strncpy(json, (char *)dataRx, dataRxSize);
        json[dataRxSize] = '\0';

	ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG.c_str(), "    - ReceiveJson:%s ", json);

        //normally c2dPort should be read from the json here.

        delete json;
    }
    else
    {
        err = ARDISCOVERY_ERROR;
    }

    return err;
}


int JumpingNetworkManager::startNetwork()
{
    int failed = 0;
    eARNETWORK_ERROR netError = ARNETWORK_OK;
    eARNETWORKAL_ERROR netAlError = ARNETWORKAL_OK;
    int pingDelay = 0; // 0 means default, -1 means no ping

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG.c_str(), "- Start ARNetwork");

    // Create the ARNetworkALManager
    this->alManager_ = ARNETWORKAL_Manager_New(&netAlError);
    if (netAlError != ARNETWORKAL_OK)
    {
        failed = 1;
    }

    if (!failed)
    {
        // Initilize the ARNetworkALManager
        netAlError = ARNETWORKAL_Manager_InitWifiNetwork(this->alManager_, JS_IP_ADDRESS.c_str(), JS_C2D_PORT, JS_D2C_PORT, 1);
        if (netAlError != ARNETWORKAL_OK)
        {
            failed = 1;
        }
    }

    if (!failed)
    {
        // Create the ARNetworkManager.
        this->netManager_ = ARNETWORK_Manager_New(this->alManager_, numC2dParams_, c2dParams_, numD2cParams_, d2cParams_, pingDelay, onDisconnectNetwork, this, &netError);
        if (netError != ARNETWORK_OK)
        {
            failed = 1;
        }
    }

    if (!failed)
    {
        // Create and start Tx and Rx threads.
        if (ARSAL_Thread_Create(&(this->rxThread_), ARNETWORK_Manager_ReceivingThreadRun, this->netManager_) != 0)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "Creation of Rx thread failed.");
            failed = 1;
        }

        if (ARSAL_Thread_Create(&(this->txThread_), ARNETWORK_Manager_SendingThreadRun, this->netManager_) != 0)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "Creation of Tx thread failed.");
            failed = 1;
        }
    }

    // Print net error
    if (!failed)
    {
        if (netAlError != ARNETWORKAL_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "ARNetWorkAL Error : %s", ARNETWORKAL_Error_ToString(netAlError));
        }

        if (netError != ARNETWORK_OK)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, TAG.c_str(), "ARNetWork Error : %s", ARNETWORK_Error_ToString(netError));
        }
    }

    return failed;
}


void JumpingNetworkManager::stopNetwork()
{
    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG.c_str(), "- Stop ARNetwork");

    // ARNetwork cleanup
    if (this->netManager_ != nullptr)
    {
        ARNETWORK_Manager_Stop(this->netManager_);
        if (this->rxThread_ != nullptr)
        {
            ARSAL_Thread_Join(this->rxThread_, nullptr);
            ARSAL_Thread_Destroy(&(this->rxThread_));
            this->rxThread_ = nullptr;
        }

        if (this->txThread_ != nullptr)
        {
            ARSAL_Thread_Join(this->txThread_, nullptr);
            ARSAL_Thread_Destroy(&(this->txThread_));
            this->txThread_ = nullptr;
        }
    }

    if (this->alManager_ != nullptr)
    {
        ARNETWORKAL_Manager_Unlock(this->alManager_);

        ARNETWORKAL_Manager_CloseWifiNetwork(this->alManager_);
    }

    ARNETWORK_Manager_Delete(&(this->netManager_));
    ARNETWORKAL_Manager_Delete(&(this->alManager_));
}

void JumpingNetworkManager::onDisconnectNetwork(ARNETWORK_Manager_t *, ARNETWORKAL_Manager_t *, void *)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG.c_str(), "onDisconnectNetwork ...");
}

int JumpingNetworkManager::sendPilotingPosture(eARCOMMANDS_JUMPINGSUMO_PILOTING_POSTURE_TYPE type)
{
    int sentStatus = 1;
    u_int8_t cmdBuffer[128];
    int32_t cmdSize = 0;
    eARCOMMANDS_GENERATOR_ERROR cmdError;
    eARNETWORK_ERROR netError = ARNETWORK_ERROR;

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG.c_str(), "- Send Piloting Posture %d", type);

    // Send Posture command
    cmdError = ARCOMMANDS_Generator_GenerateJumpingSumoPilotingPosture(cmdBuffer, sizeof(cmdBuffer), &cmdSize, type);
    if (cmdError == ARCOMMANDS_GENERATOR_OK)
    {
        netError = ARNETWORK_Manager_SendData(this->netManager_, JS_NET_CD_ACK_ID, cmdBuffer, cmdSize, nullptr, &(arnetworkCmdCallback), 1);
    }

    if ((cmdError != ARCOMMANDS_GENERATOR_OK) || (netError != ARNETWORK_OK))
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG.c_str(), "Failed to send Posture command. cmdError:%d netError:%s", cmdError, ARNETWORK_Error_ToString(netError));
        sentStatus = 0;
    }

    return sentStatus;
}

int JumpingNetworkManager::sendPilotingPCMD(unsigned char flag, char speed, char turn)
{
    int sentStatus = 1;
    u_int8_t cmdBuffer[128];
    int32_t cmdSize = 0;
    eARCOMMANDS_GENERATOR_ERROR cmdError;
    eARNETWORK_ERROR netError = ARNETWORK_ERROR;

    ARSAL_PRINT(ARSAL_PRINT_INFO, TAG.c_str(),
		"- Send Piloting PCMD flag: %d speed: %d turn: %d", flag, speed, turn);

    // Send Posture command
    cmdError = ARCOMMANDS_Generator_GenerateJumpingSumoPilotingPCMD(cmdBuffer,
								    sizeof(cmdBuffer),
								    &cmdSize,
								    flag, speed, turn);
    if (cmdError == ARCOMMANDS_GENERATOR_OK)
    {
        netError = ARNETWORK_Manager_SendData(this->netManager_, JS_NET_CD_ACK_ID, cmdBuffer, cmdSize, nullptr, &(arnetworkCmdCallback), 1);
    }

    if ((cmdError != ARCOMMANDS_GENERATOR_OK) || (netError != ARNETWORK_OK))
    {
        ARSAL_PRINT(ARSAL_PRINT_WARNING, TAG.c_str(), "Failed to send Posture command. cmdError:%d netError:%s", cmdError, ARNETWORK_Error_ToString(netError));
        sentStatus = 0;
    }

    return sentStatus;
}


eARNETWORK_MANAGER_CALLBACK_RETURN JumpingNetworkManager::arnetworkCmdCallback(int buffer_id, uint8_t *, void *, eARNETWORK_MANAGER_CALLBACK_STATUS cause)
{
    eARNETWORK_MANAGER_CALLBACK_RETURN retval = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;

    ARSAL_PRINT(ARSAL_PRINT_DEBUG, TAG.c_str(), "    - arnetworkCmdCallback %d, cause:%d ", buffer_id, cause);

    if (cause == ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT)
    {
        retval = ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP;
    }

    return retval;
}
