[EntityEditorProps(category: "Gruppe Adler", description: "On The Fly Gamemode Manager")]
class GRAD_OnTheFlyManagerClass : GenericEntityClass
{
}

class GRAD_OnTheFlyManager : GenericEntity
{
	protected static GRAD_OnTheFlyManager s_Instance;
	protected bool m_bOpforSpawnDone;
	protected bool m_bBluforSpawnDone;
	
	//------------------------------------------------------------------------------------------------
	bool OpforSpawnDone()
	{
		return m_bOpforSpawnDone;
	}
	
	//------------------------------------------------------------------------------------------------
	bool BluforSpawnDone()
	{
		return m_bBluforSpawnDone;
	}

	
	//------------------------------------------------------------------------------------------------
	void TeleportFactionToMapPos(Faction faction, int mapPos[2])
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);
		
		foreach (int playerId : playerIds)
		{
			Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(playerId);
			
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return;
			
			if (playerFaction == faction)
			{
				Print(string.Format("Player with ID %1 is Member of Faction %2 and will be teleported to %3", playerId, playerFaction.GetFactionKey(), mapPos), LogLevel.NORMAL);
				playerController.TeleportPlayerToMapPos(playerId, mapPos);
			}
		}
		
		if (faction == "USSR") {
			m_bOpforSpawnDone = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportYet(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "Opposing faction did not teleport yet. Wait for the signal.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportTwice(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport once. Deal with it.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	void NotifyCantTeleportThisFaction(Faction faction)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "You can only teleport your own faction. Stop trying.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) == faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void NotifyOpposingFaction(Faction faction, SCR_MapMarkerBase marker)
	{
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetAllPlayers(playerIds);

		string title = "On The Fly";
		string message = "Opposing faction was teleported. Open your map and choose a spawn point.";
		int duration = 10;
		bool isSilent = false;
		
		foreach (int playerId : playerIds)
		{
			if (SCR_FactionManager.SGetPlayerFaction(playerId) != faction)
			{
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
				if (!playerController)
					return;
			
				playerController.ShowHint(message, title, duration, isSilent);
				playerController.InsertLocalMarker(marker);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static GRAD_OnTheFlyManager GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	void GRAD_OnTheFlyManager(IEntitySource src, IEntity parent)
	{
		if (s_Instance)
		{
			Print("Only one instance of GRAD_OnTheFlyManager is allowed in the world!", LogLevel.WARNING);
			delete this;
			return;
		}

		s_Instance = this;

		// rest of the init code
	}
}