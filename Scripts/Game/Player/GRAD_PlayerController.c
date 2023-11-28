//------------------------------------------------------------------------------------------------
modded class SCR_PlayerController : PlayerController
{
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
		
		// Open Map before creating marker to avoid script error
		// No luck with opening the map
		/*
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;
		SCR_MapEntity m_MapEntity = SCR_MapEntity.GetMapInstance();
		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.FULLSCREEN, configComp.GetGadgetMapConfig(), GetRootWidget());
		m_MapEntity.OpenMap(mapConfigFullscreen);
		*/
		
		/*
		SCR_MapEditorComponent mapEditorComponent = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		if (mapEditorComponent)
		{
			mapEditorComponent.ToggleMap();
		}
		*/
		
		// Creating the marker will trow an error om SCR_MapMarkerBase because it's meant to only create 
		// markers while the map is open. I think this error can be ignored	

		// duplicating instead of assigning marker because with that the marker would change it's 
		// faction and becomes invisible for opfor
			
		SCR_MapMarkerBase localMarker = new SCR_MapMarkerBase();
		localMarker.SetType(marker.GetType());
		int worldPos[2];
		marker.GetWorldPos(worldPos);
		localMarker.SetWorldPos(worldPos[0], worldPos[1]);
		localMarker.SetMarkerConfigID(marker.GetMarkerConfigID());
		localMarker.SetCustomText(marker.GetCustomText());
		localMarker.SetColorEntry(marker.GetColorEntry());
		localMarker.SetIconEntry(marker.GetIconEntry());
		
		mapMarkerManager.InsertLocalMarker(localMarker);
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
			Print(string.Format("OTF - Player with ID %1 successfully teleported to position %2", GetPlayerId(), pos), LogLevel.NORMAL);
		else
			Print(string.Format("OTF - Player with ID %1 NOT successfully teleported to position %2", GetPlayerId(), pos), LogLevel.WARNING);
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
		
		IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		
		if (!playerEntity)
			return;
		
		Print(string.Format("OTF - Player with ID %1 has position %2", playerId, playerEntity.GetOrigin()), LogLevel.NORMAL);
		
		bool teleportSuccessful = false;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		vector newWorldPos;
		
		while (!teleportSuccessful)
		{
			mapPos[0] = mapPos[0] + Math.RandomFloat(-3, 3);
			mapPos[1] = mapPos[1] + Math.RandomFloat(-3, 3);
			
			newWorldPos = otfManager.MapPosToWorldPos(mapPos);
			
			teleportSuccessful = SCR_Global.TeleportLocalPlayer(newWorldPos, SCR_EPlayerTeleportedReason.DEFAULT);
		}
		
		Print(string.Format("OTF - Player with ID %1 successfully teleported to position %2", GetPlayerId(), newWorldPos), LogLevel.NORMAL);
	}
};
