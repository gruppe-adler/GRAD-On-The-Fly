//------------------------------------------------------------------------------------------------
//! Map marker manager, keeps array of markers and takes care of synchronization
//! Attached to GameMode entity
modded class SCR_MapMarkerManagerComponent : SCR_BaseGameModeComponent
{
	bool teleported = false;
	
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! RPC result of marker add broadcast
	override void OnAddSynchedMarker(SCR_MapMarkerBase marker)
	{
		super.OnAddSynchedMarker(marker);
		
		// this function is executed on all machines (server and clients) 
		
		if (teleported)
			return;
		
		
		if (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM)
			return;
		
	
		int markerOwnerId = marker.GetMarkerOwnerID();
		Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(markerOwnerId);
		Faction localPlayerFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		Print(string.Format("local player faction %1 - marker owner faction %2", localPlayerFaction, markerOwnerFaction), LogLevel.NORMAL);
		
		if (localPlayerFaction != markerOwnerFaction)
			return;
		
		int markerPos[2];
		marker.GetWorldPos(markerPos);
		string markerText = marker.GetCustomText();
		Print(string.Format("Custom Marker '%1' placed at pos %2", markerText, markerPos), LogLevel.NORMAL);
		
		markerText.ToLower();
		if (markerText != "teleport")
			return;
		
		int playerId = GetGame().GetPlayerController().GetPlayerId();
		
		TeleportPlayer(playerId, markerPos);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TeleportPlayer(int playerId, int mapPos[2])
	{
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
			
			teleportSuccessful = SCR_Global.TeleportPlayer(playerId, newWorldPos, SCR_EPlayerTeleportedReason.DEFAULT);
		}
		
		teleported = true;
		
		Print(string.Format("Player with ID %1 teleported to position %2", playerId, newWorldPos), LogLevel.NORMAL);
	}
}