class GRAD_ToggleSmokeUserAction : ScriptedUserAction
{
	private GRAD_BarrelSmokeComponent m_BarrelSmokeComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		return CanBePerformedScript(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{	
		if (!m_BarrelSmokeComponent) // Do nothing if there is no barrel smoke component
		{
			Print("GRAD OTF: No Barrel Smoke Component", LogLevel.ERROR);
			
			return false;
		}
		
		return true;
	}
	
	//---------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_BarrelSmokeComponent.IsSmoking())
			m_BarrelSmokeComponent.DisableSmoke();
		else
			m_BarrelSmokeComponent.EnableSmoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		if(m_BarrelSmokeComponent.IsSmoking())
		{
			outName = "Disable Smoke";
		} else
		{
			outName = "Enable Smoke";
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_BarrelSmokeComponent = GRAD_BarrelSmokeComponent.Cast(pOwnerEntity.FindComponent(GRAD_BarrelSmokeComponent));
	}
}