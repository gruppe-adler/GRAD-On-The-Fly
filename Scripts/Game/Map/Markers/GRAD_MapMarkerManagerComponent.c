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
		string markerOwnerFactionName = markerOwnerFaction.GetFactionKey();
		
		int markerPos[2];
		marker.GetWorldPos(markerPos);
		
		string markerText = marker.GetCustomText();
		Print(string.Format("Custom Marker '%1' placed at pos %2 by faction %3", markerText, markerPos, markerOwnerFactionName), LogLevel.NORMAL);
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		markerText.ToLower();
		
		// allow debug spawn without peer tool
		if (markerText == "debug")
		{
				otfManager.TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFactionName, markerPos, true);
				otfManager.NotifyOpposingFaction(markerOwnerFaction, marker);
				return;
		}
		
		// manage opfor marker placement		
		if (markerText == "opfor")
		{
			if (otfManager.OpforSpawnDone()) {
				otfManager.NotifyCantTeleportTwice(markerOwnerFaction);
				return;
			}
			if (markerOwnerFactionName == "USSR") {	
				otfManager.TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFactionName, markerPos, false);
				otfManager.NotifyOpposingFaction(markerOwnerFaction, marker);
				return;
			}
			if (markerOwnerFactionName == "US") {
				otfManager.NotifyCantTeleportThisFaction(markerOwnerFaction);
				return;
			}
		}
		
		// manage blufor marker placement
		if (markerText == "blufor")
		{
			if (otfManager.OpforSpawnDone()) {
				if (markerOwnerFactionName == "US") {
					if (otfManager.BluforSpawnDone()) {
						otfManager.NotifyCantTeleportTwice(markerOwnerFaction);
						return;
					}
					otfManager.TeleportFactionToMapPos(markerOwnerFaction, markerOwnerFactionName, markerPos, false);
				} else {
					otfManager.NotifyCantTeleportThisFaction(markerOwnerFaction);
				}
			} else {
				if (markerOwnerFactionName == "US") {
					otfManager.NotifyCantTeleportYet(markerOwnerFaction)
				} else {
					otfManager.NotifyCantTeleportThisFaction(markerOwnerFaction);
				}
			}
		}
	}
}