//------------------------------------------------------------------------------------------------
class GRAD_BluforShopAction : ADM_ShopAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager)
			return false;
		
		int otfPhase = otfManager.GetOnTheFlyPhase();
		
		if(!((otfPhase == EOnTheFlyPhase.BLUFOR) || (otfPhase == EOnTheFlyPhase.GAME)))
			return false;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
 	{
		if ( !super.CanBePerformedScript( user ) )
			return false;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager)
			return false;
		
		int otfPhase = otfManager.GetOnTheFlyPhase();
		
		if(!((otfPhase == EOnTheFlyPhase.BLUFOR) || (otfPhase == EOnTheFlyPhase.GAME)))
			return false;
		
		return true;
 	}
};