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
		
		// Open map before creating marker
		ToggleMap(true);
		
		// create marker
		GetGame().GetCallqueue().CallLater(SetMarker, 3000, false, marker);
	}

	//------------------------------------------------------------------------------------------------
	void SetMarker(SCR_MapMarkerBase marker)
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
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
	void ToggleMap(bool open)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc) return;
		
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!ch) return;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(ch.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager) return;
		
		if (!gadgetManager.GetGadgetByType(EGadgetType.MAP)) return;
		
		IEntity mapEntity = gadgetManager.GetGadgetByType(EGadgetType.MAP);
		
		if (open)
			gadgetManager.SetGadgetMode(mapEntity, EGadgetMode.IN_HAND, true);
		else
			gadgetManager.SetGadgetMode(mapEntity, EGadgetMode.IN_SLOT, false);
	}

	//------------------------------------------------------------------------------------------------
	void DrawCircleOnMap()
	{
		// DO NOT USE THIS FUNCTION
		// STILL UNDER DEVELOPMENT
		// ONLY WORKS IF MAP IS OPEN
		// NEEDS TO BE REDRAWN ON EVERY CHANGE OF THE MAP
		// DIMENSIONS ARE NOT PERFECT
		
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		CanvasWidget mapWidget = mapEntity.GetMapWidget();
		
		Widget mapFrame = mapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		
		Widget line = GetGame().GetWorkspace().CreateWidgets("{4B995CEAA55BBECC}UI/Layouts/Map/MapDrawCircle.layout", mapFrame);
		ImageWidget lineImage = ImageWidget.Cast(line.FindAnyWidget("DrawCircleImage"));
		
		// 4438,8662
		int mapPosX = 4438;
		int mapPosY = 8662;
		int mapRadius = 1000;
		
		int screenPosX, screenPosY, screenRadius;
		
		mapEntity.WorldToScreen(mapPosX, mapPosY, screenPosX, screenPosY, true);
		mapEntity.WorldToScreen(0, mapRadius, screenRadius, screenRadius, true);
		
		vector circleVector = mapEntity.GetMapWidget().SizeToPixels({ 0, screenRadius * 2});
		lineImage.SetSize(GetGame().GetWorkspace().DPIUnscale(circleVector.Length()), GetGame().GetWorkspace().DPIUnscale(circleVector.Length()));
		
		Print(screenRadius);
		
		FrameSlot.SetPos(line, GetGame().GetWorkspace().DPIUnscale(screenPosX - circleVector.Length()), GetGame().GetWorkspace().DPIUnscale(screenPosY));	// needs unscaled coords
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
		
		// Close map before creating marker
		ToggleMap(false);
		
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
