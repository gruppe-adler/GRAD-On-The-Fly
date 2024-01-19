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
		
		// only characters with the role 'OTF Commander' can perform this action
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(user);
		GRAD_CharacterRoleComponent characterRoleComponent = GRAD_CharacterRoleComponent.Cast(ch.FindComponent(GRAD_CharacterRoleComponent));
		string characterRole = characterRoleComponent.GetCharacterRole();
		if (characterRole != "OTF Commander")
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

		otfManager.AddBarrelSpawnRadiusMarkerToAllPlayers();
			
		otfManager.AddBarrelMarkerToAllPlayers();
	}
}