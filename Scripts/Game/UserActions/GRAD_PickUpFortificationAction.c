//------------------------------------------------------------------------------------------------
class GRAD_PickUpFortificationAction : SCR_PickUpItemAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager)
			return false;
		
		int otfPhase = otfManager.GetOnTheFlyPhase();
		
		if(!((otfPhase == EOnTheFlyPhase.OPFOR) || (otfPhase == EOnTheFlyPhase.BLUFOR)))
			return false;
		
		return true;
 	}
};