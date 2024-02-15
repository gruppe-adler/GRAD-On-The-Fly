//------------------------------------------------------------------------------------------------
class GRAD_OpforShopAction : ADM_ShopAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager)
			return false;
		
		int otfPhase = otfManager.GetOnTheFlyPhase();
		
		if(!((otfPhase == EOnTheFlyPhase.OPFOR) || (otfPhase == EOnTheFlyPhase.BLUFOR) || (otfPhase == EOnTheFlyPhase.GAME)))
			return false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController.GetTimerDuration() > (5 * 60))
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
		
		if(!((otfPhase == EOnTheFlyPhase.OPFOR) || (otfPhase == EOnTheFlyPhase.BLUFOR) || (otfPhase == EOnTheFlyPhase.GAME)))
			return false;
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController.GetTimerDuration() > (5 * 60))
			return false;
		
		return true;
 	}
};