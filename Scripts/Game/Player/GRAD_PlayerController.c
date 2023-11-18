//------------------------------------------------------------------------------------------------
modded class SCR_PlayerController : PlayerController
{
	bool otfTeleported = false;

	//------------------------------------------------------------------------------------------------
	void InsertLocalMarker(SCR_MapMarkerBase marker)
	{
		Rpc(RpcDo_Owner_InsertLocalMarker, marker);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_InsertLocalMarker(SCR_MapMarkerBase marker)
	{
		// executed locally on players machine
		
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		mapMarkerManager.InsertLocalMarker(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowHint(string message, string title, int duration, bool isSilent)
	{
		Rpc(RpcDo_Owner_ShowHint, message, title, duration, isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_ShowHint(string message, string title, int duration, bool isSilent)
	{
		// executed locally on players machine
		
		SCR_HintManagerComponent.GetInstance().ShowCustomHint(message, title, duration, isSilent);
	}
		
	//------------------------------------------------------------------------------------------------
	void TeleportPlayer(vector pos)
	{
		Rpc(RpcDo_Owner_TeleportPlayer, pos);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_TeleportPlayer(vector pos)
	{
		// executed locally on players machine
		
		if(SCR_Global.TeleportLocalPlayer(pos, SCR_EPlayerTeleportedReason.DEFAULT))
			Print(string.Format("Player with ID %1 successfully teleported to position %2", GetPlayerId(), pos), LogLevel.NORMAL);
		else
			Print(string.Format("Player with ID %1 NOT successfully teleported to position %2", GetPlayerId(), pos), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	void TeleportPlayerToMapPos(int playerId, int mapPos[2])
	{
		Rpc(Rpc_Do_Owner_TeleportPlayerToMapPos, playerId, mapPos);
	}
		
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void Rpc_Do_Owner_TeleportPlayerToMapPos(int playerId, int mapPos[2])
	{
		// executed locally on players machine
		
		if (otfTeleported)
			return;
		
		BaseWorld world = GetGame().GetWorld();
		IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		
		if (!playerEntity)
			return;
		
		Print(string.Format("Player with ID %1 has position %2", playerId, playerEntity.GetOrigin()), LogLevel.NORMAL);
		
		bool teleportSuccessful = false;
		
		vector worldPos = {mapPos[0], 500, mapPos[1]}; // start tracing in 500m height
		
		vector newWorldPos;
		
		while (!teleportSuccessful)
		{
			worldPos[0] = worldPos[0] + Math.RandomFloat(-3, 3);
			worldPos[2] = worldPos[2] + Math.RandomFloat(-3, 3);
				
			vector outDir = {0, -1, 0}; // downward direction
			outDir *= 1000; // trace for 1000 meters
			
	        autoptr TraceParam trace = new TraceParam();
	        trace.Start = worldPos;
	        trace.End = worldPos + outDir;
	        trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
	        trace.LayerMask = TRACE_LAYER_CAMERA;
  
	        float traceDis = world.TraceMove(trace, null);
	        
			newWorldPos = worldPos + outDir * traceDis;
			
			teleportSuccessful = SCR_Global.TeleportLocalPlayer(newWorldPos, SCR_EPlayerTeleportedReason.DEFAULT);
		}
		
		otfTeleported = true;
		
		Print(string.Format("Player with ID %1 successfully teleported to position %2", GetPlayerId(), newWorldPos), LogLevel.NORMAL);
	}
};
