
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class GRAD_OnTheFlyToolbarAction : SCR_EditorToolbarAction
{
	
	//------------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		GRAD_OnTheFlyManager otfManager = GRAD_OnTheFlyManager.GetInstance();
		
		int onTheFlyPhase = otfManager.GetOnTheFlyPhase();
		
		if (onTheFlyPhase == EOnTheFlyPhase.GAME)
			onTheFlyPhase = EOnTheFlyPhase.GAMEMASTER;
		else
			onTheFlyPhase += 1;
		
		otfManager.SetOnTheFlyPhase(onTheFlyPhase);
	}
}