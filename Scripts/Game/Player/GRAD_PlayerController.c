//------------------------------------------------------------------------------------------------
modded class SCR_PlayerController : PlayerController
{
	[Attribute(defvalue: "{257513225CC1D5FB}UI/OTF/OTF_Overlay.layout")]
	protected ResourceName m_sOverlay;
	
	[Attribute(defvalue: "{B764E0A789AF2F5F}UI/OTF/OTF_CountdownTimer.layout")]
	protected ResourceName m_sCountdownTimer;
	
	protected ref Widget m_wOverlayDisplay;
	protected ref Widget m_wCountdownDisplay;
	protected ref Widget m_wTimerDisplay;
	
	protected bool m_bCoutdownRunning = false;
	protected bool m_bTimerRunning = false;
	
	protected int m_iCountdownDuration = 0;
	protected int m_iTimerDuration = 0;
	
	protected ref GRAD_MapMarkerUI m_MapMarkerUI;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		InitMapMarkerUI();
    }
	
	//------------------------------------------------------------------------------------------------
	void InitMapMarkerUI()
	{
		if (!m_MapMarkerUI)
		{
			m_MapMarkerUI = new GRAD_MapMarkerUI();
			m_MapMarkerUI.Init();
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	void DeleteFortificationsFromInventory()
	{
		Rpc(RpcDo_Owner_DeleteFortificationsFromInventory);
	}
		
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_DeleteFortificationsFromInventory()
	{
		// executed locally on players PS_VirtualMachine

		IEntity entity = GetControlledEntity();
		
		SCR_InventoryStorageManagerComponent inventoryStorageManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		array<IEntity> items = {};
		inventoryStorageManager.GetItems(items);
		
		int count = 0;
		foreach(IEntity item : items)
		{
			// fortification items are identified by having the placementOverrideComponent
			// strictly this would also be true for the barrel and for the traffic cone
			GRAD_PlacementOverrideComponent placementOverrideComponent = GRAD_PlacementOverrideComponent.Cast(item.FindComponent(GRAD_PlacementOverrideComponent));
			if (placementOverrideComponent)
			{
				delete item;
				count++;
			}
		}
		
		Print(string.Format("OTF - %1 fortification items deleted from inventory", count), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	void InsertMarker(SCR_MapMarkerBase marker)
	{
		Rpc(RpcDo_Owner_InsertMarker, marker);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_InsertMarker(SCR_MapMarkerBase marker)
	{
		// executed locally on players PS_VirtualMachine

		SetMarker(marker);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetMarker(SCR_MapMarkerBase marker)
	{
		SCR_MapMarkerManagerComponent mapMarkerManager = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		
		SCR_MapMarkerBase newMarker = new SCR_MapMarkerBase();
		newMarker.SetType(marker.GetType());
		int worldPos[2];
		marker.GetWorldPos(worldPos);
		newMarker.SetWorldPos(worldPos[0], worldPos[1]);
		newMarker.SetMarkerConfigID(marker.GetMarkerConfigID());
		newMarker.SetCustomText(marker.GetCustomText());
		newMarker.SetColorEntry(marker.GetColorEntry());
		newMarker.SetIconEntry(marker.GetIconEntry());
		
		mapMarkerManager.InsertStaticMarker(newMarker, true, true);
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
	void AddCircleMarker(float startX, float startY, float endX, float endY)
	{
		Rpc(RpcDo_Owner_AddCircleMarker, startX, startY, endX, endY);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_AddCircleMarker(float startX, float startY, float endX, float endY)
	{
		m_MapMarkerUI.AddCircle(startX, startY, endX, endY);
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
	void ShowOverlay(string message, int duration)
	{
		Rpc(RpcDo_Owner_ShowOverlay, message, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_ShowOverlay(string message, int duration)
	{
		// executed locally on players machine
		
		// Disable countdown/timer if new overlay will be displayed
		m_bCoutdownRunning = false;
		m_bTimerRunning = false;
		
		// Remove previously displays
		RemoveOverlay();
		RemoveCountdown();
		RemoveTimer();
		
		m_wOverlayDisplay = GetGame().GetWorkspace().CreateWidgets(m_sOverlay);
		
		TextWidget textWidget = TextWidget.Cast(m_wOverlayDisplay.FindAnyWidget("Text0"));
		
		if (textWidget)
			textWidget.SetText(message);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_CREATED);
		
		GetGame().GetCallqueue().CallLater(RemoveOverlay, duration * 1000, false); // x sec later
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveOverlay()
	{
		if (m_wOverlayDisplay)
		{
			delete m_wOverlayDisplay;
		}	
	}

	//------------------------------------------------------------------------------------------------
	void ShowCountdown(int duration)
	{
		Rpc(RpcDo_Owner_ShowCountdown, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_ShowCountdown(int duration)
	{
		// executed locally on players machine

		// Disable countdown/timer if new overlay will be displayed
		m_bCoutdownRunning = false;
		m_bTimerRunning = false;
		
		// Remove previously displays
		RemoveOverlay();
		RemoveCountdown();
		RemoveTimer();
		
		m_wCountdownDisplay = GetGame().GetWorkspace().CreateWidgets(m_sCountdownTimer);
		
		m_bCoutdownRunning = true;
		
		m_iCountdownDuration = duration;
		
		UpdateCountdown();
		
		// kill every previously running process
		GetGame().GetCallqueue().Remove(UpdateCountdown);
		GetGame().GetCallqueue().Remove(UpdateTimer);
		
		GetGame().GetCallqueue().CallLater(UpdateCountdown, 1000, true); // every second
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateCountdown()
	{
		if (!m_bCoutdownRunning)
		{
			GetGame().GetCallqueue().Remove(UpdateCountdown);
			return;
		}
		
		int minutes = m_iCountdownDuration / 60;
		int seconds = m_iCountdownDuration - (minutes * 60);
		
		string minutesStr;
		string secondsStr;

		if (minutes < 10)
			minutesStr = string.Format("0%1", minutes);
		else
			minutesStr = string.Format("%1", minutes);
		
		if (seconds < 10)
			secondsStr = string.Format("0%1", seconds);
		else
			secondsStr = string.Format("%1", seconds);
		
		if (!m_wCountdownDisplay)
			return;
		
		TextWidget textWidget = TextWidget.Cast(m_wCountdownDisplay.FindAnyWidget("Text0"));
		
		if (textWidget)
			textWidget.SetText(string.Format("%1:%2", minutesStr, secondsStr));
		
		if (m_iCountdownDuration == 0)
		{
			GetGame().GetCallqueue().Remove(UpdateCountdown);
			
			m_bCoutdownRunning = false;
			
			if (textWidget)
			{
				textWidget.SetColor(new Color(1.0, 0.0, 0.0, 1.0));
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_SIREN);
			}
		}
		else
			m_iCountdownDuration--;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveCountdown()
	{
		if (m_wCountdownDisplay)
		{
			delete m_wCountdownDisplay;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowTimer()
	{
		Rpc(RpcDo_Owner_ShowTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_Owner_ShowTimer()
	{
		// executed locally on players machine

		// Disable countdown/timer if new overlay will be displayed
		m_bCoutdownRunning = false;
		m_bTimerRunning = false;
		
		// Remove previously displays
		RemoveOverlay();
		RemoveCountdown();
		RemoveTimer();
		
		m_wTimerDisplay = GetGame().GetWorkspace().CreateWidgets(m_sCountdownTimer);
		
		m_bTimerRunning = true;
		
		m_iTimerDuration = 0;
		
		UpdateTimer();
		
		// kill every previously running process
		GetGame().GetCallqueue().Remove(UpdateTimer);
		GetGame().GetCallqueue().Remove(UpdateCountdown);
		
		GetGame().GetCallqueue().CallLater(UpdateTimer, 1000, true); // every second
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTimer()
	{
		if (!m_bTimerRunning)
		{
			GetGame().GetCallqueue().Remove(UpdateTimer);
			return;
		}
		
		int minutes = m_iTimerDuration / 60;
		int seconds = m_iTimerDuration - (minutes * 60);
		
		string minutesStr;
		string secondsStr;

		if (minutes < 10)
			minutesStr = string.Format("0%1", minutes);
		else
			minutesStr = string.Format("%1", minutes);
		
		if (seconds < 10)
			secondsStr = string.Format("0%1", seconds);
		else
			secondsStr = string.Format("%1", seconds);
		
		if (!m_wTimerDisplay)
			return;
		
		TextWidget textWidget = TextWidget.Cast(m_wTimerDisplay.FindAnyWidget("Text0"));
		
		if (textWidget)
			textWidget.SetText(string.Format("%1:%2", minutesStr, secondsStr));

		m_iTimerDuration++;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveTimer()
	{
		if (m_wTimerDisplay)
		{
			delete m_wTimerDisplay;
		}	
	}

	//------------------------------------------------------------------------------------------------
	int GetTimerDuration()
	{
		return m_iTimerDuration;
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
			Print(string.Format("OTF - Player with ID %1 successfully teleported to position %2 - Deprecated", GetPlayerId(), pos), LogLevel.NORMAL);
		else
			Print(string.Format("OTF - Player with ID %1 NOT successfully teleported to position %2 - Deprecated", GetPlayerId(), pos), LogLevel.WARNING);
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
		{
			Print(string.Format("OTF - Player entity missing; skipping teleport"), LogLevel.WARNING);
			return;
		}
		
		Print(string.Format("OTF - Player with ID %1 has position %2", playerId, playerEntity.GetOrigin()), LogLevel.NORMAL);
		
		bool teleportSuccessful = false;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		vector newWorldPos;
		
		int teleportAttemptsCounter = 0;
		
		while (!teleportSuccessful)
		{
			if (teleportAttemptsCounter >= 5)
			{
				ShowHint("Teleport not successful. Contact your Game Master for manual teleport.", "On The Fly", 10, false);
				Print(string.Format("OTF - Player with ID %1 NOT successfully teleported to position %2", GetPlayerId(), newWorldPos), LogLevel.NORMAL);
				break;
			}
			
			mapPos[0] = mapPos[0] + Math.RandomFloat(-30, 30);
			mapPos[1] = mapPos[1] + Math.RandomFloat(-30, 30);
			
			newWorldPos = otfManager.MapPosToWorldPos(mapPos);
			
			teleportSuccessful = SCR_Global.TeleportLocalPlayer(newWorldPos, SCR_EPlayerTeleportedReason.DEFAULT);
			
			if (teleportSuccessful)
				Print(string.Format("OTF - Player with ID %1 successfully teleported to position %2", GetPlayerId(), newWorldPos), LogLevel.NORMAL);
			
			teleportAttemptsCounter++;
		}
	}
};
