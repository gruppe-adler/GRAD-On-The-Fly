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
			int markerOwnerId = marker.GetMarkerOwnerID();
			IEntity markerOwnerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(markerOwnerId);
			
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			SCR_RespawnSystemComponent respawnComponent = gameMode.GetRespawnSystemComponent();
			Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(markerOwnerId);
			string markerOwnerFactionName = markerOwnerFaction.GetFactionName();
        
			
			Print(string.Format("Custom Marker '%1' placed at pos %2", markerText, markerPos), LogLevel.NORMAL);
			
			markerText.ToLower();
			if (markerText != "teleport")
				return;
			
			
		    array<int> playerIds = {};
			GetGame().GetPlayerManager().GetPlayers(playerIds);
		
		    SCR_EntityCatalog globalCatalog;
		
		    foreach (int playerId : playerIds)
		    {
				
				BaseWorld world = GetGame().GetWorld();
				
				IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
				string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
				Faction currentPlayerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
				string currentPlayerFactionName = currentPlayerFaction.GetFactionName();
				
				if (markerOwnerFactionName != currentPlayerFactionName)
					Print(string.Format("%1 denied teleporting as faction %2 differs from teleporting faction %3", playerName, currentPlayerFaction, markerOwnerFaction, LogLevel.NORMAL));
					return;
				
				
				Print(string.Format("Player has position %1", playerEntity.GetOrigin()), LogLevel.NORMAL);
				
				
				vector newWorldPos;
				
				bool teleportSuccessful = false;
				
				vector worldPos = {markerPos[0], 500, markerPos[1]}; // start tracing in 500m height
				
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

				Print(string.Format("%1 in faction %2 teleported to position %3", playerName, currentPlayerFaction, newWorldPos), LogLevel.NORMAL);
			}
		}
	}
}