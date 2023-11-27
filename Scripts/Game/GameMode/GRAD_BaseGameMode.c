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
			
			Print(string.Format("OTF - Event started in phase 'GAMEMASTER'"), LogLevel.NORMAL);
			
			otfManager.NotifyAllOnPhaseChange(EOnTheFlyPhase.GAMEMASTER);
		}
	}
}
