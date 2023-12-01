
//------------------------------------------------------------------------------------------------
//! Dynamic map marker -> squad leader
modded class SCR_MapMarkerSquadLeader : SCR_MapMarkerEntity
{
	// MODDED: changed from 15s to 1s
	const float SL_UPDATE_DELAY = 1; 		// seconds 
	
	//------------------------------------------------------------------------------------------------
	// RPL EVENTS
	//------------------------------------------------------------------------------------------------
	override void OnPlayerIdUpdate()
	{
		PlayerController pController = GetGame().GetPlayerController();
		if (!pController)
			return;
		
		// MODDED: removed filter for own squad; now you see your own squad
		SetLocalVisible(true);
		m_bDoGroupTextUpdate = true;
	}
}