//------------------------------------------------------------------------------------------------
//! Map marker object base class 
modded class SCR_MapMarkerBase
{

	//------------------------------------------------------------------------------------------------
	//! Fetch marker definition from config & create widget
	override void OnCreateMarker()
	{
		super.OnCreateMarker();
		
		Print("OnCreateMarker");
	}

};
