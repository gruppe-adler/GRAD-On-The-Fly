class GRAD_FinishOpforPhaseUserAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{	
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		if(!otfManager || (otfManager.GetOnTheFlyPhase() != EOnTheFlyPhase.OPFOR))
			return false;
		
		return true;
	}
	
	//---------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;
		
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		if(!otfManager)
			return;
		
		otfManager.SetOnTheFlyPhase(EOnTheFlyPhase.BLUFOR);
		
		otfManager.AddBarrelMarkerToAllPlayers();
		
		otfManager.AddBarrelSpawnRadiusMarkerToAllPlayers();
	}
}