modded class ADM_ShopUI: ChimeraMenuBase
{
	protected TextWidget m_wBalance;
	
	//------------------------------------------------------------------------------------------------
	override void PurchaseItem(SCR_ButtonComponent button)
	{
		super.PurchaseItem(button);
		
		UpdateBalance();
	}

	//------------------------------------------------------------------------------------------------
	void UpdateBalance()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		IEntity entity = playerController.GetControlledEntity();
		
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		int totalCurrency = ADM_CurrencyComponent.FindTotalCurrencyInInventory(inventory);
		
		if (m_wBalance)
			m_wBalance.SetText(string.Format("Balance %1 $", totalCurrency));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		super.OnMenuInit();

		m_wBalance = TextWidget.Cast(m_wRoot.FindAnyWidget("Balance"));
		
		if (!m_wBalance)
			Print("OTF - Couldn't find m_wBalance widget!", LogLevel.ERROR);
		
		UpdateBalance();
	}
}
