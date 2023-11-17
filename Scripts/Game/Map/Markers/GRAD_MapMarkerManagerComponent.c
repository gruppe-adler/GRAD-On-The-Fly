//------------------------------------------------------------------------------------------------
//! Map marker manager, keeps array of markers and takes care of synchronization
//! Attached to GameMode entity
modded class SCR_MapMarkerManagerComponent : SCR_BaseGameModeComponent
{	
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! RPC result of marker add broadcast
	override void OnAddSynchedMarker(SCR_MapMarkerBase marker)
	{
		super.OnAddSynchedMarker(marker);
		
		// this function is executed on all machines (server and clients) 
		// so we should filter on server
		
		if (!Replication.IsServer())
			return;
		
		if (marker.GetType() == SCR_EMapMarkerType.PLACED_CUSTOM)
		{
			int markerPos[2];
			marker.GetWorldPos(markerPos);
			string markerText = marker.GetCustomText();
			Print(string.Format("Custom Marker '%1' placed at pos %2", markerText, markerPos), LogLevel.NORMAL);
			
			markerText.ToLower();
			if (markerText != "teleport")
				return;
			
			array<int> playerIds = {};
			GetGame().GetPlayerManager().GetPlayers(playerIds);
			foreach (int playerId : playerIds)
			{
				BaseWorld world = GetGame().GetWorld();
				IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				
				Print(string.Format("Player with ID %1 has position %2", playerId, playerEntity.GetOrigin()), LogLevel.NORMAL);
				
				vector worldPos = {markerPos[0], 500, markerPos[1]}; // start tracing in 500m height

				vector outDir = {0, -1, 0}; // downward direction
				outDir *= 1000; // trace for 1000 meters
				
		        autoptr TraceParam trace = new TraceParam();
		        trace.Start = worldPos;
		        trace.End = worldPos + outDir;
		        trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
		        trace.LayerMask = TRACE_LAYER_CAMERA;
   
		        float traceDis = world.TraceMove(trace, null);
		        
				vector newWorldPos = worldPos + outDir * traceDis;
				
				SCR_Global.TeleportPlayer(playerId, newWorldPos, SCR_EPlayerTeleportedReason.DEFAULT);
				
				Print(string.Format("Player with ID %1 teleported to position %2", playerId, newWorldPos), LogLevel.NORMAL);
			}
		}
	}
}