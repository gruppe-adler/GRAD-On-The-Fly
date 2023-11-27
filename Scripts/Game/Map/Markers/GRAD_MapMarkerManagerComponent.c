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
		
		// this function is executed on all machines (server and clients) but filtered to server
		
		if (!Replication.IsServer())
			return;
		
		if (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM)
			return;
		
		int markerOwnerId = marker.GetMarkerOwnerID();
		Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(markerOwnerId);
		
		int markerPos[2];
		marker.GetWorldPos(markerPos);
		
		string markerText = marker.GetCustomText();
		Print(string.Format("OTF - Custom Marker '%1' placed at pos %2 by faction %3", markerText, markerPos, markerOwnerFaction.GetFactionKey()), LogLevel.NORMAL);
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		markerText.ToLower();
		
		switch (markerText)
		{
			case "debug":
				otfManager.DebugMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
	
			case "opfor":
				otfManager.OpforMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
			
			case "blufor":
				otfManager.BluforMarkerCreated(marker, markerPos, markerOwnerFaction);
				break;
	
			default:
				Print("OTF - Non-relevant marker placed.");
				break;
		}
	}
}