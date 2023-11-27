modded class SCR_BaseGameMode
{
	override protected void OnGameStateChanged()
	{
		super.OnGameStateChanged();
	
		SCR_EGameModeState newState = GetState();
		Print("SCR_BaseGameMode::OnGameStateChanged = " + SCR_Enum.GetEnumName(SCR_EGameModeState, newState));
		
		if (newState == SCR_EGameModeState.GAME)
			Print("lala");
	}
}
