modded class SCR_BaseGameMode
{
	override protected void OnGameStateChanged()
	{
		super.OnGameStateChanged();
	
		if (!Replication.IsServer())
			return;
		
		SCR_EGameModeState newState = GetState();
	
		if (newState == SCR_EGameModeState.GAME)
		{
			GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
			if (!otfManager)
				return;
			
			otfManager.SetOnTheFlyPhase(EOnTheFlyPhase.GAMEMASTER);
		}
	}
}
