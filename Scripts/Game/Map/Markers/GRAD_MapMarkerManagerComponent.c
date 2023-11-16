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
			Print(string.Format("Custom Marker '%1' placed at pos %2", markerText, markerPos));
		}
	}
	
}