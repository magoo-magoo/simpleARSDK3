#include "JumpingNetworkManager.hh"

int	main()
{
  JumpingNetworkManager netManager;

  if (netManager.ardiscoveryConnect())
    {
      return 1;
    }

  if (netManager.startNetwork())
    {
      return 1;
    }
  //netManager.sendPilotingPosture(ARCOMMANDS_JUMPINGSUMO_PILOTING_POSTURE_TYPE_KICKER);
  //netManager.sendPilotingPosture(ARCOMMANDS_JUMPINGSUMO_PILOTING_POSTURE_TYPE_JUMPER);
  netManager.sendPilotingPosture(ARCOMMANDS_JUMPINGSUMO_PILOTING_POSTURE_TYPE_STANDING);


  return (0);
}
