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
			
			otfManager.SetOnTheFlyPhase(EOnTheFlyPhase.GAMEMASTER);
		}
	}
}
