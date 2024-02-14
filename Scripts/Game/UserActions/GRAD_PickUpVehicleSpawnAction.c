//------------------------------------------------------------------------------------------------
class GRAD_PickUpVehicleSpawnAction : SCR_PickUpItemAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager || (otfManager.GetOnTheFlyPhase() != EOnTheFlyPhase.GAME))
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager || (otfManager.GetOnTheFlyPhase() != EOnTheFlyPhase.GAME))
			return false;
		
		return true;
 	}
};