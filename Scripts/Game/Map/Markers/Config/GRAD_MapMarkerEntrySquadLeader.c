[BaseContainerProps(), SCR_MapMarkerTitle()]
modded class SCR_MapMarkerEntrySquadLeader: SCR_MapMarkerEntryDynamic
{
	//------------------------------------------------------------------------------------------------
	// Overriding the override from Lobby mod to reanable Squad Leader Map Marker
	override void RegisterMarker(SCR_MapMarkerSquadLeader marker, SCR_AIGroup group)
	{
		m_mGroupMarkers.Insert(group, marker);
	}
}